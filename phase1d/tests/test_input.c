/*
This case is mainly to test output of test_join
If there are three invalid children, test_join will return no_children
If there are two invalid children and one valid child, test_join will return success

*/
#include "phase1.h"
#include <assert.h>
#include <stdio.h>

int child(void *arg) {
    USLOSS_Console("Child %d\n", (int) arg);
    return (int) arg;
}

int P2_Startup(void *notused)
{
 int status;
  int rc;
  int pid;
  void *arg = (void *) 5;
    rc = P1_Fork("invalid", child, arg, USLOSS_MIN_STACK, -1, 0, &pid);
		
		
	rc = P1_Fork("invalid", child, arg, USLOSS_MIN_STACK, 3, 7, &pid);
		
	
	//rc = P1_Fork("child", child, NULL, USLOSS_MIN_STACK, 4 , 0, &pid); //this with two invalid, will return success
	
	rc = P1_Fork("invalid", child, arg, USLOSS_MIN_STACK, 3, 7, &pid); // this with two invalid, will return no_children
    

      rc = P1_Join(0, &pid, &status);
			
			//rc = P1_Join(1, &pid, &status); // this one should return no_children, since there is no matching
			
			//rc = P1_Join(2, &pid, &status); // this one should return invalid tag, since tag should be 1 or 0
			assert(rc != P1_INVALID_TAG); // check tag
            assert(rc != P1_SUCCESS);
			assert(rc != P1_NO_CHILDREN); //
       
    
    return status;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
