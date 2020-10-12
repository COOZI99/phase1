/**
 *  Authors: Bianca Lara, Ann Chang
 *  Due Date: October 14th, 2020
 *  Phase 1c
 *  Submission Type: Group
 *  Comments: The phase 1c implementation of phase 1. Implements the 
 *  outlined functions for creating, freeing, performing P/V, and getting 
 *  info of the semaphores. The Sem struct had 3 variables added to it:
 *      int      notFreed       determining if the semaphore is freed or not
 *      int      blocked        keeping track how many are blocked
 *      Node     *queue         linked list of the pid that is waiting for
 *                              specific sid
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include "usloss.h"
#include "phase1Int.h"

static void checkInKernelMode();
static void enqueue(int sid, int pid);
static int dequeue(int sid);

typedef struct Node {
    int          val;
    struct Node* next;
} Node;

typedef struct Sem
{
    char        name[P1_MAXNAME+1]; 
    u_int       value;
    // more fields here
    int         notFreed;
    int         blocked;
    Node        *queue;
} Sem;


static Sem sems[P1_MAXSEM];

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
 * Helper function to add pid to the particular sid in order to
 * keep track of processes that is waiting for this semaphores
 */
static void enqueue(int sid, int pid){
    Node *node = (Node*)malloc(sizeof(Node)); 
    node->next = NULL;
    node->val = pid;
    if(sems[sid].queue == NULL){
        sems[sid].queue = node;
    }else{
        Node *head = sems[sid].queue;
        while(head->next != NULL){
            head = head->next;
        }
        head->next = node;
    }
}

/*
 * Helper function to return the process pid that is 
 * waiting for the sid and also removing it from the queue
 */
static int dequeue(int sid){
    
    if(sems[sid].queue == NULL){
        return -1;
    }else{
        int pid = sems[sid].queue->val;
        Node *temp = sems[sid].queue;
        sems[sid].queue = sems[sid].queue->next;
        free(temp);
        return pid;
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
        sems[i].blocked = 0;
    }
}

int P1_SemCreate(char *name, unsigned int value, int *sid)
{
    checkInKernelMode();                    // check for kernel mode
    int enabled = P1DisableInterrupts();    // disable interrupts
    // check parameters
    if (name == NULL) {
        return P1_NAME_IS_NULL;
    }
    if (strlen(name) > P1_MAXNAME) {
        return P1_NAME_TOO_LONG;
    }
    int i;
    for (i=0; i<P1_MAXSEM; i++) {
        if (sems[i].notFreed == TRUE && strcmp(sems[i].name, name) == 0) {
            reEnableInterrupts(enabled);
            return P1_DUPLICATE_NAME;
        }
        // find a free Sem and initialize it
        if (sems[i].notFreed == FALSE) {
            *sid = i;
            sems[i].notFreed = TRUE;
            strcpy(sems[i].name, name);
            sems[i].value = value;
            sems[i].queue = NULL;
            reEnableInterrupts(enabled);
            return P1_SUCCESS;
        }
    }  
    // re-enable interrupts if they were previously enabled
    reEnableInterrupts(enabled);
    return P1_TOO_MANY_SEMS;
}

int P1_SemFree(int sid) 
{
    checkInKernelMode();             // check for kernel mode
    int     result = P1_SUCCESS;
    if(sid < 0 || sid >= P1_MAXSEM || sems[sid].notFreed == FALSE){
        result = P1_INVALID_SID;
    }else if(sems[sid].blocked != 0){
        result = P1_BLOCKED_PROCESSES;
    }else{
        sems[sid].name[0] = '\0';
        sems[sid].notFreed = FALSE;
        while(sems[sid].queue != NULL){
            Node *temp = sems[sid].queue;                   
            sems[sid].queue = sems[sid].queue->next;
            free(temp);
        }
    }

    return result;
}

int P1_P(int sid) 
{
    int result = P1_SUCCESS;
    checkInKernelMode();             // check for kernel mode
    if (sid < 0 || sid >= P1_MAXSEM || sems[sid].notFreed == FALSE) {
        return P1_INVALID_SID;
    }
    int enabled = P1DisableInterrupts(); // disable interrupts

    if (sems[sid].value <= 0) {
        int pid = P1_GetPid();
        int val = P1SetState(pid, P1_STATE_BLOCKED, sid);
        assert(val == P1_SUCCESS);
        sems[sid].blocked ++;
        enqueue(sid,pid);
        P1Dispatch(FALSE);
    } 
    sems[sid].value--;
    reEnableInterrupts(enabled);
    return result;
}

int P1_V(int sid) 
{
    int result = P1_SUCCESS;
    checkInKernelMode();                    // check for kernel mode
    if (sid < 0 || sid >= P1_MAXSEM || sems[sid].notFreed == FALSE) {
        return P1_INVALID_SID;
    }
    int enabled = P1DisableInterrupts();    // disable interrupts
    int pid = dequeue(sid);
    // if a process is waiting for this semaphore
    //      set the process's state to P1_STATE_READY
    if(pid != -1){
        int val = P1SetState(pid, P1_STATE_READY, sid);
        sems[sid].blocked --;
        assert(val == P1_SUCCESS);
    }else{
        sems[sid].value ++;                     // value++
    }
    // re-enable interrupts if they were previously enabled
    reEnableInterrupts(enabled); 
    P1Dispatch(FALSE);
    return result;
}

int P1_SemName(int sid, char *name) {
    checkInKernelMode();                    // check for kernel mode
    int result = P1_SUCCESS;
    if(sid < 0 || sid >= P1_MAXSEM || sems[sid].notFreed == FALSE){
        result = P1_INVALID_SID;
    }else if(sems[sid].name == NULL){
        result = P1_NAME_IS_NULL;
    }else{
        strcpy(name,sems[sid].name);
    }
    return result;
}

