#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

// Test forking a process
static int
Output(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg); 
    P1_Quit(11);
    // should not return
    return 0;
}

int P6Proc(void *arg){
    int pid;
    int rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    int status;
    rc = P1GetChildStatus(0, &pid, &status);
    USLOSS_Console("TEST PASSED\n");
 	
    return 0;
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    P1ProcInit();
    USLOSS_Console("startup\n");
    // fork the first process with priority 6 so it doesn't fail students' check
    // that priority must be 6 for process with pid = 0
    rc = P1_Fork("P6Proc", P6Proc, "Hello World!\n", USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
