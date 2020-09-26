#include <stdio.h>
#include <phase1.h>
#include <stdlib.h>
#include <phase1Int.h>
#include <assert.h>

//startup(): Test creating 51 contexts, only 50 should be created.

static int cids[51];
void

Hello(void *arg)
{
	int id = (int)arg;
        USLOSS_Console("CID: %d\n", id);
	int retval = P1ContextSwitch(cids[id+1]);
	if(retval == P1_INVALID_CID){
		USLOSS_Console("P1_INVALID_CID in Hello.\n");
	}
 	else if (retval == P1_SUCCESS){
	}
	USLOSS_Halt(0);
}


void
startup(int argc, char **argv)
{

    int retval;
    P1ContextInit();

    int i = 0;
    for (i = 0; i < 51; i++){
    	retval = P1ContextCreate(Hello, (void *)i, USLOSS_MIN_STACK, &cids[i]);
        if(retval == P1_TOO_MANY_CONTEXTS){
		USLOSS_Console("Context: %d. Too many contexts created.\n", i);
	} else assert(retval == P1_SUCCESS);
    } 
    retval = P1ContextSwitch(cids[0]);
 
    USLOSS_Console("This line should not be here\n.");
    assert(retval == P1_SUCCESS);
    assert(0);
}

// Do not modify anything below this line.

void
finish(int argc, char **argv)
{
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

