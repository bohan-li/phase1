#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

/*
This test case creates a first process Output, forks it to another process forked, and creates two new child processes c1, c2 in the forked.
Forked has highest priority and c1, c2 have lowest priority, so it will create c1, c2, finish, then add the children to Output.
Since join is not yet implemented, output should terminate with children, producing an error.
*/
void printState(P1_State state) {
	switch (state) {
	case P1_STATE_READY: 
		USLOSS_Console("ready\n");
		break;
	case P1_STATE_FREE: 
		USLOSS_Console("free\n");
		break;
	case P1_STATE_RUNNING: 
		USLOSS_Console("running\n");
		break;
	case P1_STATE_QUIT: 
		USLOSS_Console("quit\n");
		break;
	case P1_STATE_BLOCKED: 
		USLOSS_Console("blocked\n");
		break;
	case P1_STATE_JOINING:
		USLOSS_Console("joining\n");
		break;
	default: USLOSS_Console("NOT A VALID STATE\n");
	}
}


static int childFunc(void *arg) {
	return 12;
}

static int forked(void *arg) {
	char *msg = (char *)arg;

	USLOSS_Console("%s", msg);

	int priority = 1, tag = 1;
	int rc;
	P1_ProcInfo info;
	int pid;

	// child 1
	rc = P1_Fork("c1", childFunc, arg, USLOSS_MIN_STACK, priority, tag, &pid);
	assert(rc == P1_SUCCESS);
	rc = P1_GetProcInfo(pid, &info);
	assert(rc == P1_SUCCESS);
	assert(pid == 2);

	assert(info.state == P1_STATE_READY);
	assert(info.priority == priority);
	assert(info.tag == tag);
	assert(info.parent == 1);
	assert(info.numChildren == 0);

	rc = P1_GetProcInfo(P1_GetPid(), &info);
	assert(rc == P1_SUCCESS);
	assert(info.numChildren == 1);

	// child 2
	rc = P1_Fork("c2", childFunc, arg, USLOSS_MIN_STACK, priority, tag, &pid);

	assert(rc == P1_SUCCESS);
	rc = P1_GetProcInfo(pid, &info);
	assert(rc == P1_SUCCESS);
	assert(pid == 3);
	assert(info.state == P1_STATE_READY);
	assert(info.priority == priority);
	assert(info.tag == tag);
	assert(info.parent == 1);
	assert(info.numChildren == 0);

	rc = P1_GetProcInfo(P1_GetPid(), &info);
	assert(rc == P1_SUCCESS);
	assert(info.numChildren == 2);

	return 222;
}

// first process
static int
Output(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
	int priority = 5;
	int pid;
	int rc = P1_Fork("forked", forked, "Forked\n", USLOSS_MIN_STACK, priority, 0, &pid);
	assert(rc == P1_SUCCESS);
	P1_ProcInfo info;
	rc = P1_GetProcInfo(pid, &info);
	assert(rc == P1_SUCCESS);
	assert(pid == 1);
	assert(info.state == P1_STATE_QUIT);
	assert(info.priority == priority);
	assert(info.tag == 0);
	assert(info.parent == 0);
	assert(info.numChildren == 2);

	rc = P1_GetProcInfo(P1_GetPid(), &info);
	assert(rc == P1_SUCCESS);
	assert(info.numChildren == 3);

	int status;
	rc = P1GetChildStatus(0, &pid, &status);
	assert(rc == P1_SUCCESS);
	assert(pid == 1);
	assert(status == 222);

	rc = P1_GetProcInfo(P1_GetPid(), &info);
	assert(rc == P1_SUCCESS);
	assert(info.numChildren == 2);
    return 0;
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    P1ProcInit();
    USLOSS_Console("startup\n");
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 4, 0, &pid);
    assert(rc == P1_SUCCESS);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}