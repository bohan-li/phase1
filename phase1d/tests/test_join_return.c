/*
  test case for join
  output should be

  ---------Starting Test 11 ----------
global 6
 ---------Ending Test 11 ----------
*/

#include "phase1.h"
#include <stdio.h>
#include <assert.h>

int global = 1;

int child(void *arg) {
	global += (int) arg;
	USLOSS_Console("child %d\n", global);
	return global;
}

int P2_Startup(void *no){

  USLOSS_Console(" \n---------Starting Test 11 ----------\n");
  int status;
  int rc;
  int pid;
  void *arg = (void *) 5;

  rc = P1_Fork("invalid", child, arg, USLOSS_MIN_STACK, -1, 0, &pid);
  assert(rc == P1_INVALID_PRIORITY);
  rc = P1_Fork("invalid", child, arg, USLOSS_MIN_STACK, 3, 7, &pid);
  assert(rc == P1_INVALID_TAG);
  rc = P1_Fork(NULL, child, arg, USLOSS_MIN_STACK, 4, 0, &pid);
  assert(rc == P1_NAME_IS_NULL);
  
  rc = P1_Join(0, &pid, &status);
  assert(rc == P1_NO_CHILDREN);
  
  rc = P1_Fork(NULL, child, arg, USLOSS_MIN_STACK, 4, 1, &pid);
  assert(rc == P1_NAME_IS_NULL);
  rc = P1_Fork("valid", child, arg, USLOSS_MIN_STACK, 4, 1, &pid);
  assert(rc == P1_SUCCESS);
  rc = P1_Join(0, &pid, &status);
  assert(rc == P1_NO_CHILDREN);
  rc = P1_Join(1, &pid, &status);
  assert(rc == P1_SUCCESS);
  assert(global == 6);
  USLOSS_Console(" ---------Ending Test 11 ----------\n");
  return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
