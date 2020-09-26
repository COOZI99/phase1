/*
 * Tests that functions do not blindly enable interrupts.
 */


#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>

static void
Fail(void) 
{
    USLOSS_Console("TEST FAILED.\n");
    USLOSS_Halt(0);
}

static int
Child(void *arg)
{
    return 11;
}

static int
Tester(void *arg) 
{
    int rc, pid, status;
    P1_ProcInfo info;

    // disable interrupts
    rc = USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);
    assert(rc == USLOSS_ERR_OK);

    // get our PID

    pid = P1_GetPid();

    // make sure interrupts are still off

    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_INT) != 0) {
        Fail();
    }

    // Get our process info

    rc = P1_GetProcInfo(pid, &info);
    assert(rc == P1_SUCCESS);

    // make sure interrupts are still off

    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_INT) != 0) {
        Fail();
    }

    // fork a child (at higher priority)
    rc = P1_Fork("Child", Child, NULL, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);

    // make sure interrupts are still off

    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_INT) != 0) {
        Fail();
    }

    // get child's status

    rc = P1GetChildStatus(0, &pid, &status);
    assert(rc == P1_SUCCESS);

    // make sure interrupts are still off

    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_INT) != 0) {
        Fail();
    }

    USLOSS_Console("TEST PASSED.\n");
    return 4;
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    P1ProcInit();
    rc = P1_Fork("Tester", Tester, NULL, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
