#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

static void
Output(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    USLOSS_Halt(0);
}

static void printInfo(void *integer) {
	int thisNum = *(int*)integer;
	USLOSS_Console("printInfo %d\n", thisNum);
	if (thisNum == P1_MAXPROC - 1) {
		USLOSS_Halt(0);
	}
	else {
		P1ContextSwitch(thisNum + 1);
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
		assert(rc == P1_SUCCESS);
		assert(cid == i);
	}
	rc = P1ContextCreate(printInfo, &args[0], USLOSS_MIN_STACK, &cid);
	assert(rc == P1_TOO_MANY_PROCESSES);
    rc = P1ContextSwitch(0);
    // should not return
    assert(rc == P1_SUCCESS);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {
	USLOSS_Console("here\n");
	for (int i = 0; i < P1_MAXPROC; i++) P1ContextFree(i);
	printf("made it here");
}

void finish(int argc, char **argv) {}