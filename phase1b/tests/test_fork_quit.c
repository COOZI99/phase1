#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdlib.h>

// Test forking and quitting 
static int
Output(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    P1_Quit(11);
    // should not return
    assert(0);
    return 0;
}

static int
Parent(void *arg)
{   
    int     pid;
    int     rc;
    int     status;
    int     child;
    char    *msg = (char *) arg;

    // child has higher priority and should run to completion.
    rc = P1_Fork("Child", Output, msg, USLOSS_MIN_STACK, 1, 0, &child);
    assert(rc == P1_SUCCESS);
    rc = P1GetChildStatus(0, &pid, &status);
    assert(rc == P1_SUCCESS);
    assert(pid == child);
    assert(status == 11);
    return 0;
}


int P6Proc(void *arg){
    int pid;
    int rc = P1_Fork("Parent", Parent, "Hello World!\n", USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);
    int rv, status, childpid;
    rv = P1GetChildStatus(0, &childpid, &status);
    assert(rv == P1_SUCCESS);
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
    rc = P1_Fork("P6Proc", P6Proc, "Hello World!\n", USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS);
    // should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
