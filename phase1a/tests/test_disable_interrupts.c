#include <stdio.h>
#include <phase1.h>
#include <stdlib.h>
#include <phase1Int.h>
#include <assert.h>
static int cids[2];

// startup(): Test enabling and disabling interrupts by checking PSR interrupt bit

void
Hello()
{
	
	USLOSS_Console("Printing Hello in context Hello.\n");
	
	USLOSS_Halt(0);
}

void
startup(int argc, char **argv)
{
    int retval, interrupt;
    P1ContextInit();

    retval = P1ContextCreate(Hello, NULL, USLOSS_MIN_STACK, &cids[0]);
    assert(retval == P1_SUCCESS);
    
    // running in with interrupts disabled
    interrupt = (USLOSS_PsrGet() & USLOSS_PSR_CURRENT_INT) >> 1;
    assert(interrupt == FALSE);

    // enable interrupts
    P1EnableInterrupts();
    interrupt = (USLOSS_PsrGet() & USLOSS_PSR_CURRENT_INT) >> 1;     
    assert(interrupt == TRUE);
     
    // disable interrupts
    retval = P1DisableInterrupts(); 
    assert(retval == TRUE); //previous interrupt was TRUE
    interrupt = (USLOSS_PsrGet() & USLOSS_PSR_CURRENT_INT) >> 1;
    assert(interrupt == FALSE);

    // enable interrupts again
    P1EnableInterrupts();
    interrupt = (USLOSS_PsrGet() & USLOSS_PSR_CURRENT_INT) >> 1;
    assert(interrupt == TRUE);  
   
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

