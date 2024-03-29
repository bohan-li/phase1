#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include<stdio.h> 
static int flag = 0;

static int
Unblocks(void *arg)
{
    int sem = (int) arg;
    int rc;

    flag = 1;
    USLOSS_Console("V on semaphore.\n");
    rc = P1_V(sem);
    assert(rc == P1_SUCCESS);
    // will never get here as Blocks will run and call USLOSS_Halt.
    return 12;
}

static int
Blocks(void *arg) 
{
    int sem = (int) arg;
    int rc;
    int pid;

    USLOSS_Console("here %d\n", sem);
    rc = P1_Fork("Unblocks", Unblocks, (void *) sem, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("P on semaphore.\n");
    rc = P1_P(sem);
    assert(rc == P1_SUCCESS);
    assert(flag == 1);
    USLOSS_Console("Test passed.\n");
    USLOSS_Halt(0);
    // should not return
    assert(0);
    return 0;
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    int sem;

    P1SemInit();
    
	for (int i = 0; i < 50;i++){ //  make P1_MAXSEM sems
		char result[50]; 
		sprintf(result, "%d", i); 
		rc = P1_SemCreate(result, 0, &sem); 
	}
	rc = P1_SemFree(47);
	//*******************************
	//rc = P1_SemFree(47);
	//assert(rc != P1_INVALID_SID); // free twice the same place should throw an error
	//*******************************
	assert(rc == P1_SUCCESS);

	rc = P1_SemCreate("asdf", 0, &sem); 
	assert(rc == P1_SUCCESS);
    assert(sem == 47);
    // Blocks blocks then Unblocks unblocks it
    rc = P1_Fork("Blocks", Blocks, (void *) sem, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
