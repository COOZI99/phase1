#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>
#include "tester.h"

// Tests for valid priorities of processes
static int foo = 0;

int
Child(void *arg)
{   
    int     rc;
    int     child;
    int     priority = (int) arg;
    if (priority != 1) {
        char name[100];
        int  x = priority - 1;
        snprintf(name, sizeof(name), "Child%d", x);
        rc = P1_Fork(name, Child, (void *) x, USLOSS_MIN_STACK, x, 0, &child);
        assert(rc == P1_SUCCESS);

    }
    assert(foo == (priority-1));
    foo = priority;
    return priority;
}

int P6Proc(void *arg){
    int pid;
    int rc = P1_Fork("Child5", Child, (void *) 5, USLOSS_MIN_STACK, 5, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(foo == 5);
    USLOSS_Console("Test passed.\n");
    USLOSS_Halt(0);
    return 0;
}


void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    P1ProcInit();
    rc = P1_Fork("P6Proc", P6Proc, (void *) 6, USLOSS_MIN_STACK, 6, 0, &pid);    
    assert(rc == P1_SUCCESS);
    // should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
