
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include "usloss.h"
#include "phase1Int.h"

void checkIfIsKernel();

typedef struct Sem
{
    char        name[P1_MAXNAME+1];
    u_int       value;
    // more fields here
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
        sems.queueHead[i] = -1;
        sems.queueSize[i] = 0;
    }
}

int P1_SemCreate(char *name, unsigned int value, int *sid)
{
    checkIfIsKernel();
    int result = P1_SUCCESS;
    // check for kernel mode
    // disable interrupts
    // check parameters
    // find a free Sem and initialize it
    // re-enable interrupts if they were previously enabled
    return result;
}

int P1_SemFree(int sid) 
{
    checkIfIsKernel();

    int     result = P1_SUCCESS;
    // more code here
    return result;
}

int P1_P(int sid) 
{
    checkIfIsKernel();

    int result = P1_SUCCESS;
    // check for kernel mode
    // disable interrupts
    // while value == 0
    //      set state to P1_STATE_BLOCKED
    // value--
    // re-enable interrupts if they were previously enabled
    return result;
}

int P1_V(int sid) 
{
    checkIfIsKernel();

    int result = P1_SUCCESS;
    // check for kernel mode
    // disable interrupts
    // value++
    // if a process is waiting for this semaphore
    //      set the process's state to P1_STATE_READY
    // re-enable interrupts if they were previously enabled
    return result;
}

int P1_SemName(int sid, char *name) {
    checkIfIsKernel();

    int result = P1_SUCCESS;
    // more code here
    return result;
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