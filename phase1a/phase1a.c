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
    void *stack;
	int isAllocated;
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
    currentCid = -1;
	int i;
	for (i = 0; i < P1_MAXPROC; i++){
		contexts[i].startFunc = NULL;
		contexts[i].startArg = NULL;
        contexts[i].stack = NULL;
		contexts[i].isAllocated = FALSE;
	}
}

int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {
    int result = P1_SUCCESS;

	// find a free context and initialize it
	*cid = -1;
	for (int i = 0; i < P1_MAXPROC; i++) if (!contexts[i].isAllocated) {
		contexts[i].isAllocated = TRUE;
		*cid = i;
		break;
	}
    if (*cid == -1) return P1_TOO_MANY_CONTEXTS;

	contexts[*cid].startFunc = func;
	contexts[*cid].startArg = arg;

    // allocate stack
    if (stacksize < USLOSS_MIN_STACK) return P1_INVALID_STACK;
	char *stack = malloc(stacksize * sizeof(char));
    contexts[*cid].stack = stack;

	USLOSS_Context new;
	USLOSS_ContextInit(&new , stack, stacksize, P3_AllocatePageTable(*cid), &launch);
    contexts[*cid].context = new;
    return result;
}

int P1ContextSwitch(int cid) {
    int result = P1_SUCCESS;
    if (!cidIsValid(cid)) return P1_INVALID_CID;

    USLOSS_Context *oldContext = cidIsValid(currentCid) ? &(contexts[currentCid].context) : NULL;
    currentCid = cid;
    USLOSS_ContextSwitch(oldContext, &(contexts[cid].context));

    return result;
}

int P1ContextFree(int cid) {
    int result = P1_SUCCESS;
    if (!cidIsValid(cid)) return P1_INVALID_CID;

	contexts[cid].isAllocated = FALSE;
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
    return cid >= 0 && cid < P1_MAXPROC && contexts[cid].isAllocated;
}
