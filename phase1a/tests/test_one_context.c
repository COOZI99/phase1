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
    int cid;
    
    int rc;
    P1ContextInit();
    rc = P1ContextCreate(Output, "Hello World!\n", USLOSS_MIN_STACK, &cid);
    assert(rc == P1_SUCCESS);

    int invalidCid1 = 48;
    int invalidCid2 = 50;
    int invalidCid3 = -2;
    rc = P1ContextSwitch(invalidCid1);
    assert(rc == P1_INVALID_CID);
    rc = P1ContextSwitch(invalidCid2);
    assert(rc == P1_INVALID_CID);
    rc = P1ContextSwitch(invalidCid3);
    assert(rc == P1_INVALID_CID);

    rc = P1ContextSwitch(cid);
    // should not return
    assert(rc == P1_SUCCESS);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}