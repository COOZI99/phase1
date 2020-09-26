#include <stdio.h>
#include <phase1.h>
#include <stdlib.h>
#include <phase1Int.h>
#include <assert.h>

static int cids[2];
void
Hello()
{
    for (int i = 1; i <= 10; i++) {
        printf("%d Hello ", i);
        int retval = P1ContextSwitch(cids[1]);
	assert(retval == P1_SUCCESS);
    }
    USLOSS_Halt(0);
}

void
World() 
{
    for (int i = 1; i <= 10; i++) {
        printf("World");
        for (int j = 0; j < i; j++) {
            printf("!");
        }
        printf("\n");
        int retval = P1ContextSwitch(cids[0]);
        assert(retval == P1_SUCCESS); 
    }
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
    
    // should not return here
    USLOSS_Console("This line should not be here\n.");
    assert(retval == P1_SUCCESS);
    assert(0);
}

// Do not modify anything below this line.

void
finish(int argc, char **argv)
{
    // Free World only since we halted in Hello
    int retval;
    retval = P1ContextFree(cids[1]);
    assert(retval == P1_SUCCESS);
    USLOSS_Console("Context World was freed.\n");
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

