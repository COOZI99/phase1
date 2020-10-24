#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "usloss.h"
#include "phase1.h"
#include "phase1Int.h"

static void DeviceHandler(int type, void *arg);
static void SyscallHandler(int type, void *arg);
static void IllegalInstructionHandler(int type, void *arg);

static int sentinel(void *arg);

static void IllegalMessage(int n, void *arg){
    P1_Quit(1024);
}

static void checkInKernelMode() {
    if(!(USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE)){
        USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalMessage;
        USLOSS_IllegalInstruction();
    }
}

void 
startup(int argc, char **argv)
{
    checkInKernelMode();
    int pid;
    P1SemInit();

    // initialize device data structures
    P1ContextInit();
    P1ProcInit();
    P1SemInit();
    // put device interrupt handlers into interrupt vector
    USLOSS_IntVec[USLOSS_SYSCALL_INT] = SyscallHandler;

    /* create the sentinel process */
    int rc = P1_Fork("sentinel", sentinel, NULL, USLOSS_MIN_STACK, 6 , 0, &pid);
    assert(rc == P1_SUCCESS);
    // should not return
    assert(0);
    return;

} /* End of startup */

int 
P1_WaitDevice(int type, int unit, int *status) 
{
    int     result = P1_SUCCESS;
    // disable interrupts
    // check kernel mode
    // P device's semaphore
    // set *status to device's status
    // restore interrupts
    return result;
}

int 
P1_WakeupDevice(int type, int unit, int status, int abort) 
{
    int     result = P1_SUCCESS;
    // disable interrupts
    // check kernel mode
    // save device's status to be used by P1_WaitDevice
    // save abort to be used by P1_WaitDevice
    // V device's semaphore 
    // restore interrupts
    return result;
}

static void
DeviceHandler(int type, void *arg) 
{
    // if clock device
    //      P1_WakeupDevice every 5 ticks
    //      P1Dispatch(TRUE) every 4 ticks
    // else
    //      P1_WakeupDevice
}

static int
sentinel (void *notused)
{
    int     pid;
    int     rc;

    /* start the P2_Startup process */
    rc = P1_Fork("P2_Startup", P2_Startup, NULL, 4 * USLOSS_MIN_STACK, 2 , 0, &pid);
    assert(rc == P1_SUCCESS);

    // enable interrupts
    P1EnableInterrupts();
    // while sentinel has children
    int status
    while (P1GetChildStatus(0, pid, &status) == P1_SUCCESS) {
        USLOSS_WaitInt();
    }
    //      get children that have quit via P1GetChildStatus (either tag)
    //      wait for an interrupt via USLOSS_WaitInt
    USLOSS_Console("Sentinel quitting.\n");
    return 0;
} /* End of sentinel */

int 
P1_Join(int tag, int *pid, int *status) 
{
    checkInKernelMode();
    int result = P1_SUCCESS;
    // disable interrupts
    int enabled = P1DisableInterrupts();
    // kernel mode
    // do
    //     use P1GetChildStatus to get a child that has quit
    int rc = P1GetChildStatus(tag, pid, status);  
    if (rc == P1_NO_CHILDREN) {
        return P1_NO_CHILDREN;
    }
    if (rc == P1_INVALID_TAG) {
        return P1_INVALID_TAG;
    }
    if (rc == P1_NO_QUIT) {
        P1SetState(P1_GetPid(), P1_STATE_JOINING, 0);
        P1Dispatch(FALSE);
    }
    
    //     if no children have quit
    //        set state to P1_STATE_JOINING vi P1SetState
    //        P1Dispatch(FALSE)
    // until either a child quit or there are no more children
    return result;
}

static void
SyscallHandler(int type, void *arg) 
{
    USLOSS_Console("System call %d not implemented.\n", (int) arg);
    USLOSS_IllegalInstruction();
}

void finish(int argc, char **argv) {}
