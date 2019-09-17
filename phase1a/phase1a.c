#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

extern  USLOSS_PTE  *P3_AllocatePageTable(int cid);
extern  void        P3_FreePageTable(int cid);

void checkIfKernelMode();

typedef struct Context {
    void            (*startFunc)(void *);
    void            *startArg;
    USLOSS_Context  context;

    int cid;
    void *stack;
    USLOSS_PTE pageTable;
    // you'll need more stuff here
} Context;

static Context   contexts[P1_MAXPROC];

static int currentCid = -1;

/*
 * Helper function to call func passed to P1ContextCreate with its arg.
 */
static void launch(void)
{
    assert(contexts[currentCid].startFunc != NULL);
    contexts[currentCid].startFunc(contexts[currentCid].startArg);
}

void P1ContextInit(void)
{
    checkIfKernelMode();
	int i;
	for (i = 0; i < P1_MAXPROC; i++){
		contexts[i].startFunc = NULL;
		contexts[i].startArg = NULL;
        contexts[i].context = NULL;
        contexts[i].cid = -1;
	}
}

int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {
    checkIfKernelMode();
    int result = P1_SUCCESS;
    // find a free context and initialize it
    // allocate the stack, specify the startFunc, etc.
	int i;
	for (i = 0; i < P1_MAXPROC; i++){
		
	}
	
	
    return result;
}

int P1ContextSwitch(int cid) {
    int result = P1_SUCCESS;
    // switch to the specified context
    return result;
}

int P1ContextFree(int cid) {
    int result = P1_SUCCESS;
    if (currentCid >= P1_MAXPROC) return P1_TOO_MANY_PROCESSES;
    return result;
}


void 
P1EnableInterrupts(void) 
{
    // set the interrupt bit in the PSR
}

/*
 * Returns true if interrupts were enabled, false otherwise.
 */
int 
P1DisableInterrupts(void) 
{
    int enabled = FALSE;
    // set enabled to TRUE if interrupts are already enabled
    // clear the interrupt bit in the PSR
    return enabled;
}


void checkIfKernelMode(){
    if (USLOSS_PsrGet() != 1) {
        printf("The OS must be in kernel mode!");
        USLOSS_Halt(1);
    }
}