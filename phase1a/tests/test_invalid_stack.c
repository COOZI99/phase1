#include <stdio.h>
#include <phase1.h>
#include <stdlib.h>
#include <phase1Int.h>
#include <assert.h>

// startup(): Create a context with stack size < USLOSS_MIN_STACK

static int cids[2];
void
Hello()
{
        USLOSS_Console("Printing Hello in context Hello.\n");
	USLOSS_Halt(0);
}

void World(){
	USLOSS_Console("World\n");
}
void
startup(int argc, char **argv)
{

    int retval;
    P1ContextInit();
    
    retval = P1ContextCreate(Hello, NULL, USLOSS_MIN_STACK, &cids[0]);
    assert(retval == P1_SUCCESS);

    retval = P1ContextCreate(World, NULL, USLOSS_MIN_STACK - 1, &cids[1]);
    if(retval == P1_INVALID_STACK){
        USLOSS_Console("Context World: stacksize is less than USLOSS_MIN_STACK.\n");
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

