/*
Phase 1b
*/

#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

typedef struct ChildNode {
	int pid;
	int status;
	struct ChildNode *next;
} ChildNode;

typedef struct PCB {
    int             cid;                // context's ID
    int             cpuTime;            // process's running time
    char            name[P1_MAXNAME+1]; // process's name
    int             priority;           // process's priority
    P1_State        state;              // state of the PCB
    // more fields here
    int				(*startFunc)(void *);
	void			*startArg;
	int 			tag;
	ChildNode 		*childrenHead;
	int 			numChildren;
	ChildNode		*quitChildrenHead;
	int 			numChildrenQuit;
	int 			parent;
	int 			sid;
} PCB;

static int firstProcess = -1;

static PCB processTable[P1_MAXPROC];   // the process table
static int currentPid = -1;

void checkIfIsKernel();
int pidIsValid(int pid);
void addToHead(ChildNode *head, int pid, int status, int *listSize);
static void launch(void *arg);
extern  USLOSS_PTE  *P3_AllocatePageTable(int cid);
extern  void        P3_FreePageTable(int cid);

static int priorityQueue[P1_MAXPROC]; // heap of pid's
static int queueSize = 0;

// basic heap functionality methods
int removeMaxPriority();
void insertIntoRunnableQueue(int pid);
void removeElementFromQueue(int element);

void P1ProcInit(void)
{
	checkIfIsKernel();
    P1ContextInit();
    // initialize everything including the processTable
    currentPid = -1;
    queueSize = 0;
    firstProcess = -1;

	int i;
	for (i = 0; i < P1_MAXPROC; i++){
		processTable[i].cid = 0;
		processTable[i].cpuTime = 0;
		processTable[i].priority = 0;
		processTable[i].startFunc = NULL;
		processTable[i].startArg = NULL;
		processTable[i].state = P1_STATE_FREE;
	}

}

int P1_GetPid(void) 
{
	checkIfIsKernel();
	return currentPid;
}

/*
 * Helper function to call func passed to P1_Fork with its arg.
 */
static void launch(void *arg)
{
    int status = processTable[currentPid].startFunc(processTable[currentPid].startArg);
    P1_Quit(status);
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
	if (priority < 1 || priority > 6 || (priority == 6 && firstProcess != -1)){
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
		if (processTable[i].state == P1_STATE_FREE) {
			P1SetState(i, P1_STATE_READY, 0);
			*pid = i;
			break;
		}
		else if (strcmp(processTable[i].name, name) == 0) {
			return P1_DUPLICATE_NAME;
		}
	}
	if (i == P1_MAXPROC) return P1_TOO_MANY_PROCESSES;

	*pid = i;
	P1ContextCreate(launch, arg, stacksize, &processTable[*pid].cid);
	processTable[*pid].startFunc = func;
	processTable[*pid].startArg = arg;
	processTable[*pid].priority = priority;
	processTable[*pid].tag = tag;
	processTable[*pid].parent = currentPid;
	processTable[*pid].childrenHead = (ChildNode *) malloc(sizeof(ChildNode));
	processTable[*pid].childrenHead -> next = NULL;
	processTable[*pid].numChildren = 0;
	processTable[*pid].quitChildrenHead = (ChildNode *) malloc(sizeof(ChildNode));
	processTable[*pid].quitChildrenHead -> next = NULL;
	processTable[*pid].numChildrenQuit = 0;
	if (currentPid != -1) {
		addToHead(processTable[currentPid].childrenHead, *pid, 0, &processTable[currentPid].numChildren);
	}
	strncpy(processTable[*pid].name, name, P1_MAXNAME + 1);

	insertIntoRunnableQueue(*pid);

    // if this is the first process or this process's priority is higher than the 
    //    currently running process call P1Dispatch(FALSE)
    int isFirstFork = firstProcess == -1;
    if (isFirstFork) firstProcess = *pid;
	if (isFirstFork || priority > processTable[currentPid].priority) P1Dispatch(FALSE);

    // re-enable interrupts if they were previously enabled
	if (interruptsWereEnabled) P1EnableInterrupts();
    return result;
}

void 
P1_Quit(int status) 
{
	// check for kernel mode
	checkIfIsKernel();
    // disable interrupts
    P1DisableInterrupts();
    // remove from ready queue, set status to P1_STATE_QUIT
    removeElementFromQueue(currentPid);
    processTable[currentPid].state = P1_STATE_QUIT;

	// if first process verify it doesn't have children
    if (currentPid == firstProcess ) {
    	if (processTable[firstProcess].numChildren != 0) {
    		USLOSS_Console("First process quitting with children, halting. \n");
    		USLOSS_Halt(1);
    	} 
    } else { // otherwise give children to first process
    	ChildNode *temp = processTable[currentPid].childrenHead;
    	while (temp -> next != NULL) {
    		addToHead(processTable[firstProcess].childrenHead, temp -> next -> pid, temp -> next -> status, &processTable[firstProcess].numChildren);
    		if (processTable[temp -> next -> pid].state == P1_STATE_QUIT) 
    			addToHead(processTable[firstProcess].quitChildrenHead, temp -> next -> pid, temp -> next -> status, &processTable[firstProcess].numChildrenQuit);
    	}
    }

	int parent = processTable[currentPid].parent;
    if (pidIsValid(parent)) {
    	// add ourself to list of our parent's children that have quit
    	addToHead(processTable[parent].quitChildrenHead, currentPid, status, &processTable[parent].numChildrenQuit);
	}

    if (processTable[parent].state == P1_STATE_JOINING) P1SetState(parent, P1_STATE_READY, 0);
	currentPid = -1;
    P1Dispatch(FALSE);
    // should never get here
    assert(0);
}

