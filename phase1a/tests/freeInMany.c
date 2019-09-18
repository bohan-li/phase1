#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include "usloss.h"

int thisNum;

static void Output(void *arg) 
{
    int msg = (int ) arg;

    USLOSS_Console("insert in free context: %d\n", msg);
	if (thisNum == P1_MAXPROC - 1) {
		USLOSS_Halt(0);
	}
	else {
		USLOSS_Console("here is %d\n", thisNum);
		thisNum += 1;
		P1ContextSwitch(thisNum); 
 	}
}


static void printInfo(void *integer) {
	thisNum = *(int*)integer;
	USLOSS_Console("printInfo %d\n", *(int*)integer);
	if (thisNum == P1_MAXPROC - 1) {
		USLOSS_Halt(0);
	}
	else {
		USLOSS_Console("here is %d\n", thisNum);
		thisNum += 1;
		P1ContextSwitch(thisNum);
	}
}

void
startup(int argc, char **argv)
{
	P1ContextInit();
	int cid, rc;
	int args[P1_MAXPROC];
	for (int i = 0; i < P1_MAXPROC; i++) {
		args[i] = i;
		rc = P1ContextCreate(printInfo, &args[i], USLOSS_MIN_STACK, &cid);
	}
	
	//rc = P1ContextCreate(Output, (void *)123, USLOSS_MIN_STACK, &cid); 
	//assert(rc == P1_TOO_MANY_PROCESSES); 
	//assert(rc == P1_SUCCESS);
	//line 48 ~ 50: it will throw an error before free. Done.
	
	P1ContextFree(7);  
	// try to find a free context
	rc = P1ContextCreate(Output, (void *)123, USLOSS_MIN_STACK, &cid);
	assert(rc == P1_SUCCESS);

    rc = P1ContextSwitch(0);

    // should not return
    assert(rc == P1_SUCCESS);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {
	USLOSS_Console("here\n");
	for (int i = 0; i < P1_MAXPROC; i++) P1ContextFree(i);
	USLOSS_Console("made it here\n");
}

void finish(int argc, char **argv) {}