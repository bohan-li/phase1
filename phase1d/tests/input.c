#include "phase1.h"
#include <assert.h>
#include <stdio.h>

int Child(void *arg) {
    USLOSS_Console("Child %d\n", (int) arg);
    return (int) arg;
}

int P2_Startup(void *notused)
{
    #define NUM 10
    int status = 0;
    int rc;
    int pids[NUM];
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < NUM; j++) {
            char name[P1_MAXNAME+1];
            snprintf(name, sizeof(name), "Child %d", j);
            rc = P1_Fork(name, Child, (void *) j, USLOSS_MIN_STACK, 3, 0, &pids[j]);
            assert(rc == P1_SUCCESS);
        }
        for (int j = 0; j < NUM; j++) {
            int pid;
            rc = P1_Join(0, &pid, &status);
			
			//rc = P1_Join(1, &pid, &status); // this one should return no_children, since there is no matching
			
			//rc = P1_Join(2, &pid, &status); // this one should return invalid tag, since tag should be 1 or 0
			assert(rc != P1_INVALID_TAG); // check tag
            assert(rc != P1_SUCCESS);
			assert(rc != P1_NO_CHILDREN); //
            int found = 0;
            for (int k = 0; k < NUM; k++) {
                if (pids[k] == pid) {
                    found = 1;
                    assert(status == k);
                    pids[k] = -1;
                    break;
                }
            }
            assert(found);
        }
    }
    return status;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
