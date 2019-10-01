
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include "usloss.h"
#include "phase1Int.h"
#include "phase1.h"

void checkIfIsKernel();
int isValidSid(int sid); 

typedef struct Sem
{
    char        name[P1_MAXNAME+1];
    u_int       value;
    // more fields here
    int         initialized;
    int         queue[P1_MAXPROC];
    int         queueHead;
    int         queueSize;
} Sem;

static Sem sems[P1_MAXSEM];

void 
P1SemInit(void) 
{
    checkIfIsKernel();
    P1ProcInit();
    // initialize sems here
    int i;
    for (i = 0; i < P1_MAXPROC; i++) {
        sems[i].initialized = FALSE;
        sems[i].queueHead = -1;
        sems[i].queueSize = 0;
    }
}

int P1_SemCreate(char *name, unsigned int value, int *sid)
{
    checkIfIsKernel();
    int result = P1_SUCCESS;
    
    int interruptsWereEnabled = P1DisableInterrupts();
    int i;

    // check parameters
    if (name == NULL) return P1_NAME_IS_NULL;
    if (strlen(name) > P1_MAXNAME) return P1_NAME_TOO_LONG;
    for (i = 0; i < P1_MAXSEM; i++) {
        if (sems[i].initialized && strcmp(name, sems[i].name) == 0) return P1_DUPLICATE_NAME;
    }

    // find a free Sem and initialize it
    for (i = 0; i < P1_MAXSEM; i++) {
        if (!sems[i].initialized) {
            sems[i].initialized = TRUE;
            break;
        }
    }
    if (i == P1_MAXSEM) return P1_TOO_MANY_SEMS;
    strncpy(sems[i].name, name, P1_MAXPROC);
    sems[i].name[P1_MAXPROC] = '\0';
    *sid = i;

    if (interruptsWereEnabled) P1EnableInterrupts();
    return result;
}

int P1_SemFree(int sid) 
{
    checkIfIsKernel();

    int     result = P1_SUCCESS;
    if (!isValidSid(sid)) return P1_INVALID_SID;
    if (sems[sid].queueSize > 0) return P1_BLOCKED_PROCESSES;

    sems[sid].initialized = FALSE;
    sems[sid].queueSize = 0;
    sems[sid].queueHead = -1;

    return result;
}

int P1_P(int sid) 
{
    checkIfIsKernel();
    if (!isValidSid(sid)) return P1_INVALID_SID;

    int result = P1_SUCCESS;
    int interruptsWereEnabled;

    while (1) {
        interruptsWereEnabled = P1DisableInterrupts();

        if (sems[sid].value > 0) {
            sems[sid].value--;
            break;
        }
        int thisPid = P1_GetPid();
        P1SetState(thisPid, P1_STATE_BLOCKED, sid);

        // add item to blocked queue of semaphore
        if (sems[sid].queueSize == 0) {
            sems[sid].queue[0] = thisPid;
            sems[sid].queueHead = 0;
            sems[sid].queueSize++;
        } else {
            // check if process is in sem queue before adding
            int i, endIndex = sems[sid].queueHead + sems[sid].queueSize;
            for (i = sems[sid].queueHead; i < endIndex; i++) {
                if (sems[sid].queue[i % P1_MAXPROC] == thisPid) break;
            }
            // duplicate not found, add the item to the blocked queue
            if (i == endIndex) {
                sems[sid].queue[i % P1_MAXPROC] = thisPid;
                sems[sid].queueSize++;

                // maintain priority order
                int j;
                for (j = endIndex; j > sems[sid].queueHead; j--) {
                    int thisIndex = j % P1_MAXPROC;
                    int nextIndex = (j - 1) % P1_MAXPROC;

                    P1_ProcInfo thisProcInfo, nextProcInfo;
                    int rc;
                    rc = P1_GetProcInfo(sems[sid].queue[thisIndex], &thisProcInfo);
                    assert(rc == P1_SUCCESS);
                    rc = P1_GetProcInfo(sems[sid].queue[nextIndex], &nextProcInfo);
                    assert(rc == P1_SUCCESS);

                    // swap if out of priority order
                    if (thisProcInfo.priority < nextProcInfo.priority) {
                        int thisPid = sems[sid].queue[thisIndex];
                        sems[sid].queue[thisIndex] = sems[sid].queue[nextIndex];
                        sems[sid].queue[nextIndex] = thisPid;
                    }
                    else break;
                }
            }
        }

        if (interruptsWereEnabled) P1EnableInterrupts();
        P1Dispatch(FALSE);
    }

    if (interruptsWereEnabled) P1EnableInterrupts();
    return result;
}

int P1_V(int sid) 
{
    checkIfIsKernel();
    if (!isValidSid(sid)) return P1_INVALID_SID;

    int result = P1_SUCCESS;
    int interruptsWereEnabled = P1DisableInterrupts();

    sems[sid].value++;
    // if a process is waiting for this semaphore
    //      set the process's state to P1_STATE_READY
    if (sems[sid].queueSize > 0) {
        P1SetState(sems[sid].queue[sems[sid].queueHead], P1_STATE_READY, sid);
        // remove the process from the blocked queue of the semaphore
        sems[sid].queueHead = (sems[sid].queueHead + 1) % P1_MAXPROC;
        sems[sid].queueSize--;

        if (interruptsWereEnabled) P1EnableInterrupts();
        P1Dispatch(FALSE);
    }

    if (interruptsWereEnabled) P1EnableInterrupts();
    return result;
}

int P1_SemName(int sid, char *name) {
    checkIfIsKernel();
    if (!isValidSid(sid)) return P1_INVALID_SID;
    if (name == NULL) return P1_NAME_IS_NULL;
    strncpy(name, sems[sid].name, P1_MAXPROC);
    name[P1_MAXNAME] = '\0';
    return P1_SUCCESS;
}

/*
 * Checks psr to make sure OS is in kernel mode, halting USLOSS if not. Mode bit
 * is the LSB.
 */
void checkIfIsKernel(){ 
    if ((USLOSS_PsrGet() & 1) != 1) {
        USLOSS_Console("The OS must be in kernel mode!");
        USLOSS_IllegalInstruction();
    }
}

int isValidSid(int sid) {
    return sid >= 0 && sid < P1_MAXSEM && sems[sid].initialized;
}
