/**
 *  Authors: Bianca Lara, Ann Chang
 *  Due Date: October 28th, 2020
 *  Phase 1d
 *  Submission Type: Group
 *  Comments: The phase 1d implementation of phase 1. Implements the 
 *  interrupt handlers for device, boots the entire Phase 1.
 *  The Device struct had 3 variables added to it:
 *      int     abort   if device needs to abort
 *      int     sid     semaphore for device
 *      int     status  
 */
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
static void reEnableInterrupts(int enabled);
static int sentinel(void *arg);

// Device struct
typedef struct Device
{
    int abort;
    int sid;
    int status;

} Device;

// Defining 2d array for devices
static Device array[4][USLOSS_MAX_UNITS];
static int calls = 0;

///////////////////////////////////////////////////////////////////////////////
// Helper Functions
///////////////////////////////////////////////////////////////////////////////
static void IllegalMessage(int n, void *arg){
    P1_Quit(1024);
}

static void checkInKernelMode() {
    if(!(USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE)){
        USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalMessage;
        USLOSS_IllegalInstruction();
    }
}

static void reEnableInterrupts(int enabled) {
    if (enabled == TRUE) {
        P1EnableInterrupts();
    }
}

/*
 * checking if type and unit is valid
 */
static int checkTypeUnit(int type, int unit){
    if((type == USLOSS_CLOCK_INT || type == USLOSS_ALARM_INT) && unit != 0){
        return P1_INVALID_UNIT;
    }
    if(type == USLOSS_DISK_INT && (unit < 0 || unit > 1)){
        return P1_INVALID_UNIT;
    }
    if (unit < 0 || unit >= USLOSS_MAX_UNITS){
        return P1_INVALID_UNIT;
    }
    if(type < USLOSS_CLOCK_INT || type > USLOSS_TERM_INT){
        return P1_INVALID_TYPE;
    }
    return P1_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// End of Helper Functions
///////////////////////////////////////////////////////////////////////////////

void 
startup(int argc, char **argv)
{
    checkInKernelMode();
    P1SemInit();

    int pid;
    int i, j;
    int count = 0;
    // initialize device data structures 
    for( i = 0; i < 4; i ++){
        for( j = 0; j < USLOSS_MAX_UNITS; j++){
            int semID;
            static char name[P1_MAXNAME + 1];
            snprintf(name,sizeof(name), "%s%d","Sem",count);
            int val = P1_SemCreate(name,0,&semID);
            assert(val == P1_SUCCESS);
            array[i][j].sid = semID;
            array[i][j].status = -1;
            array[i][j].abort = 0;
            count ++;
        }
    }
    // put device interrupt handlers into interrupt vector
    USLOSS_IntVec[USLOSS_SYSCALL_INT] = SyscallHandler;
    USLOSS_IntVec[USLOSS_CLOCK_INT] = DeviceHandler;
    USLOSS_IntVec[USLOSS_ALARM_INT] = DeviceHandler;
    USLOSS_IntVec[USLOSS_DISK_INT] = DeviceHandler;
    USLOSS_IntVec[USLOSS_TERM_INT] = DeviceHandler;

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
    checkInKernelMode();
    int result = P1_SUCCESS;
    // disable interrupts
    int enabled = P1DisableInterrupts();

    // verifying type and unit
    result = checkTypeUnit(type,unit);
    if(result == P1_SUCCESS){
        // P device's semaphore
        int val = P1_P(array[type][unit].sid);
        assert(val == P1_SUCCESS);
        // set *status to device's status
        *status = array[type][unit].status;
        // abort
        if(array[type][unit].abort == 1){
            result = P1_WAIT_ABORTED;
        }
    }

    // restore interrupts
    reEnableInterrupts(enabled);
    return result;
}

int 
P1_WakeupDevice(int type, int unit, int status, int abort) 
{
    checkInKernelMode();
    int result = P1_SUCCESS;
    // disable interrupts
    int enabled = P1DisableInterrupts();

    result = checkTypeUnit(type,unit);
    if(result == P1_SUCCESS){
        // save device's status to be used by P1_WaitDevice
        array[type][unit].status = status;
        // save abort to be used by P1_WaitDevice
        array[type][unit].abort = abort;
        // V device's semaphore 
        int val = P1_V(array[type][unit].sid);
        assert(val == P1_SUCCESS);
    }

    // restore interrupts
    reEnableInterrupts(enabled);
    return result;
}

static void
DeviceHandler(int type, void *arg) 
{
    int unit = (int)arg;

    // if clock device
    //      P1_WakeupDevice every 5 ticks
    //      P1Dispatch(TRUE) every 4 ticks
    if(type == USLOSS_CLOCK_INT){
        calls ++;
        if(calls % 5 == 0){
            int status;
            assert(USLOSS_DeviceInput(USLOSS_CLOCK_DEV, unit, &status) == P1_SUCCESS);
            assert(P1_WakeupDevice(USLOSS_CLOCK_DEV,unit,status,0) == P1_SUCCESS); 
        }else if(calls % 4 == 0){
            P1Dispatch(TRUE);
        }
    }else{
        int status;
        assert(USLOSS_DeviceInput(type, unit, &status) == P1_SUCCESS);
        assert(P1_WakeupDevice(type, unit, status, 0) == P1_SUCCESS);
    }

}

static int
sentinel (void *notused)
{
int pid;
    int currPid = P1_GetPid();
    int rc;
    int status;
    /* start the P2_Startup process */
    rc = P1_Fork("P2_Startup", P2_Startup, NULL, 4 * USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);

    P1EnableInterrupts();

    // while sentinel has children
    //      get children that have quit via P1GetChildStatus (either tag)
    //      wait for an interrupt via USLOSS_WaitInt
    P1_ProcInfo process;
    rc = P1_GetProcInfo(currPid, &process);
    while (process.numChildren > 0) {   
        int tag1, tag2;
        // Removing all the child tags
        do{
            tag1 = (P1GetChildStatus(0, &pid, &status) == P1_SUCCESS);
            tag2 = (P1GetChildStatus(1, &pid, &status) == P1_SUCCESS);
        }while(tag1 == 1 || tag2 == 1);
        
        USLOSS_WaitInt();
        rc = P1_GetProcInfo(currPid, &process);
    }

    USLOSS_Console("Sentinel quitting.\n");
    return 0;
   
} /* End of sentinel */

int 
P1_Join(int tag, int *pid, int *status) 
{
     // kernel mode
    checkInKernelMode();
    int result = P1_SUCCESS;
    int childPid;
    // disable interrupts
    int rc = P1DisableInterrupts();
    rc = P1GetChildStatus(tag, pid, status);  

    // validate tag
    if (rc == P1_INVALID_TAG) {
        return P1_INVALID_TAG;
    }
    // check if theres children
    if(rc == P1_NO_CHILDREN){
        return P1_NO_CHILDREN;
    }

    if(rc == P1_NO_QUIT){
        // Representing process
        P1_ProcInfo process;
        // do
        //     use P1GetChildStatus to get a child that has quit  
        //     if no children have quit
        // until either a child quit or there are no more children
        int val = P1_GetProcInfo(P1_GetPid(),&process);
        while(rc != P1_SUCCESS && process.numChildren > 0){
            val = P1SetState(P1_GetPid(),P1_STATE_JOINING,process.sid);
            val = P1_GetProcInfo(P1_GetPid(),&process);
            P1Dispatch(FALSE);
            rc = P1GetChildStatus(tag,&childPid,status);
        }
    
    }
    *pid = childPid;
    return result;
}

static void
SyscallHandler(int type, void *arg) 
{
    USLOSS_Console("System call %d not implemented.\n", (int) arg);
    USLOSS_IllegalInstruction();
}

void finish(int argc, char **argv) {}
