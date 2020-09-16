#include <stdio.h>
#include <phase1.h>
#include <stdlib.h>
#include <phase1Int.h>
#include <assert.h>

// finish(): test freeing 49 contexts

static int cids[50];

void
Hello(void *arg)
{
	int id = (int)arg;
	int retval = P1ContextSwitch(cids[id+1]);
        assert(retval == P1_SUCCESS);

}

void FreeContext(void *arg){
    // Free the first 49 contexts and leave the last one since it can't free itself.
    int retval = 0;
    for(int i = 0; i < 49; i++){
        retval = P1ContextFree(cids[i]);
        assert(retval == P1_SUCCESS);
        USLOSS_Console("Context %d freed.\n", i);
    }
    USLOSS_Halt(0);

}
void
startup(int argc, char **argv)
{

    int retval;
    P1ContextInit();

    // Create 48 contexts Hello
    int i = 0;
    for (i = 0; i < 49; i++){
    	retval = P1ContextCreate(Hello, (void *)i, USLOSS_MIN_STACK, &cids[i]);
        if(retval == P1_TOO_MANY_CONTEXTS){
		USLOSS_Console("Creating context %d. Too many contexts created.\n", i+1);
	} else assert(retval == P1_SUCCESS);
    } 

    // Create FreeContext to free the 48 Hello contexts
    retval = P1ContextCreate(FreeContext, NULL, USLOSS_MIN_STACK, &cids[49]);
    assert(retval == P1_SUCCESS);

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

