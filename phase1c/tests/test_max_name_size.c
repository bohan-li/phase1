#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <string.h>

static int flag = 0;
static int global = 3;

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

    char name[15];
    sprintf(name, "Unblocks%d", global);
    rc = P1_Fork(name, Unblocks, (void *) sem, USLOSS_MIN_STACK, 5, 0, &pid);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("P on semaphore, global %d.\n", global);
    rc = P1_P(sem);
    assert(rc == P1_SUCCESS);
    assert(flag == 1);
}

static int
Helper(void *arg) 
{
    int sem = (int) arg;
    int rc;
    int pid;
    char name[15];
    sprintf(name, "Blocks%d", global);
    rc = P1_Fork(name, Blocks, (void *) sem, USLOSS_MIN_STACK, global, 0, &pid);
    assert(rc == P1_SUCCESS);
    global--;
    sprintf(name, "Blocks%d", global);
    rc = P1_Fork(name, Blocks, (void *) sem, USLOSS_MIN_STACK, global, 0, &pid);
    assert(rc == P1_SUCCESS);
    global--;
    sprintf(name, "Blocks%d", global);
    rc = P1_Fork(name, Blocks, (void *) sem, USLOSS_MIN_STACK, global, 0, &pid);
    assert(rc == P1_SUCCESS);

    char name1[P1_MAXNAME + 1];
    rc = P1_SemName(sem, name1);
    assert(rc == P1_SUCCESS);
    assert(strcmp(name1, "sem") == 0);

    rc = P1_SemFree(sem);
    assert(rc == P1_SUCCESS);
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