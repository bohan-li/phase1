/*
Phase 1b
*/

#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


typedef struct PCB {
	int            cid;                // context's ID
	int            cpuTime;            // process's running time
	char           name[P1_MAXNAME+1]; // process's name
	int            priority;           // process's priority
	P1_State       state;              // state of the PCB
	// more fields here
	int            initialize; // 
	int            (*startFunc)(void *);
	void           *startArg;
	void           *stack;
	USLOSS_Context context;
} PCB;

static PCB processTable[P1_MAXPROC];   // the process table
static int currentPid = -1;
void checkIfIsKernel();
static void launch(void);
extern  USLOSS_PTE	*P3_AllocatePageTable(int cid);
extern  void		P3_FreePageTable(int cid);


void P1ProcInit(void)
{
	P1ContextInit();
	// initialize everything including the processTable
	int i;
	for (i = 0; i < P1_MAXPROC; i++){
		processTable[i].cid = 0;
		processTable[i].cpuTime = 0;
		processTable[i].priority = 0;
		processTable[i].initialize = 1;
	}

}

int P1_GetPid(void) 
{
	return currentPid;
}

int P1_Fork(char *name, int (*func)(void*), void *arg, int stacksize, int priority, int tag, int *pid ) 
{
	int result = P1_SUCCESS;

	// check for kernel mode
	checkIfIsKernel();
	// disable interrupts
	P1DisableInterrupts();
	// check all parameters
	if (name == NULL){
		return P1_NAME_IS_NULL;
	}
	int len = strlen(name);
	if (len > P1_MAXNAME){
		return P1_NAME_TOO_LONG;
	}
	if (priority < 1 || priority > 6){
		return P1_INVALID_PRIORITY;
	}
	if (tag != 1 && tag != 0){
		return P1_INVALID_TAG;
	}
	if (stacksize < USLOSS_MIN_STACK){
		return P1_INVALID_STACK;
	}
	// create a context using P1ContextCreate
	int i;
	// find a free context and initialize it
	for (i = 0; i < P1_MAXPROC; i++){
		if (strcmp(processTable[i].name, name) == 0){
			return P1_DUPLICATE_NAME;
		}	
		if (processTable[i].initialize) {
			processTable[i].initialize = 0;
			*pid = i;
			break;
		}
	}
	if (i == P1_MAXPROC) return P1_TOO_MANY_PROCESSES;
	// allocate and initialize PCB
	processTable[*pid].startFunc = func;
	processTable[*pid].startArg = arg;

	// allocate stack
	void *stack = malloc(stacksize);
	processTable[*pid].stack = stack;
	USLOSS_Context new;
	USLOSS_ContextInit(&new , stack, stacksize, P3_AllocatePageTable(*pid), &launch);
	processTable[*pid].context = new;
	// if this is the first process or this process's priority is higher than the 
	//    currently running process call P1Dispatch(FALSE)
	// re-enable interrupts if they were previously enabled
	P1EnableInterrupts();
	return result;
}

void 
P1_Quit(int status) 
{
    // check for kernel mode
    // disable interrupts
    // remove from ready queue, set status to P1_STATE_QUIT
    // if first process verify it doesn't have children, otherwise give children to first process
    // add ourself to list of our parent's children that have quit
    // if parent is in state P1_STATE_JOINING set its state to P1_STATE_READY
    P1Dispatch(FALSE);
    // should never get here
    assert(0);
}


int 
P1GetChildStatus(int tag, int *cpid, int *status) 
{
    int result = P1_SUCCESS;
    // do stuff here
    return result;
}

int
P1SetState(int pid, P1_State state, int sid) 
{
    int result = P1_SUCCESS;
    // do stuff here
    return result;
}

void
P1Dispatch(int rotate)
{
    // select the highest-priority runnable process
    // call P1ContextSwitch to switch to that process
}

int
P1_GetProcInfo(int pid, P1_ProcInfo *info)
{
    int         result = P1_SUCCESS;
	if (pid < 0 || pid >= P1_MAXPROC){
		return P1_INVALID_PID;
	}
    // fill in info here
	strcpy(info->name, processTable[pid].name);
	info->state = processTable[pid].state;       // process's state
	//info->sid // semaphore on which process is blocked, if any
	info->priority = processTable[pid].priority; // process's priority
	//info->tag = processTable[pid].tag;           // process's tag
    //info->cpu                                  // CPU consumed (in us)
    //info->parent;                              // parent PID
    //info->children[P1_MAXPROC];                // childen PIDs
    //info->numChildren;                         // # of children

   return result;
}


/*
 * Checks psr to make sure OS is in kernel mode, halting USLOSS if not. Mode bit
 * is the LSB.
 */
void checkIfKernelMode(){
    if ((USLOSS_PsrGet() & 1) != 1) {
        USLOSS_Console("The OS must be in kernel mode!");
        USLOSS_Halt(1);
    }
}

void checkIfIsKernel(){ 
	if ((USLOSS_PsrGet() & 1) != 1) {
		USLOSS_Console("The OS must be in kernel mode!");
		USLOSS_Halt(1);
    }
}

static void launch(void)
{
	assert(processTable[currentPid].startFunc != NULL);
	processTable[currentPid].startFunc(processTable[currentPid].startArg);
}

