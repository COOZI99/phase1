#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include "usloss.h"
#include <stdlib.h>

/* P1SetState()
 * Tests setting child's state to P1_STATE_QUIT after it has called P1_QUIT()
 */ 
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

    USLOSS_Console("Parent now forks a Child with priority 1\n");
    val = P1_Fork("Child", Child, (void *) 0, USLOSS_MIN_STACK, 1, 0, &child);
    assert(val == P1_SUCCESS);

    USLOSS_Console("Setting Child's state to P1_STATE_QUIT\n");
    // set Child's state to P1_STATE_QUIT
    val = P1SetState(2, P1_STATE_QUIT, 0);
    if(val != P1_SUCCESS){
        USLOSS_Console("TEST FAILED\n");
        USLOSS_Halt(0);
    }
    
    P1_ProcInfo *info = malloc(sizeof(P1_ProcInfo));
    val = P1_GetProcInfo(2, info);
    assert(val == P1_SUCCESS);
    if(info->state != P1_STATE_QUIT){
        USLOSS_Console("TEST FAILED\n");
        USLOSS_Halt(0);
    }
    free(info);
    USLOSS_Console("Parent quitting\n");
    USLOSS_Console("TEST PASSED\n");
    USLOSS_Halt(0);
    // should not return
    return 0;
}

int P6Proc(void *arg){
    int pid;
    USLOSS_Console("P6Proc forks parent Hello with priority 2\n");
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
    USLOSS_Console("Forks P6Proc with priority 6\n");
    rc = P1_Fork("P6Proc", P6Proc, (void *) 2, USLOSS_MIN_STACK, 6, 0, &pid);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {
    int val = P1SetState(0, P1_STATE_QUIT, 0);
    assert(val == P1_SUCCESS);
}