void freeList(ChildNode *head) {
	ChildNode *temp = head;
	while (temp != NULL) {
		ChildNode *current = temp;
		temp = temp -> next;
		free(current);
	}
}

void removeElementFromList(ChildNode *head, int pid, int *listSize) {
	ChildNode *temp = head;

	while (temp -> next != NULL) {
		if (temp -> next -> pid == pid) {
			ChildNode *found = temp -> next;
			temp -> next = found -> next;
			free(found);
			(*listSize)--;
			continue;
		}
		temp = temp -> next;
	}
}

void freeProcess(int pid) {
	int interruptsWereEnabled = P1DisableInterrupts();
	P1ContextFree(processTable[pid].cid);
	freeList(processTable[pid].childrenHead);
	freeList(processTable[pid].quitChildrenHead);
	processTable[pid].childrenHead = processTable[pid].quitChildrenHead = NULL;
	int parent = processTable[pid].parent;

	removeElementFromList(processTable[parent].childrenHead, pid, &processTable[parent].numChildren);
	removeElementFromList(processTable[parent].quitChildrenHead, pid, &processTable[parent].numChildrenQuit);

	processTable[pid].state = P1_STATE_FREE;
	if (interruptsWereEnabled) P1EnableInterrupts();
}

int 
P1GetChildStatus(int tag, int *pid, int *status) 
{
	// check for kernel mode
	checkIfIsKernel();

    if (tag % 2 != tag) return P1_INVALID_TAG;

    if (processTable[currentPid].numChildren == 0) return P1_NO_CHILDREN;
    for (ChildNode *child = processTable[currentPid].quitChildrenHead; child -> next != NULL; child = child -> next) {
    	if (processTable[child -> next -> pid].tag == tag) {
    		*pid = child -> next -> pid;
    		*status = child -> next -> status;
    		freeProcess(*pid);
    		return P1_SUCCESS;
    	}
    }
    return P1_NO_QUIT;
}

int
P1SetState(int pid, P1_State state, int sid) 
{
	// check for kernel mode
	checkIfIsKernel();
	if (!pidIsValid(pid)) return P1_INVALID_PID;

   	int result = P1_SUCCESS;

    switch(state) {
    	case P1_STATE_READY:
    		insertIntoRunnableQueue(pid);
    		break;
    	case P1_STATE_JOINING:
			if (processTable[pid].numChildrenQuit != 0) return P1_CHILD_QUIT;
    		break;
    	case P1_STATE_BLOCKED:
    		processTable[pid].sid = sid;
    		break;
    	case P1_STATE_QUIT: break;
    	default: return P1_INVALID_STATE;
    }
    if (processTable[pid].state == P1_STATE_READY) removeElementFromQueue(pid);
    processTable[pid].state = state;
    return result;
}

void
P1Dispatch(int rotate)
{
	// check for kernel mode
	checkIfIsKernel();

    // select the highest-priority runnable process
    int highestPriorityProcess = removeMaxPriority();

    if (highestPriorityProcess == -1) {
    	USLOSS_Console("No runnable processes, halting.\n");
    	USLOSS_Halt(0);
    } else if (currentPid != -1 && processTable[highestPriorityProcess].priority == processTable[currentPid].priority && !rotate) {
    	return;
    }

    if (currentPid != -1) P1SetState(currentPid, P1_STATE_READY, 0);
    currentPid = highestPriorityProcess;
    processTable[currentPid].state = P1_STATE_RUNNING;
    removeElementFromQueue(currentPid);
    P1ContextSwitch(processTable[currentPid].cid);
    // call P1ContextSwitch to switch to that process
}

int
P1_GetProcInfo(int pid, P1_ProcInfo *info)
{
	// check for kernel mode
	checkIfIsKernel();
    int result = P1_SUCCESS;
	if (!pidIsValid(pid)){
		return P1_INVALID_PID;
	}
    // fill in info here
	strcpy(info->name, processTable[pid].name);
	info->state = processTable[pid].state;       // process's state
	info->sid = processTable[pid].sid;			 // semaphore on which process is blocked, if any
	info->priority = processTable[pid].priority; // process's priority
	info->tag = processTable[pid].tag;           // process's tag
	info->cpu = processTable[pid].cpuTime;       // CPU consumed (in us)
    info->parent = processTable[pid].parent;     // parent PID

	int i = 0;
	for (ChildNode *child = processTable[pid].childrenHead; child->next != NULL; child = child->next) {
		info->children[i++] = child->pid;
	}
    info->numChildren = processTable[pid].numChildren;

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

int pidIsValid(int pid) {
	return pid >= 0 && pid < P1_MAXPROC && processTable[pid].state != P1_STATE_FREE;
}

void addToHead(ChildNode *head, int pid, int status, int *listSize) {
	ChildNode *temp = head;
	while (temp -> next != NULL) {
		if (temp -> next -> pid == pid) return;
	}

	ChildNode *newNode = (ChildNode *) malloc(sizeof(ChildNode));
	newNode -> pid = pid;
	newNode -> status = status;
	newNode -> next = NULL;

	temp -> next = newNode;
	(*listSize)++;
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
int removeMaxPriority() {
	if (queueSize == 0) return -1;
	swap(0, --queueSize);
	bubbleDown(0);

	return priorityQueue[queueSize];
}

/**
 * Inserts a pid into the heap.
 */
void insertIntoRunnableQueue(int pid) {
	priorityQueue[queueSize] = pid;
	bubbleUp(queueSize);
	queueSize++;
}

void removeElementFromQueue(int element) {
	int i;
	for (i = 0; i < queueSize; i++) {
		if (priorityQueue[i] == element) {
			swap(i, --queueSize);
			bubbleDown(i);
			return;
		}
	}
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
	return children;
}

