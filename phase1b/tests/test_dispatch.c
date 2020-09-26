#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include "usloss.h"
#include <stdlib.h>

// Tests calling P1Dispatch(TRUE) with 3 processes of the same priority
int Child2(void *arg){
        USLOSS_Console("In Child2 proc\n");
        USLOSS_Console("Child2 proc quitting\n");
        P1_Quit(13);
        return 0;


}

int Child(void *arg){
	USLOSS_Console("In Child proc\n");	
	USLOSS_Console("Child proc quitting\n");
	P1_Quit(12);	
	return 0;

}

static int
Output(void *arg) 
{
    char *msg = (char *) arg;
    int val, child;
    USLOSS_Console("%s", msg);
  
    USLOSS_Console("Parent now forks Child with priority 3\n"); 
    val = P1_Fork("Child", Child, (void *) 0, USLOSS_MIN_STACK, 3, 0, &child);
    assert(val == P1_SUCCESS);

    USLOSS_Console("Parent now forks Child2 with priority 3\n"); 
    val = P1_Fork("Child2", Child2, (void *) 0, USLOSS_MIN_STACK, 3, 0, &child);
    assert(val == P1_SUCCESS);
    
    USLOSS_Console("Parent calling P1Dispatch(TRUE)\n");
    P1Dispatch(TRUE);	
 
    int pid,  status;
    val = P1GetChildStatus(0, &pid, &status);
    assert(val == P1_SUCCESS);
    assert(pid == 2);
    assert(status == 12);

    val = P1GetChildStatus(0, &pid, &status);
    assert(val == P1_SUCCESS);
    assert(pid == 3);
    assert(status == 13);

    USLOSS_Console("Parent quitting\n");
    USLOSS_Console("TEST PASSED.\n");
    USLOSS_Halt(0);
    // should not return
    return 0;
}


int P6Proc(void *arg){
    int pid;
    USLOSS_Console("Forking parent Hello with priority 3\n");
    int rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 3, 0, &pid);
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
