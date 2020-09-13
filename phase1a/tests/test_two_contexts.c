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
    int cid1, cid2;
    int rc;
    USLOSS_Console("Hello\n");
    P1ContextInit();
    rc = P1ContextCreate(Output, "Hello World!\n", USLOSS_MIN_STACK, &cid1);
    assert(rc == P1_SUCCESS);
    rc = P1ContextCreate(Output, "Goodbye.\n", USLOSS_MIN_STACK+1, &cid2);

    rc = P1ContextSwitch(cid2);
    // should not return
    assert(rc == P1_SUCCESS);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}