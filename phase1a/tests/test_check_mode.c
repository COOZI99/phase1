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
    
    P1EnableInterrupts();
    int val = USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_MODE);
    assert(val == USLOSS_DEV_OK);
    USLOSS_Console("Current mode bit: %d\n", USLOSS_PsrGet() & 0x1);

    // P1DisableInterrupts() will first do a kernel check and reports that the current mode is user mode
    // since we set the mode bit to 0. It then halts.
    int previous = P1DisableInterrupts();
    assert(previous == TRUE);
    
    P1ContextInit();
    
    // should not get here
    rc = P1ContextCreate(Output, "Hello World!\n", USLOSS_MIN_STACK, &cid);
    assert(rc == P1_SUCCESS);
    rc = P1ContextSwitch(cid);
      
   
    // should not return
    assert(rc == P1_SUCCESS);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
