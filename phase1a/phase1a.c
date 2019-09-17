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
    int result = P1_SUCCESS;
    // find a free context and initialize it
    // allocate the stack, specify the startFunc, etc.
	int i;
	for (i = 0; i < P1_MAXPROC; i++){
		if (contexts[i].startFunc == NULL){
			break;
		}
	}
	if (i == P1_MAXPROC){
		return P1_TOO_MANY_CONTEXTS;
	}
	contexts[i].startFunc = func;
	printf("%d----%d\n", result,12121212);
	contexts[i].startArg = arg;
	printf("%d----%d\n", result,121212333);
	// below this line, all is correct
	char stack0[stacksize];  // there is somethign wrong this line
	printf("%d----%d\n", result,813);
	if (stacksize < USLOSS_MIN_STACK){
		return P1_INVALID_STACK;
	}
	printf("%d----%d\n", result,814);
	USLOSS_Context new;
	printf("%d----%d\n", result,889);
	USLOSS_ContextInit(&new , stack0, sizeof(stack0), NULL, launch);
	printf("%d----%d\n", result,890);
	//USLOSS_ContextSwitch(contexts[i]->Context, &new);
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
