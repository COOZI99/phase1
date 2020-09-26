#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include "usloss.h"


/* P1SetState(),
 * Tests setting process's state by passing invalid PIDs
 */
static int
Output(void *arg) 
{
    char *msg = (char *) arg;
    USLOSS_Console("%s", msg);
    int val;
    USLOSS_Console("Setting process with pid -1 to P1_STATE_JOINING\n");
    val = P1SetState(-1, P1_STATE_JOINING, 0);
    if(val != P1_INVALID_PID){
        USLOSS_Console("TEST FAILED\n");
        USLOSS_Halt(0);
    }
    
    USLOSS_Console("Return value is P1_INVALID_PID\n");

    USLOSS_Console("Setting process with pid 50 to P1_STATE_JOINING\n");
    val = P1SetState(50, P1_STATE_JOINING, 0);
    if(val != P1_INVALID_PID){
        USLOSS_Console("TEST FAILED\n");
        USLOSS_Halt(0);
    }
    USLOSS_Console("Return value is P1_INVALID_PID\n");

    USLOSS_Console("Setting process with pid 3 to P1_STATE_READY\n");
    val = P1SetState(3, P1_STATE_READY, 0);
    if(val != P1_INVALID_PID){
        USLOSS_Console("TEST FAILED\n");
        USLOSS_Halt(0);
    }   
    USLOSS_Console("Return value is P1_INVALID_PID\n");
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
    assert(rc == P1_SUCCESS);
        

    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
