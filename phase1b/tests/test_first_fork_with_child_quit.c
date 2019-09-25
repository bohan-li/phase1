#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>


static int forked(void *arg) {
	char *msg = (char *)arg;

	USLOSS_Console("%s", msg);
	return 1;
}

static int
Output(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
	int pid;
	int rc = P1_Fork("forked", forked, "Forked\n", USLOSS_MIN_STACK, 5, 0, &pid);
	assert(rc == P1_SUCCESS);
    return 0;
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    P1ProcInit();
    USLOSS_Console("startup\n");
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}