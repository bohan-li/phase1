#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>

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
LowPriorityBlock(void *arg)
{
    int sem = (int) arg;
    int rc;
    int pid;

    USLOSS_Console("P on semaphore.\n");
    rc = P1_P(sem);
    USLOSS_Console("Low priority third\n");
    assert(rc == P1_SUCCESS);
    assert(flag == 1);
    return 0;
}

static int
MidPriorityBlock(void *arg)
{
    int sem = (int) arg;
    int rc;
    int pid;


    USLOSS_Console("P on semaphore.\n");
    rc = P1_P(sem);
    USLOSS_Console("Middle priority second\n");
    assert(rc == P1_SUCCESS);
    assert(flag == 1);
    return 0;
}

static int
HighPriorityBlock(void *arg)
{
    int sem = (int) arg;
    int rc;
    int pid;


    USLOSS_Console("P on semaphore.\n");
    rc = P1_P(sem);
    USLOSS_Console("High priority first\n");
    assert(rc == P1_SUCCESS);
    assert(flag == 1);
    return 0;
}

static int
Helper(void *arg)
{
    int sem;
    int rc;
    int pid;

    rc = P1_SemCreate("sem", 0, &sem);
    assert(rc == P1_SUCCESS);
    // Blocks blocks then Unblocks unblocks it
    rc = P1_Fork("LowPriorityBlock", LowPriorityBlock, (void *) sem, USLOSS_MIN_STACK, 4, 0, &pid);
    assert(rc == P1_SUCCESS);

    rc = P1_Fork("HighPriorityBlock", HighPriorityBlock, (void *) sem, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);

    rc = P1_Fork("MidPriorityBlock", MidPriorityBlock, (void *) sem, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);

    rc = P1_Fork("Unblocks", Unblocks, (void *) sem, USLOSS_MIN_STACK, 5, 0, &pid);
    assert(rc == P1_SUCCESS);

    rc = P1_Fork("Unblocks1", Unblocks, (void *) sem, USLOSS_MIN_STACK, 5, 0, &pid);
    assert(rc == P1_SUCCESS);

    rc = P1_Fork("Unblocks2", Unblocks, (void *) sem, USLOSS_MIN_STACK, 5, 0, &pid);
    assert(rc == P1_SUCCESS);

    rc = P1_SemFree(sem);
    assert(rc == P1_SUCCESS);
    USLOSS_Halt(0);
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    int sem;

    P1SemInit();
    rc = P1_Fork("Helper", Helper, NULL, USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

<<<<<<< HEAD
void finish(int argc, char **argv) {}
=======
void finish(int argc, char **argv) {}
>>>>>>> prep for rebase
