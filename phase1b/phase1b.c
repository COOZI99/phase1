/*
Phase 1b
*/

#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

// Implementing a circularly linked list for best queue structure
typedef struct Node {
    int          val;
    struct Node* next;
} Node;

typedef struct PCB {
    int             cid;                // context's ID
    int             cpuTime;            // process's running time
    char            name[P1_MAXNAME+1]; // process's name
    int             priority;           // process's priority
    P1_State        state;              // state of the PCB
    // more fields here
    int             parentPid;          // The process ID of the parent
    Node            *childrenPids;      // The children process IDs of the process
    int             numChildren;        // The total number of children
    Node           *quitChildren;      // The children who have quit
    int             numQuit;            // The number of children who have quit
    
} PCB;

static PCB processTable[P1_MAXPROC];   // the process table
static Node *readyQueue;               // pointer to last item in circular ready queue

void P1ProcInit(void)
{
    P1ContextInit();
    for (int i = 0; i < P1_MAXPROC; i++) {
        processTable[i].state = P1_STATE_FREE;
        // initialize the rest of the PCB
    }
    // initialize everything else

}

// Halting the program for an illegal message
static void IllegalMessage(int n, void *arg){
    P1_Quit(1024);
}

static void checkInKernelMode() {
    // Checking if we are in kernal mode
    if(!(USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE)){
        USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalMessage;
        USLOSS_IllegalInstruction();
    }
}

int P1_GetPid(void) 
{
    return *readyQueue;
}

int P1_Fork(char *name, int (*func)(void*), void *arg, int stacksize, int priority, int tag, int *pid ) 
{
    int result = P1_SUCCESS;

    // check for kernel mode
    // disable interrupts
    // check all parameters
    // create a context using P1ContextCreate
    // allocate and initialize PCB
    // if this is the first process or this process's priority is higher than the 
    //    currently running process call P1Dispatch(FALSE)
    // re-enable interrupts if they were previously enabled
    return result;
}

void 
P1_Quit(int status) 
{
    // check for kernel mode
    checkInKernelMode();
    // disable interrupts
    int ret = P1DisableInterrupts();
    // remove from ready queue, set status to P1_STATE_QUIT
    int currentPid = readyQueue->val;
    readyQueue++;
    ret = P1SetState(currentPid, P1_STATE_QUIT, 0);

    // if first process verify it doesn't have children, otherwise give children to first process
    if (currentPid == 0 && processTable[currentPid].numChildren > processTable[currentPid].numQuit) {
        USLOSS_Console("First process quitting with children, halting.\n");
        USLOSS_Halt(1);
    }
    if (currentPid > 0) {
        Node* head = processTable[0].childrenPids->next;
        processTable[0].childrenPids->next = processTable[currentPid].childrenPids->next;
        processTable[currentPid].childrenPids->next = head;
        processTable[0].numChildren += processTable[currentPid].numChildren;
        processTable[0].numQuit += processTable[currentPid].numQuit;
    }
    // add ourself to list of our parent's children that have quit
    Node* head = processTable[processTable[currentPid].parentPid].quitChildren->next;
    Node* quitNode;
    quiteNode->val = currentPid;
    quitNode->next = head;
    processTable[processTable[currentPid].parentPid].quitChildren->next = quitNode;
    processTable[processTable[currentPid].parentPid].numQuit += 1;
    // if parent is in state P1_STATE_JOINING set its state to P1_STATE_READY
    if (processTable[processTable[currentPid].parentPid].state == P1_STATE_JOINING) {
        P1SetState(processTable[currentPid].parentPid, P1_STATE_READY, 0);
    }
    P1Dispatch(FALSE);
    // should never get here
    assert(0);
}


int 
P1GetChildStatus(int tag, int *cpid, int *status) 
{
    int result = P1_SUCCESS;
    // do stuff here
    return result;
}

int
P1SetState(int pid, P1_State state, int sid) 
{
    if (pid < 0 || P1_MAXPROC <= pid || contexts[processTable[pid].cid].wasCreated == 0) {
        return P1_INVALID_PID;
    }
    if (state != P1_STATE_READY && state != P1_STATE_JOINING && state != P1_STATE_BLOCKED
        && state != P1_STATE_QUIT) {
            return P1_INVALID_STATE;
        }
    if (state == P1_STATE_READY) {
        // adding to the ready queue
        Node *head = readyQueue->next;
        Node *readyNode;
        readyNode->val = pid;
        readyNode->next = head;
        readyQueue->next = readyNode;
    }
    processTable[pid].state = state;
    return P1_SUCCESS;
}

void
P1Dispatch(int rotate)
{
    int i;
    int cid=-1;
    int maxPriority = 7;
    for (i=0; i<P1_MAXPROC; i++) {
        if (processTable[i].priority < maxPriority) {

        }
    }

    // select the highest-priority runnable process
    // call P1ContextSwitch to switch to that process
}

int
P1_GetProcInfo(int pid, P1_ProcInfo *info)
{
    int         result = P1_SUCCESS;
    // fill in info here
    return result;
}







