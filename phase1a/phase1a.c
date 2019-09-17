#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

extern  USLOSS_PTE  *P3_AllocatePageTable(int cid);
extern  void        P3_FreePageTable(int cid);

void checkIfKernelMode();
int cidIsValid(int cid);

typedef struct Context {
    void            (*startFunc)(void *);
    void            *startArg;
    USLOSS_Context  context;

    int cid;
    void *stack;
} Context;

static Context   contexts[P1_MAXPROC];

static int currentCid = -1;
static int numContexts = 0;

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
	contexts[i].startArg = arg;
	// above this line, all is correct
	char *stack = malloc(stacksize * sizeof(char));  
	if (stacksize < USLOSS_MIN_STACK){
		return P1_INVALID_STACK;
	}
	USLOSS_Context new;
	USLOSS_ContextInit(&new , stack, stacksize, NULL, launch);
	printf("%d----%d\n", stacksize,890);
	//USLOSS_ContextSwitch(contexts[i]->Context, &new);
    return result;
}

int P1ContextSwitch(int cid) {
    int result = P1_SUCCESS;
    if (cidIsValid(cid)) return P1_INVALID_CID;

    USLOSS_Context *oldContext = cidIsValid(currentCid) ? &(contexts[currentCid].context) : NULL;
    USLOSS_ContextSwitch(oldContext, &(contexts[cid].context));
    currentCid = cid;
    return result;
}

int P1ContextFree(int cid) {
    int result = P1_SUCCESS;
    if (cidIsValid(cid)) return P1_INVALID_CID;

    free(contexts[cid].stack);
    P3_FreePageTable(cid);
    return result;
}


void 
P1EnableInterrupts(void) 
{
    int rc = USLOSS_PsrSet(USLOSS_PsrGet() | (1 << 1)); // set 2nd but of the psr to 1
    assert(rc == USLOSS_DEV_OK);
}

/*
 * Returns true if interrupts were enabled, false otherwise.
 */
int 
P1DisableInterrupts(void) 
{
    int enabled = FALSE;
    unsigned int psr = USLOSS_PsrGet();
    enabled = (psr >> 1) & 1; // set enabled to TRUE if interrupts are already enabled
    
    unsigned int mask = 0xFFFFFFFF ^ (1 << 1); // all 1's except for 2nd bit
    int rc = USLOSS_PsrSet(psr & mask); // clear the interrupt bit in the PSR
    assert(rc == USLOSS_DEV_OK);

    return enabled;
}

/*
 * Checks psr to make sure OS is in kernel mode, halting USLOSS if not. Mode bit
 * is the LSB.
 */
void checkIfKernelMode(){
    if ((USLOSS_PsrGet() & 1) != 1) {
        printf("The OS must be in kernel mode!");
        USLOSS_Halt(1);
    }
}

int cidIsValid(int cid) {
    return cid >= 0 && cid < numContexts;
}
