#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include "usloss.h"
#include "stdlib.h"

/* P1SetState()
 * Tests setting Parent's state to P1_STATE_BLOCKED at sem 42
 * The Parent process should not run to completion
 */ 

int Child(void *arg){
	USLOSS_Console("In Child proc\n");
        USLOSS_Console("Child sets parent's state to P1_STATE_BLOCKED at sid 42\n");
	
        // set Hello to be not runnable that is blocked at sem 42
	int val = P1SetState(1, P1_STATE_BLOCKED, 42);
    	if(val != P1_SUCCESS){
          USLOSS_Console("TEST FAILED\n");
          USLOSS_Halt(0);
        }
	
	// check if the sem is correct
	P1_ProcInfo *info = malloc(sizeof(P1_ProcInfo));
	val = P1_GetProcInfo(1, info);
	assert(val == P1_SUCCESS);
	if(info->sid != 42){
           USLOSS_Console("TEST FAILED\n");
           USLOSS_Halt(0);
        }
	free(info);
	USLOSS_Console("Parent is now blocked on sem 42\n");
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

    USLOSS_Console("Parent now forks a Child with priority 1\n");
    val = P1_Fork("Child", Child, (void *) 0, USLOSS_MIN_STACK, 1, 0, &child);
    assert(val == P1_SUCCESS);
    int status;
    val = P1GetChildStatus(0, &status, &child);
    USLOSS_Console("!!!Should not get here since Parent is blocked\n");
    USLOSS_Console("TEST FAILED\n");
    USLOSS_Halt(0);
    // should not return
    return 0;
}

int P6Proc(void *arg){
    int pid;
    USLOSS_Console("P6Proc forks parent Hello with priority 2\n");
    int rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("TEST PASSED\n");
    USLOSS_Halt(0);
    return 0;
}


void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    P1ProcInit();
    USLOSS_Console("startup\n");
    USLOSS_Console("Forks P6Proc with priority 6\n");
    rc = P1_Fork("P6Proc", P6Proc, (void *) 2, USLOSS_MIN_STACK, 6, 0, &pid);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {
}
