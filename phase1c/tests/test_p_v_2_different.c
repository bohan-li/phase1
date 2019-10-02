#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

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

    USLOSS_Console("P on semaphore.\n");
    rc = P1_P(sem);
    assert(rc == P1_SUCCESS);
    assert(flag == 1);

    return 0;
}

static int
Helper(void *arg) 
{
    int sem = (int) arg;
    int rc;
    int pid;
    int s1, s2;

    rc = P1_Fork("Blocks", Blocks, (void *) sem, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    s1 = sem;
    rc = P1_SemCreate("sem1", 0, &sem);
    assert(rc == P1_SUCCESS);
    s2 = sem;
    rc = P1_Fork("Blocks1", Blocks, (void *) sem, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);

    rc = P1_Fork("Unblocks", Unblocks, (void *) s2, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);

    rc = P1_Fork("Unblocks1", Unblocks, (void *) s1, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);

    rc = P1_SemFree(s1);
    assert(rc == P1_SUCCESS);

    rc = P1_SemFree(s2);
    assert(rc == P1_SUCCESS);

    USLOSS_Console("Test passed.\n");
    USLOSS_Halt(0);
    // should not return
    return 0;
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    int sem;

    P1SemInit();
    rc = P1_SemCreate("sem", 0, &sem);
    assert(rc == P1_SUCCESS);
    // Blocks blocks then Unblocks unblocks it
    rc = P1_Fork("Helper", Helper, (void *) sem, USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}