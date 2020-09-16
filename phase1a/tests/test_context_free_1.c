#include <stdio.h>
#include <phase1.h>
#include <stdlib.h>
#include <phase1Int.h>
#include <assert.h>

// finish(): test freeing an unused context and -1 cid context

static int cids[2];
void
Hello()
{
        USLOSS_Console("In Hello.\n");
	int retval = P1ContextSwitch(cids[1]);
	assert(retval == P1_SUCCESS);
	USLOSS_Console("Back in Hello.\n");
	USLOSS_Halt(0);
}

void World(){
	USLOSS_Console("In World.\n");
    int retval = P1ContextSwitch(cids[0]);
    assert(retval == P1_SUCCESS);
}
void
startup(int argc, char **argv)
{

    int retval;
    P1ContextInit();
    retval = P1ContextCreate(Hello, NULL, USLOSS_MIN_STACK, &cids[0]);
    assert(retval == P1_SUCCESS);

    retval = P1ContextCreate(World, NULL, USLOSS_MIN_STACK, &cids[1]);
    assert(retval == P1_SUCCESS);
    retval = P1ContextSwitch(cids[0]);

    // should not get to here
    USLOSS_Console("This line should not be here.\n");
    assert(retval == P1_SUCCESS);
    assert(0);
}

// Do not modify anything below this line.

void
finish(int argc, char **argv)
{
    int retval;
    retval = P1ContextFree(2);
    if(retval == P1_INVALID_CID){
    	USLOSS_Console("P1_INVALID_CID: Context with cid %d was NOT freed.\n", 2); 
    }
    retval = P1ContextFree(-1);
    if(retval == P1_INVALID_CID){
    	USLOSS_Console("P1_INVALID_CID: Context with cid %d was NOT freed.\n", -1);
    }
    USLOSS_Console("Goodbye.\n");
}

void
test_setup(int argc, char **argv)
{
    // Do nothing.
}

void
test_cleanup(int argc, char **argv)
{
    // Do nothing.
}

