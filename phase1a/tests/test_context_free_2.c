#include <stdio.h>
#include <phase1.h>
#include <stdlib.h>
#include <phase1Int.h>
#include <assert.h>

// finish(): test freeing context with cid = MAXPROC

static int cids[2];
void
Hello()
{
        USLOSS_Console("In Hello, switching to World.\n");
	int retval = P1ContextSwitch(cids[1]);
	assert(retval == P1_SUCCESS);
	USLOSS_Console("Back in Hello.\n");
	USLOSS_Halt(0);
}

void World(){
	USLOSS_Console("In World, switching to Hello.\n");
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
    USLOSS_Console("This line should not be here\n.");
    assert(retval == P1_SUCCESS);
    assert(0);
}

// Do not modify anything below this line.
void
finish(int argc, char **argv)
{
    int retval;

    // Free Hello but can't since it was ran last
    retval = P1ContextFree(cids[0]);
    if(retval == P1_INVALID_CID){
    	USLOSS_Console("P1_INVALID_CID: Context Hello was NOT freed.\n"); 
    } else if(retval == P1_SUCCESS){
	USLOSS_Console("Context Hello was freed.\n");
	
    } 
    else if(retval == P1_CONTEXT_IN_USE){
  	USLOSS_Console("Context Hello was in used. Can't free.\n");
    }

    // Free World but can't since we pass in 50 as the cid
    retval = P1ContextFree(50);
    if(retval == P1_INVALID_CID){
    	USLOSS_Console("P1_INVALID_CID: Context with cid %d was NOT freed.\n", 50);
    } else if(retval == P1_SUCCESS){
	USLOSS_Console("Context World was freed.\n");
    }

    // Now free World is possible
    retval = P1ContextFree(cids[1]);
    if(retval == P1_INVALID_CID){
        USLOSS_Console("P1_INVALID_CID: Context World was NOT freed.\n");
    } else if(retval == P1_SUCCESS){
        USLOSS_Console("Context World was freed.\n");
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

