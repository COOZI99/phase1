/*
Phase 1b
*/

#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

static void checkInKernelMode();
// static void enqueue(int pid);

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
    int             tag;
    // more fields here
    int             (*func)(void *);
    void            *arg;
    int             parentPid;          // The process ID of the parent
    Node            *childrenPids;      // The children process IDs of the process
    int             numChildren;        // The total number of children
    int             numQuit;            // The number of children who have quit
    int             sid;
    
} PCB;

int currentPID = 0;
static PCB processTable[P1_MAXPROC];   // the process table
static Node *readyQueue;               // pointer to last item in circular ready queue

void P1ProcInit(void)
{
    P1ContextInit();
    for (int i = 0; i < P1_MAXPROC; i++) {
        processTable[i].sid = -1;
        processTable[i].cid = 0;
        processTable[i].cpuTime = 0;
        processTable[i].priority = 0;
        processTable[i].state = P1_STATE_FREE;
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

static void launch(void *arg) {
    // int pid = (int) *arg;  FIGURE THIS OUT
    int pid = readyQueue->val;
    // Add a clock for how long this function takes to run
    int retVal = processTable[pid].func(processTable[pid].arg);
    // Stop clock and store value in cpuTime
    P1_Quit(retVal);
}

int P1_GetPid(void) 
{
    return readyQueue->next->val;
}

void reEnableInterrupts(int enabled) {
    if (enabled == TRUE) {
        P1EnableInterrupts();
    }
}

int P1_Fork(char *name, int (*func)(void*), void *arg, int stacksize, int priority, int tag, int *pid ) 
{
    // check for kernel mode
    checkInKernelMode();

    // disable interrupts
    int val = P1DisableInterrupts();

    // check all parameters
    // checking if tag is 0 or 1
    if( tag != 0 || tag != 1){
        reEnableInterrupts(val);
        return P1_INVALID_TAG;
    }

    // checking priority
    if(priority < 1 || priority > 6){
        reEnableInterrupts(val);
        return P1_INVALID_PRIORITY;
    }

    // checking stacksize
    if( stacksize < USLOSS_MIN_STACK){
        reEnableInterrupts(val);
        return P1_INVALID_STACK;
    }

    // checking if name is null
    if(name == NULL){
        reEnableInterrupts(val);
        return P1_NAME_IS_NULL;
    }

    if(sizeof(name) < P1_MAXNAME){
        reEnableInterrupts(val);
        return P1_NAME_TOO_LONG;
    }

    int i;
    for (i=0; i<P1_MAXPROC; i++) {
        // checking if there are duplicates 
        if(processTable[i].state != P1_STATE_FREE && strcmp(name,processTable[i].name) == 0){
            reEnableInterrupts(val);
            return P1_DUPLICATE_NAME;
        }
        // create a context using P1ContextCreate
        // allocate and initialize PCB
        if (processTable[i].state == P1_STATE_FREE) {
            *pid = i;
            // Setting the first fork
            if(i == 0){
                processTable[i].priority = 6;
                processTable[i].parentPid = 0;
                processTable[i].numChildren = 0;
            }
            else{
                // Setting the parent and children for other forks
                processTable[i].priority = priority;
                processTable[i].parentPid = currentPID;
                Node *temp = (Node*)malloc(sizeof(Node)); 
                temp->next = NULL;
                temp->val = i;
                if(processTable[currentPID].numChildren == 0){
                    processTable[currentPID].childrenPids = temp;
                }else{
                    Node *head = processTable[currentPID].childrenPids;
                    while(head != NULL){
                        if(head->next == NULL){
                            head->next = temp;
                            break;
                        }
                        head = head->next;
                    }
                }
                processTable[currentPID].numChildren ++;
            }
            *pid = i;
            int cid;
            // int retval = P1ContextCreate(func,arg,stacksize,&cid);
            int retVal = P1ContextCreate(launch, pid, stacksize, &cid);
            if (retVal != P1_SUCCESS) {
                reEnableInterrupts(val);
                return retVal;
            }
            processTable[i].cid = cid;
            processTable[i].cpuTime = 0;
            strcpy(processTable[i].name,name);
            processTable[i].state = P1_STATE_READY;
            processTable[i].numQuit = 0;
            processTable[i].tag = tag;
            
        //    enqueue(i);
            // if this is the first process or this process's priority is higher than the 
            //    currently running process call P1Dispatch(FALSE)
            int oldPriority = processTable[currentPID].priority;
            if(priority < oldPriority){
                currentPID = i;
                // P1Dispatch(FALSE);
            }
            int ret = P1ContextSwitch(processTable[currentPID].cid);
            assert(ret == P1_SUCCESS);
            // *pid = i;
            reEnableInterrupts(val);
            return P1_SUCCESS;
        }
    }
    reEnableInterrupts(val);
    return P1_TOO_MANY_PROCESSES;
}

void 
P1_Quit(int status) 
{
    // check for kernel mode
    checkInKernelMode();
    // disable interrupts
    int enabled = P1DisableInterrupts();
    // remove from ready queue, set status to P1_STATE_QUIT
    Node *currentNode = readyQueue->next;
    int currentPid = currentNode->val;
    readyQueue = readyQueue->next->next;
    free(currentNode); // Should this be done here or in set state?
    int retVal = P1SetState(currentPid, P1_STATE_QUIT, 0);
    if (retVal != P1_SUCCESS) {
        USLOSS_Halt(1);
    }
    // if first process verify it doesn't have children, otherwise give children to first process
    if (currentPid == 0 && processTable[currentPid].numChildren > processTable[currentPid].numQuit) {
        USLOSS_Console("First process quitting with children, halting.\n");
        USLOSS_Halt(1);
    }
    if (currentPid > 0) {
        Node *head = processTable[0].childrenPids->next;
        processTable[0].childrenPids->next = processTable[currentPid].childrenPids->next;
        processTable[currentPid].childrenPids->next = head;
        processTable[0].numChildren += processTable[currentPid].numChildren;
        processTable[0].numQuit += processTable[currentPid].numQuit;
    }
    // add ourself to list of our parent's children that have quit
    processTable[processTable[currentPid].parentPid].numQuit += 1;
    // if parent is in state P1_STATE_JOINING set its state to P1_STATE_READY
    if (processTable[processTable[currentPid].parentPid].state == P1_STATE_JOINING) {
        retVal = P1SetState(processTable[currentPid].parentPid, P1_STATE_READY, 0);
        if (retVal != P1_SUCCESS) {
            USLOSS_Halt(1);
        }
    }
    reEnableInterrupts(enabled);
    P1Dispatch(FALSE);
    // should never get here
    assert(0);
}


int 
P1GetChildStatus(int tag, int *pid, int *status) 
{
    int result = P1_SUCCESS;
    // if (pid < 0 || P1_MAXPROC <= pid || processTable[pid].state == P1_STATE_FREE) {
    //     return P1_INVALID_PID;
    // }
    
    return result;
}

int
P1SetState(int pid, P1_State state, int sid) 
{
    if (pid < 0 || P1_MAXPROC <= pid || processTable[pid].state == P1_STATE_FREE) {
        return P1_INVALID_PID;
    }
    if (state != P1_STATE_READY && state != P1_STATE_JOINING && state != P1_STATE_BLOCKED
        && state != P1_STATE_QUIT) {
            return P1_INVALID_STATE;
        }
    if (state == P1_STATE_READY) {
        // adding to the ready queue
        Node *head = readyQueue->next;
        Node *readyNode = malloc(sizeof(Node));
        readyNode->val = pid;
        readyNode->next = head;
        readyQueue->next = readyNode;
    }
    if (state == P1_STATE_BLOCKED) {
        processTable[pid].sid = sid;
    }
    if (state == P1_STATE_JOINING && processTable[pid].numQuit > 0) {
        return P1_CHILD_QUIT;
    }
    processTable[pid].state = state;
    return P1_SUCCESS;
}

void
P1Dispatch(int rotate)
{
    Node *ptr = readyQueue->next;
    Node *highestNode = readyQueue->next;
    // select the highest-priority runnable process
    while (ptr->next != readyQueue->next) {
        if (processTable[ptr->next->val].priority < processTable[highestNode->val].priority) {
            highestNode = ptr;
        }
    }

    if (highestNode != readyQueue->next) {
        // call P1ContextSwitch to switch to that process
        Node *newNode = ptr->next;
        ptr->next = ptr->next->next;
        newNode->next = readyQueue->next;
        readyQueue->next = newNode;
        int ret = P1ContextSwitch(processTable[newNode->val].cid);
        if (ret != P1_SUCCESS) {
            USLOSS_Halt(1);
        }
    } else if (rotate == TRUE) {
        // if rotate is true, find the next process with same priority and run it
        readyQueue = readyQueue->next;
        ptr = readyQueue;
        while (ptr->next != readyQueue) {
            if (processTable[ptr->next->val].priority == processTable[readyQueue->val].priority) {
                // call P1ContextSwitch to switch to that process
                Node *newNode = ptr->next;
                ptr->next = ptr->next->next;
                newNode->next = readyQueue->next;
                readyQueue->next = newNode;
                int ret = P1ContextSwitch(processTable[newNode->val].cid);
                if (ret != P1_SUCCESS) {
                    USLOSS_Halt(1); // Ask about this later!!!!!!
                }
                return;
            }
        }
        // No same priority found, returning current node to head of ready queue
        readyQueue = ptr;
    }
}

int
P1_GetProcInfo(int pid, P1_ProcInfo *info)
{
    int         result = P1_SUCCESS;
    if (pid < 0 || P1_MAXPROC <= pid || processTable[pid].state == P1_STATE_FREE) {
        return P1_INVALID_PID;
    }
    // strcmp(info->name,processTable[pid].name);
    info->sid = processTable[pid].sid;
    info->state = processTable[pid].state;
    info->priority = processTable[pid].priority;
    info->tag = processTable[pid].tag;
    info->cpu = processTable[pid].cpuTime;
    info->parent = processTable[pid].parentPid;
    info->numChildren = processTable[pid].numChildren;
    // int array[P1_MAXPROC];
    int i;
    Node *head = processTable[pid].childrenPids;
    for(i = 0; i < processTable[pid].numChildren; i++){
        // array[i] = head->val;
        info->children[i] = head->val;
        head = head->next;
    }
    // info->children[P1_MAXPROC] = array;
    return result;
}
