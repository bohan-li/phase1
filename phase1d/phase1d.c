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
	int status;
} Device;

static Device devices[USLOSS_MAX_TYPES][USLOSS_MAX_UNITS];

// starts up and creates the sentinel
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
	USLOSS_IntVec[USLOSS_CLOCK_INT] = DeviceHandler;	
	USLOSS_IntVec[USLOSS_ALARM_INT] = DeviceHandler;
	USLOSS_IntVec[USLOSS_DISK_INT] = DeviceHandler;
	USLOSS_IntVec[USLOSS_TERM_INT] = DeviceHandler;

    /* create the sentinel process */
    rc = P1_Fork("sentinel", sentinel, NULL, USLOSS_MIN_STACK, 6 , 0, &pid);
    assert(rc == P1_SUCCESS);
    // should not return
    assert(0);
    return;

} /* End of startup */

// causes the device specified by the given type and unit to wait
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
	
	*status = devices[type][unit].status;
    return devices[type][unit].abort ? P1_WAIT_ABORTED : P1_SUCCESS;
}

// Wakes up the device given by the type and the unit.
// The status is set as the device status.
int 
P1_WakeupDevice(int type, int unit, int status, int abort) 
{
	checkIfIsKernel();
	if (!isValidType(type)) return P1_INVALID_TYPE;
	if (!isValidUnit(type, unit)) return P1_INVALID_UNIT;
		
    int     result = P1_SUCCESS;
	
    // save device's status
	devices[type][unit].status = status;
	devices[type][unit].abort = abort;

	result = P1_V(devices[type][unit].sid);
    return result;
}
static int tick = 0;

// handles all interrupts for phase 1
static void
DeviceHandler(int type, void *arg) 
{
	checkIfIsKernel();
	tick = (tick + 1) % 20;
	int unit = (int) arg;
	int status;

	USLOSS_DeviceInput(type, unit, &status);
    if (type == USLOSS_CLOCK_DEV) {
		if (tick % 5 == 0) P1_WakeupDevice(type, unit, status, FALSE);
        if (tick % 4 == 0) P1Dispatch(TRUE);
	}
    else P1_WakeupDevice(type, unit, status, FALSE);
}

// first process, is always runnable
static int
sentinel (void *notused)
{
    int     pid;
    int     rc;

    /* start the P2_Startup process */
    rc = P1_Fork("P2_Startup", P2_Startup, NULL, 4 * USLOSS_MIN_STACK, 2 , 0, &pid);
    assert(rc == P1_SUCCESS);

	P1EnableInterrupts();
	int	tag, status;
	P1_ProcInfo info;
	while (1) {
		rc = P1_GetProcInfo(P1_GetPid(), &info);
		assert(rc == P1_SUCCESS);
		if (info.numChildren == 0) break;
		for (tag = 0; tag < 2; tag++) {
			rc = P1GetChildStatus(tag, &pid, &status);
			if (rc == P1_NO_QUIT || rc == P1_NO_CHILDREN) continue;
			assert(rc == P1_SUCCESS);
		}
		
		USLOSS_WaitInt();
	}

    USLOSS_Console("Sentinel quitting.\n");
    return 0;
} /* End of sentinel */

// rejoins the parent process with a child of the given tag
int 
P1_Join(int tag, int *pid, int *status) 
{
	checkIfIsKernel();
    int result = P1_SUCCESS;
	
	int interruptsWereEnabled = P1DisableInterrupts();
	P1_ProcInfo info;
	do {
		result = P1GetChildStatus(tag, pid, status);
		if (result == P1_INVALID_TAG || result == P1_NO_CHILDREN) return result;
		else if (result == P1_NO_QUIT) {
			P1SetState(P1_GetPid(), P1_STATE_JOINING, -1);
			P1Dispatch(FALSE);
		} else break;
		
		result = P1_GetProcInfo(P1_GetPid(), &info);
		assert(result == P1_SUCCESS);
		if (info.numChildren == 0) break;
	} while (1);

	if (interruptsWereEnabled) P1EnableInterrupts();
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

// validates type of device
int isValidType(int type) {
	return type < USLOSS_MAX_TYPES;
}

// validates unit given the device's type,
// different types have different # of units
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

