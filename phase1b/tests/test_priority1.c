#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

// Test forking a process with priority 7

int P6Proc(void *arg){
   return 0; 	
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    P1ProcInit();
    USLOSS_Console("startup\n");
    rc = P1_Fork("P6Proc", P6Proc, "Hello World!\n", USLOSS_MIN_STACK, 7, 0, &pid);
    if(rc == P1_INVALID_PRIORITY){
	USLOSS_Console("TEST PASSED\n");
	USLOSS_Halt(0);
    } else {
	USLOSS_Console("TEST FAILED\n");
        USLOSS_Halt(0);
    }
    
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
