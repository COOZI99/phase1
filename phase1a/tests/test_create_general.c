#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

static void
Output(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    USLOSS_Halt(0);
}

void
startup(int argc, char **argv)
{
    // P1EnableInterrupts();
    // int result;
    // unsigned int val = USLOSS_PsrGet() & ~(USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE);
    // result = USLOSS_PsrSet(val);
    // assert(result == USLOSS_ERR_OK);
    int cid;
    
    int rc;
    P1ContextInit();
    rc = P1ContextCreate(Output, "Hello World!\n", USLOSS_MIN_STACK, &cid);
    assert(rc == P1_SUCCESS);
    rc = P1ContextCreate(Output, "Hello Words\n", 0, &cid);
    assert(rc == P1_INVALID_STACK);

    int i;
    for (i=0; i<P1_MAXPROC-1; i++) {
        rc = P1ContextCreate(Output, "Hello World 2!\n", USLOSS_MIN_STACK, &cid);
        assert(rc == P1_SUCCESS);
    }
    rc = P1ContextCreate(Output, "Goodbye\n", USLOSS_MIN_STACK, &cid);
    assert(rc == P1_TOO_MANY_CONTEXTS);

    rc = P1ContextSwitch(cid);
    // should not return
    assert(rc == P1_SUCCESS);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}