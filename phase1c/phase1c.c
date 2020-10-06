
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include "usloss.h"
#include "phase1Int.h"

static void checkInKernelMode();

typedef struct Sem
{
    char        name[P1_MAXNAME+1];
    u_int       value;
    // more fields here
    int         blocked;
} Sem;

static Sem sems[P1_MAXSEM];

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

void 
P1SemInit(void) 
{
    P1ProcInit();
    for (int i = 0; i < P1_MAXSEM; i++) {
        sems[i].name[0] = '\0';
        // initialize rest of sem here
    }
}

int P1_SemCreate(char *name, unsigned int value, int *sid)
{
    int result = P1_SUCCESS;
    // check for kernel mode
    // disable interrupts
    // check parameters
    // find a free Sem and initialize it
    // re-enable interrupts if they were previously enabled
    return result;
}

int P1_SemFree(int sid) 
{
    int     result = P1_SUCCESS;
    if( sid > P1_MAXSEM || sid < 0){
        result = P1_INVALID_SID;
    }else if(sems[sid].blocked){
        result = P1_BLOCKED_PROCESSES;
    }else{

    }

    return result;
}

int P1_P(int sid) 
{
    int result = P1_SUCCESS;
    // check for kernel mode
    // disable interrupts
    // while value == 0
    //      set state to P1_STATE_BLOCKED
    // value--
    // re-enable interrupts if they were previously enabled
    return result;
}

int P1_V(int sid) 
{
    int result = P1_SUCCESS;
    checkInKernelMode();                    // check for kernel mode
    int enabled = P1DisableInterrupts();    // disable interrupts
    // value++
    // if a process is waiting for this semaphore
    //      set the process's state to P1_STATE_READY
    reEnableInterrupts(enabled);            // re-enable interrupts if they were previously enabled
    return result;
}

int P1_SemName(int sid, char *name) {
    int result = P1_SUCCESS;

    if( sid > P1_MAXSEM || sid < 0){
        result = P1_INVALID_SID;
    }else if(sems[sid].name == NULL){
        result = P1_NAME_IS_NULL;
    }else{
        strcpy(name,sems[sid].name);
    }
    return result;
}

