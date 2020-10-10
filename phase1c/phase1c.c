
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include "usloss.h"
#include "phase1Int.h"

typedef struct Sem
{
    char        name[P1_MAXNAME+1];
    u_int       value;
    // more fields here
    int         notFreed;
} Sem;

static Sem sems[P1_MAXSEM];

///////////////////////////////////////////////////////////////////////////////
// Helper Functions
///////////////////////////////////////////////////////////////////////////////
static void IllegalMessage(int n, void *arg){
    USLOSS_Halt(0);
}

static void checkInKernelMode() {
    if(!(USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE)){
        USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalMessage;
        USLOSS_IllegalInstruction();
    }
}

static void reenableInterrupts(int enabled) {
    if (enabled == TRUE) {
        P1EnableInterrupts();
    }
}

///////////////////////////////////////////////////////////////////////////////
// End of Helper Functions
///////////////////////////////////////////////////////////////////////////////


void 
P1SemInit(void) 
{
    P1ProcInit();
    for (int i = 0; i < P1_MAXSEM; i++) {
        sems[i].name[0] = '\0';
        sems[i].notFreed = FALSE;
        // initialize rest of sem here
    }
}

int P1_SemCreate(char *name, unsigned int value, int *sid)
{
    checkInKernelMode()                 // check for kernel mode
    int enabled = P1DisableInterrupts();    // disable interrupts
    // check parameters
    if (name == NULL) {
        return P1_NAME_IS_NULL;
    }
    if (sizeof(name) > P1_MAXNAME) {
        return P1_NAME_TOO_LONG;
    }
    int i;
    for (i=0; i<P1_MAXSEM; i++) {
        if (sems[i].notFreed == TRUE && strcmp(sems[i].name, name) == 0) {
            reenableInterrupts(enabled);
            return P1_DUPLICATE_NAME;
        }
        // find a free Sem and initialize it
        if (sems[i].notFreed == FALSE) {
            *sid = i;
            sems[i].notFreed = TRUE;
            sems[i].name = name;
            sems[i].value = value;
            return P1_SUCCESS;
        }
    }  
    // re-enable interrupts if they were previously enabled
    reenableInterrupts(enabled);
    return P1_TOO_MANY_SEMS;
}

int P1_SemFree(int sid) 
{
    int     result = P1_SUCCESS;
    // more code here
    return result;
}

int P1_P(int sid) 
{
    int result = P1_SUCCESS;
    checkInKernelMode();             // check for kernel mode
    int enabled = P1DisableInterrupts(); // disable interrupts
    while (sem[sid].value == 0) {
        P1SetState(P1_GetPid(), P1_STATE_BLOCKED, sid);
    }
    sem[sid].value--;
    reenableInterrupts(enabled);
    return result;
}

int P1_V(int sid) 
{
    int result = P1_SUCCESS;
    // check for kernel mode
    // disable interrupts
    // value++
    // if a process is waiting for this semaphore
    //      set the process's state to P1_STATE_READY
    // re-enable interrupts if they were previously enabled
    return result;
}

int P1_SemName(int sid, char *name) {
    int result = P1_SUCCESS;
    // more code here
    return result;
}

