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

void
startup(int argc, char **argv)
{
    int cid;
    int rc;
    P1ContextInit();
    rc = P1ContextCreate(Output, "Hello World!\n", USLOSS_MIN_STACK, &cid);
    assert(rc == P1_SUCCESS);
    P1EnableInterrupts();
    assert(P1DisableInterrupts());
    assert(!P1DisableInterrupts());
    USLOSS_Halt(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {
	P1ContextFree(0);
}

void finish(int argc, char **argv) {}