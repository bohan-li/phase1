#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

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

int global=0;
static int
Output2(void *arg) 
{
	
	global++;
    return 0;
}

static int
Output1(void *arg) 
{
	int pid1;
	int pid2;
	int pid3;
	int pid4; 	
	int rc,rc2;
	int i;
	int status;
	for (i=0;i<P1_MAXPROC*100;i++){
		rc = P1_Fork("2", Output2, "Hello \n", USLOSS_MIN_STACK, 1, 0, &pid1);
		//USLOSS_Console("%d %d\n", pid1, rc2);
		P1_ProcInfo info;
		P1_GetProcInfo(pid1, &info);

		rc2 = P1GetChildStatus(0, &pid1, &status);
		//printState(info.state);

		assert(global=i+1);
		assert(pid1 == 1);
		assert(rc == P1_SUCCESS);
		assert(rc2 == P1_SUCCESS);
	}
	assert(global==P1_MAXPROC*100);
	for (i=0;i<P1_MAXPROC*100;i++){
		rc = P1_Fork("2", Output2, "Hello \n", USLOSS_MIN_STACK, 2, 0, &pid1);
		
		rc2 = P1GetChildStatus(0, &pid1, &status);
		assert(global=i+1);
		assert(pid1 == 1);
		assert(rc == P1_SUCCESS);
		assert(rc2 == P1_SUCCESS);
	}
	assert(global==P1_MAXPROC*100);
	global=0;
	for (i=0;i<P1_MAXPROC*100;i++){
		rc = P1_Fork("2", Output2, "Hello \n", USLOSS_MIN_STACK, 3, 0, &pid1);
		
		rc2 = P1GetChildStatus(0, &pid1, &status);
		assert(global=i+1);
		assert(pid1 == 1);
		assert(rc == P1_SUCCESS);
		assert(rc2 == P1_SUCCESS);
	}
	assert(global==P1_MAXPROC*100);
	global=0;
	for (i=0;i<P1_MAXPROC*100;i++){
		rc = P1_Fork("2", Output2, "Hello \n", USLOSS_MIN_STACK, 4, 0, &pid1);
		
		rc2 = P1GetChildStatus(0, &pid1, &status);
		assert(global=i+1);
		assert(pid1 == 1);
		assert(rc == P1_SUCCESS);
		assert(rc2 == P1_SUCCESS);
	}
	assert(global==P1_MAXPROC*100);
	global=0;
	for (i=0;i<P1_MAXPROC*100;i++){
		rc = P1_Fork("2", Output2, "Hello \n", USLOSS_MIN_STACK, 5, 0, &pid1);
		
		rc2 = P1GetChildStatus(0, &pid1, &status);
		assert(global=i+1);
		assert(pid1 == 1);
		assert(rc == P1_SUCCESS);
		assert(rc2 == P1_SUCCESS);
	}
	assert(global==P1_MAXPROC*100);
    USLOSS_Halt(0);
    return 0;
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    P1ProcInit();
    rc = P1_Fork("1", Output1, "Hello \n", USLOSS_MIN_STACK, 6, 0, &pid);
	USLOSS_Console("OUT%d",rc);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}