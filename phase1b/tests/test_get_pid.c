#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include "usloss.h"
#include <stdlib.h>

// Test getting PID of the current running process
// There isn't a requirement that they be allocated sequentially, just that they 
// are in the unique and in the correct range.

int Child(void *arg){
	USLOSS_Console("In Child proc\n");
	int val = P1_GetPid();
    assert(val == 2);
	USLOSS_Console("Passed ChildPID check.\n");
	return 0;

}

static int
Output(void *arg) 
{
    char *msg = (char *) arg;
    int val, child;
    USLOSS_Console("%s", msg);
   
    val = P1_GetPid();
    assert(val == 1);
    USLOSS_Console("Passed ParentPID check.\n");
    val = P1_Fork("Child", Child, (void *) 0, USLOSS_MIN_STACK, 1, 0, &child);
    assert(val == P1_SUCCESS);
    USLOSS_Console("TEST PASSED\n");
    USLOSS_Halt(0);
    // should not return
    return 0;
}


int P6Proc(void *arg){
    int pid;
    int rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 2, 0, &pid);
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
    rc = P1_Fork("P6Proc", P6Proc, "Hello World!\n", USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
