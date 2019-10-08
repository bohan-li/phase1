#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "usloss.h"
#include "phase1.h"
#include "phase1Int.h"

#define USLOSS_MAX_TYPES 4

static void DeviceHandler(int type, void *arg);
static void SyscallHandler(int type, void *arg);
static void IllegalInstructionHandler(int type, void *arg);
void checkIfIsKernel();
int isValidType(int type);
int isValidUnit(int type, int unit);

static int sentinel(void *arg);

typedef struct device {
	int sid;
	int abort;
} Device;

static Device devices[USLOSS_MAX_TYPES][USLOSS_MAX_UNITS];

void 
startup(int argc, char **argv)
{
	checkIfIsKernel();
    int pid;
    P1SemInit();
	
	// initialize device datastructures
	int i, j, rc;
	char name[P1_MAXNAME];
	for (i = 0; i < USLOSS_MAX_TYPES; i++) {
		for (j = 0; j < USLOSS_MAX_UNITS; j++) {
			sprintf(name, "%d%d", i, j);
			rc = P1_SemCreate(name, 0, &(devices[i][j].sid));
			assert(rc == P1_SUCCESS);
			devices[i][j].abort = 0;
		}
	}
	
    // put device interrupt handlers into interrupt vector
    USLOSS_IntVec[USLOSS_SYSCALL_INT] = SyscallHandler;
    USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalInstructionHandler;

    /* create the sentinel process */
    int rc = P1_Fork("sentinel", sentinel, NULL, USLOSS_MIN_STACK, 6 , 0, &pid);
    assert(rc == P1_SUCCESS);
    // should not return
    assert(0);
    return;

} /* End of startup */

int 
P1_WaitDevice(int type, int unit, int *status) 
{
	checkIfIsKernel();
	if (!isValidType(type)) return P1_INVALID_TYPE;
	if (!isValidUnit(type, unit)) return P1_INVALID_UNIT;
	
    int     result = P1_SUCCESS;
    // P device's semaphore
	result = P1_P(devices[type][unit].sid);
	assert(result == P1_SUCCESS);
	
    // set *status to device's status
    return device[type][unit].abort ? P1_WAIT_ABORTED : P1_SUCCESS;
}

int 
P1_WakeupDevice(int type, int unit, int status, int abort) 
{
	checkIfIsKernel();
    int     result = P1_SUCCESS;
    // save device's status
    // V device's semaphore 
    return result;
}

static void
DeviceHandler(int type, void *arg) 
{
	checkIfIsKernel();
    // if clock device
    //      P1_WakeupDevice every 5 ticks
    //      P1Dispatch(TRUE) every 4 ticks
    // else
    //      P1_WakeupDevice
}

static int
sentinel (void *notused)
{
    int     pid;
    int     rc;

    /* start the P2_Startup process */
    rc = P1_Fork("P2_Startup", P2_Startup, NULL, 4 * USLOSS_MIN_STACK, 2 , 0, &pid);
    assert(rc == P1_SUCCESS);

    // enable interrupts
    // while sentinel has children
    //      get children that have quit via P1GetChildStatus
    //      wait for an interrupt via USLOSS_WaitInt
    USLOSS_Console("Sentinel quitting.\n");
    return 0;
} /* End of sentinel */

int 
P1_Join(int tag, int *pid, int *status) 
{
	checkIfIsKernel();
    int result = P1_SUCCESS;
    // check for kernel mode
    // disable interrupts
    // do
    //  get a child that has quit via P1GetChildStatus 
    //  if no children have quit
    //      set state to P1_STATE_JOINING vi P1SetState
    //      P1Dispatch(FALSE)
    // until either a child quit or there are no more children
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

int isValidType(int type) {
	return type < USLOSS_MAX_TYPES;
}

int isValidUnit(int type, int unit) {
	switch (type) {
		case USLOSS_CLOCK_DEV: return unit < USLOSS_CLOCK_UNITS;
		case USLOSS_ALARM_DEV: return unit < USLOSS_ALARM_UNITS;
		case USLOSS_DISK_DEV: return unit < USLOSS_DISK_UNITS;
		case USLOSS_TERM_DEV: return unit < USLOSS_TERM_UNITS;
		default: return FALSE;
	}
}

static void
IllegalInstructionHandler(int type, void *arg)
{
    P1_Quit(1024); // quit process with arbitrary status 1024
}

static void
SyscallHandler(int type, void *arg) 
{
    USLOSS_Console("System call %d not implemented.\n", (int) arg);
    USLOSS_IllegalInstruction();
}

void finish(int argc, char **argv) {}

