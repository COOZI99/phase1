#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdlib.h>

static int cids[2];


void
Hello(void *arg) 
{
    USLOSS_Console("Hello ");
    int rc = P1ContextSwitch(cids[1]);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("Goodbye.\n");
    USLOSS_Halt(0);
}

void
World(void *arg) 
{
    USLOSS_Console("World!\n");
    int rc = P1ContextSwitch(cids[0]);
    // should not return
    assert(rc == P1_SUCCESS);
    assert(0);
}

void
startup(int argc, char **argv)
{
    int rc;
    P1ContextInit();
    rc = P1ContextCreate(Hello, NULL, USLOSS_MIN_STACK, &cids[0]);
    assert(rc == P1_SUCCESS);
    rc = P1ContextCreate(World, NULL, USLOSS_MIN_STACK, &cids[1]);
    USLOSS_Console("%d, %d", cids[0], cids[1]);
    assert(rc == P1_SUCCESS);
    rc = P1ContextSwitch(cids[0]);
    // should not return
    assert(rc == P1_SUCCESS);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}