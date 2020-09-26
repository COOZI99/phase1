#include <stdio.h>
#include <phase1.h>
#include <stdlib.h>
#include <phase1Int.h>
#include <assert.h>
static int cids[2];

// startup(): Test disabling interrupts twice

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
    
    // disable interrupt
    retval = P1DisableInterrupts();
    assert(retval == FALSE);
    interrupt = (USLOSS_PsrGet() & USLOSS_PSR_CURRENT_INT) >> 1;
    assert(interrupt == FALSE);
  
    // disable again  
    retval = P1DisableInterrupts();
    assert(retval == FALSE);
    
    // enable interrupt 
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

