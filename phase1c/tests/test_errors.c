#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int flag = 0;
char* itoa(int val, int base){
	static char buf[32] = {0};
	int i = 30;
	for(; val && i ; --i, val /= base)
	
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
	
}
static int
Unblocks(void *arg)
{
    int sem = (int) arg;
    int rc;
USLOSS_Console("help\n");
    flag = 1;
    USLOSS_Console("V on semaphore.\n");
	rc = P1_SemFree(-1);
	assert(rc == P1_INVALID_SID);
	rc = P1_SemFree(sem);
	assert(rc == P1_BLOCKED_PROCESSES);
    rc = P1_V(sem);
    assert(rc == P1_SUCCESS);
	rc = P1_SemFree(sem);
	assert(rc == P1_SUCCESS);
	rc = P1_SemFree(sem);
	assert(rc == P1_INVALID_SID);
    USLOSS_Console("Test passed.\n");
    USLOSS_Halt(0);
    return 12;
}

static int
Blocks(void *arg) 
{
	USLOSS_Console("help2\n");
    int sem = (int) arg;
    int rc;
    int pid;

    rc = P1_Fork("Unblocks", Unblocks, (void *) 0, USLOSS_MIN_STACK, 2, 0, &pid);
				
    assert(rc == P1_SUCCESS);
    USLOSS_Console("P on semaphore.\n");
    rc = P1_P(-1);
    assert(rc == P1_INVALID_SID);
	rc = P1_P(P1_MAXSEM);
    assert(rc == P1_INVALID_SID);
	rc = P1_P(sem);
    assert(rc == P1_SUCCESS);
    assert(flag == 1);
    
    return 0;
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    int sem;
	int s;
	int i;
	int j;
	char * buffer;
	P1ProcInit();
    P1SemInit();
	
    rc = P1_SemCreate(NULL, 0, &sem);
    assert(rc == P1_NAME_IS_NULL);
	rc = P1_SemCreate("012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, &sem);
    assert(rc == P1_NAME_TOO_LONG);
	rc = P1_SemCreate("12345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, &sem);
    assert(rc == P1_SUCCESS);
	s=sem;
	rc = P1_SemCreate("12345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, &sem);
    assert(rc == P1_DUPLICATE_NAME);
	for (i=0;i<P1_MAXSEM-1;i++){
		buffer = itoa(i, 10);
		rc = P1_SemCreate(buffer, 0, &sem);
		assert(rc == P1_SUCCESS);
	}
	buffer = itoa(i, 10);
	rc = P1_SemCreate(buffer, 0, &sem);
	
	assert(rc == P1_TOO_MANY_SEMS);
    // Blocks blocks then Unblocks unblocks it
	USLOSS_Console("help\n");

    rc = P1_Fork("Blocks", Blocks, (void *) 0, USLOSS_MIN_STACK, 1, 0, &pid);
	USLOSS_Console("help1\n");
    assert(rc == P1_SUCCESS);
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}