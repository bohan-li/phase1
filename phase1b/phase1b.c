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
    int             cid;                // context's ID
    int             cpuTime;            // process's running time
    char            name[P1_MAXNAME+1]; // process's name
    int             priority;           // process's priority
    P1_State        state;              // state of the PCB
    // more fields here
    int 			initialize;
    int				(*startFunc)(void *);
	void			*startArg;
	int 			tag;
} PCB;

static PCB processTable[P1_MAXPROC];   // the process table
static int currentPid = -1;

void checkIfIsKernel();
static void launch(void *arg);
extern  USLOSS_PTE  *P3_AllocatePageTable(int cid);
extern  void        P3_FreePageTable(int cid);

static int priorityQueue[P1_MAXPROC]; // heap of pid's
static int queueSize = 0;
// basic functionality methods
int removeMax();
void insert(int pid);

void P1ProcInit(void)
{
    P1ContextInit();
    // initialize everything including the processTable
    currentPid = -1;
    queueSize = 0;

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

/*
 * Helper function to call func passed to P1_Fork with its arg.
 */
static void launch(void *arg)
{
    processTable[currentPid].startFunc(processTable[currentPid].startArg);
    P1_Quit(0);
}

int P1_Fork(char *name, int (*func)(void*), void *arg, int stacksize, int priority, int tag, int *pid ) 
{
	int result = P1_SUCCESS;

	// check for kernel mode
	checkIfIsKernel();

    // disable interrupts
	int interruptsWereEnabled = P1DisableInterrupts();
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

	*pid = i;
	P1ContextCreate(func, arg, stacksize, &processTable[*pid].cid);
	processTable[*pid].startFunc = func;
	processTable[*pid].startArg = arg;
	strncpy(processTable[*pid].name, name, P1_MAXNAME + 1);

    // if this is the first process or this process's priority is higher than the 
    //    currently running process call P1Dispatch(FALSE)
    // re-enable interrupts if they were previously enabled
	if (interruptsWereEnabled) P1EnableInterrupts();
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
void checkIfIsKernel(){ 
	if ((USLOSS_PsrGet() & 1) != 1) {
		USLOSS_Console("The OS must be in kernel mode!");
		USLOSS_Halt(1);
    }
}

/************************************************************************************/
/******************************* HEAP FUNCTIONALITY *********************************/
/************************************************************************************/
#define NO_SUCH_INDEX -1

/**
 * Class for holding two integers representing child indices
 * 
 * @author Bohan Li
 *
 */
typedef struct ChildrenIndices {
	int left;
	int right;
} ChildrenIndices;

// helper functions
int compare(int pid1, int pid2);
void swap(int index1, int index2);
void bubbleUp(int index);
void bubbleDown(int index);
int getParentIndex(int index);
ChildrenIndices getChildrenIndices(int index);

/**
 * Compares priority of two pid's.
 */
int compare(int pid1, int pid2) {
	return processTable[pid1].priority - processTable[pid2].priority;
}

/**
 * Swaps elements at the two indices given.
 */
void swap(int index1, int index2) {
	int temp = priorityQueue[index1];
	priorityQueue[index1] = priorityQueue[index2];
	priorityQueue[index2] = temp;
}

/**
 * Removes the highest priority pid.
 * 
 * @return the pid with the highest priority, -1 if heap is empty
 */
int removeMax() {
	if (queueSize == 0) return -1;
	swap(0, --queueSize);
	bubbleDown(0);
	return priorityQueue[queueSize];
}

/**
 * Inserts a pid into the heap.
 */
void insert(int pid) {
	priorityQueue[queueSize] = pid;
	bubbleUp(queueSize);
	queueSize++;
}

/**
 * Bubbles up an element in a heap until it is in the correct place relative to
 * its parent
 * 
 * @param index
 *            the index of the element to bubble up
 */
void bubbleUp(int index) {
	int parentIndex = getParentIndex(index);

	if (parentIndex != NO_SUCH_INDEX && compare(priorityQueue[index], priorityQueue[parentIndex]) > 0) {
		swap(index, parentIndex);
		bubbleUp(parentIndex);
	}
}

/**
 * Bubbles down an element in a heap until it is in the correct place relative
 * to its children.
 * 
 * @param index
 *            the index of the element to bubble down
 */
void bubbleDown(int index) {
	ChildrenIndices children = getChildrenIndices(index);
	int largerChildIndex;
	if (children.left == NO_SUCH_INDEX) {
		return;
	} else if (children.right == NO_SUCH_INDEX) {
		largerChildIndex = children.left;
	} else {
		largerChildIndex = compare(priorityQueue[children.left], priorityQueue[children.right]) > 0 ? children.left : children.right;
	}
	if (compare(priorityQueue[index], priorityQueue[largerChildIndex]) < 0) {
		swap(largerChildIndex, index);
		bubbleDown(largerChildIndex);
	}
}

/**
 * Returns index of parent.
 * 
 * @param index
 * @return parent indices of index, or -1 if no parent.
 */
int getParentIndex(int index) {
	if (index <= 0)
		return NO_SUCH_INDEX;
	return (index - 1) / 2;
}

/**
 * Returns children indices of index.
 * 
 * @param index
 * @return ChildrenIndices of index, -1 is the value if the child does not
 *         exist.
 */
ChildrenIndices getChildrenIndices(int index) {
	ChildrenIndices children;
	int leftChild = 2 * index + 1;
	if (leftChild >= queueSize) {
		children.left = NO_SUCH_INDEX;
		children.right = NO_SUCH_INDEX;
		return children;
	}
	int rightChild = leftChild + 1;
	if (rightChild >= queueSize) {
		rightChild = NO_SUCH_INDEX;
	}
	children.left = leftChild;
	children.right = rightChild;
}

