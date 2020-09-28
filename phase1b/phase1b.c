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

static void checkInKernelMode();
static void add_child(int pid);
static void enqueue(int pid);
// static void printQueue();
// void printing(Node *head);
void free_procress(int pid);
void remove_child(int parent, int child);

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
    Node            *quitChildren;       // The children that quitted
    int             numChildren;        // The total number of children
    int             numQuit;            // The number of children who have quit
    int             sid;
    int             status;
    int             startTime;
    
} PCB;

static PCB processTable[P1_MAXPROC];   // the process table
static Node *readyQueue;               // pointer to last item in circular ready queue
static int status;

void P1ProcInit(void)
{
    P1ContextInit();
    for (int i = 0; i < P1_MAXPROC; i++) {
        processTable[i].sid = 0;
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
    int pid = readyQueue->next->val;
    // USLOSS_Console("Launching process %d\n", pid);
    // Add a clock for how long this function takes to run
    int retVal = processTable[pid].func(processTable[pid].arg);
    // Stop clock and store value in cpuTime
    P1_Quit(retVal);
    // USLOSS_Halt(retVal);
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
    if( tag != 0 && tag != 1){
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

    if(sizeof(name) > P1_MAXNAME){
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
            USLOSS_Console("PID = %d\n",i);
            *pid = i;
            int cid;
            int retVal = P1ContextCreate(launch, pid, stacksize, &cid);
            if (retVal != P1_SUCCESS) {
                reEnableInterrupts(val);
                return retVal;
            }
            enqueue(i);
            processTable[i].childrenPids = NULL;
            processTable[i].cid = cid;
            processTable[i].cpuTime = 0;
            strcpy(processTable[i].name,name);
            processTable[i].state = P1_STATE_READY;
            processTable[i].numQuit = 0;
            processTable[i].tag = tag;
            processTable[i].priority = priority;
            processTable[i].parentPid = readyQueue->next->val;
            processTable[i].numChildren = 0;
            processTable[i].func = func;
            processTable[i].arg = arg;
            processTable[i].quitChildren = NULL;
            // Setting the first fork
            if(i == 0 && priority != 6){
                return P1_INVALID_PRIORITY;
            }else if (i != 0){
                // USLOSS_Console("Adding Child %d to %d\n", i, processTable[i].parentPid);
                add_child(i);
            }
            // if this is the first process or this process's priority is higher than the 
            //    currently running process call P1Dispatch(FALSE)
            // int oldPriority = processTable[currentPID].priority;
            // printQueue();
            P1Dispatch(FALSE);
            reEnableInterrupts(val);
            return P1_SUCCESS;
        }
    }
    reEnableInterrupts(val);
    return P1_TOO_MANY_PROCESSES;
}

static void enqueue(int pid){
    if(pid == 0){
        readyQueue = (Node*)malloc(sizeof(Node)); 
        readyQueue->val = pid;
        readyQueue->next = readyQueue;
    }else{
        Node *node = (Node*)malloc(sizeof(Node));
        node->val = pid;
        node->next = readyQueue->next;
        readyQueue->next = node;
        readyQueue = readyQueue->next;
    }
}


static void add_child(int pid){

    int currentpid = readyQueue->next->val;
    Node *new_child = (Node*)malloc(sizeof(Node)); 
    new_child->val = pid;
    new_child->next = NULL;
    Node *head = processTable[readyQueue->next->val].childrenPids;
    if(head == NULL){
        processTable[readyQueue->next->val].childrenPids = new_child;
    }else{
        while(head->next != NULL){
            head = head->next;
        }
        head->next = new_child;
    }
    processTable[currentpid].numChildren ++;

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
    // USLOSS_Console("Calling Quits for %d\n", currentPid);
    readyQueue->next = readyQueue->next->next;
    free(currentNode); // Should this be done here or in set state?
    processTable[currentPid].status = status;
    int parent = processTable[currentPid].parentPid;
    Node *new_node = (Node*)malloc(sizeof(Node));
    new_node->val = currentPid;
    new_node->next = NULL;
    if(processTable[parent].quitChildren == NULL){
        processTable[parent].quitChildren = new_node;
    }else{
        Node *temp = processTable[parent].quitChildren;
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next = new_node;
    }

     int retVal = P1SetState(currentPid, P1_STATE_QUIT, 0);
    if (retVal != P1_SUCCESS) {
        USLOSS_Halt(1);
    }
    // USLOSS_Console("Done P1SetState %d\n", currentPid);
    // if first process verify it doesn't have children, otherwise give children to first process
    if (currentPid == 0 && processTable[currentPid].numChildren > processTable[currentPid].numQuit) {
        USLOSS_Console("No runnable processes, halting.\n");
        USLOSS_Halt(1);
    } else if (currentPid == 0) {
        USLOSS_Console("No runnable processes, halting.\n");
        USLOSS_Halt(0);
    }
    if (currentPid > 0 && processTable[currentPid].childrenPids != NULL) {
        Node *head = processTable[0].childrenPids->next;
        processTable[0].childrenPids->next = processTable[currentPid].childrenPids->next;
        processTable[currentPid].childrenPids->next = head;
        processTable[0].numChildren += processTable[currentPid].numChildren;
        processTable[0].numQuit += processTable[currentPid].numQuit;
    }
    // USLOSS_Console("Adding ourselv to our parents quitters\n");
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
    int enabled = P1DisableInterrupts();
    // checking if tag is 0 or 1
    if( tag != 0 && tag != 1){
        return P1_INVALID_TAG;
    }

    if(processTable[readyQueue->next->val].childrenPids == NULL){
        return P1_NO_CHILDREN;
    }

    if(processTable[readyQueue->next->val].status == P1_STATE_BLOCKED){
        P1Dispatch(FALSE);
    }

    Node *childll = processTable[readyQueue->next->val].quitChildren;
    if(childll == NULL){
        return P1_NO_QUIT;
    }else{
        while(childll != NULL){
            if(processTable[childll->val].tag == tag){
                *pid = childll->val;
                *status = processTable[childll->val].status;
                int num = childll->val;
                remove_child(readyQueue->next->val,childll->val);
                free_procress(num);
                reEnableInterrupts(enabled);
                return P1_SUCCESS;
            }
            childll = childll->next;
        }
    }
    return P1_NO_QUIT;
}

void free_procress(int pid){
    Node *headC = processTable[pid].childrenPids;
    int val = P1ContextFree(processTable[pid].cid);
    assert(val == P1_SUCCESS);
    while(headC != NULL){
        Node *temp = headC;
        headC = headC->next;
        free(temp);
    }

    Node *headQ = processTable[pid].quitChildren;
    while(headQ != NULL){
        Node *temp = headQ;
        headQ = headQ->next;
        free(temp);
    }
    processTable[pid].state = P1_STATE_FREE;
}

void remove_child(int parent, int child){
    Node *headChildren = processTable[parent].childrenPids;

    if(headChildren->val == child){
        Node *temp = processTable[parent].childrenPids;
        processTable[parent].childrenPids = processTable[parent].childrenPids->next;
        free(temp);
    }else{
        while(headChildren != NULL && headChildren->next != NULL){
            if(headChildren->next->val == child){
                headChildren->next = headChildren->next->next;
                break;
            }
            headChildren = headChildren->next;
        }
    }

    Node *headQuit = processTable[parent].quitChildren;

    if(headQuit->val == child){
        Node *temp = processTable[parent].quitChildren;
        processTable[parent].quitChildren = processTable[parent].quitChildren->next;
        free(temp);
    }else{
        while(headQuit != NULL && headQuit->next != NULL){
            if(headChildren->next->val == child){
                headQuit->next = headQuit->next->next;
                break;
            }
            headQuit = headQuit->next;
        }
    }

    processTable[parent].numChildren --;
    processTable[parent].numQuit --;

}

// void printing(Node *head){
//     USLOSS_Console("_______________\n");
//     while(head != NULL){
//         USLOSS_Console("CHILD IS %d\n",head->val);
//         head = head->next;
//     }
//     USLOSS_Console("_______________\n");
// }


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
    if (processTable[readyQueue->next->val].startTime == 0) {
        status = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, 0, &(processTable[readyQueue->next->val].startTime));
    } else {
        int endTime;
        status = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, 0, &(endTime));
        processTable[readyQueue->next->val].cpuTime += (endTime - processTable[readyQueue->next->val].startTime);
        processTable[readyQueue->next->val].startTime = 0;
    }
    
    // processTable[readyQueue->next->val].cpuTime += (time - currentCpuTime);
    processTable[readyQueue->next->val].state = P1_STATE_READY;

    // currentCpuTime = time;
    if (readyQueue->next == readyQueue) {
        // Only one process ready
        // USLOSS_Console("Running Process: %d\n", readyQueue->val);
        processTable[readyQueue->val].state = P1_STATE_RUNNING;
        int ret = P1ContextSwitch(processTable[readyQueue->val].cid);
        if (ret != P1_SUCCESS) {
            USLOSS_Halt(1);
        }
    }

    Node *ptr = readyQueue->next;
    Node *highestNode = readyQueue;
    // select the highest-priority runnable process
    while (ptr->next != readyQueue->next) {
        if (processTable[ptr->next->val].priority < processTable[highestNode->next->val].priority) {
            highestNode = ptr;
        }
        ptr = ptr->next;
    }

    if (highestNode->next->val != readyQueue->next->val) {
        Node *newNode;
        if(highestNode->next == readyQueue){
            newNode = readyQueue;
            readyQueue = highestNode;
        }else{
            // call P1ContextSwitch to switch to that process
            newNode = highestNode->next;
            highestNode->next = highestNode->next->next;
            newNode->next = readyQueue->next;
            readyQueue->next = newNode;
        }
        processTable[newNode->val].state = P1_STATE_RUNNING;
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
                processTable[newNode->val].state = P1_STATE_RUNNING;
                int ret = P1ContextSwitch(processTable[newNode->val].cid);
                if (ret != P1_SUCCESS) {
                    USLOSS_Halt(1); // Ask about this later!!!!!!
                }
                return;
            }
            ptr = ptr->next;
        }
        // No same priority found, returning current node to head of ready queue
        readyQueue = ptr;
    } else {
        processTable[readyQueue->next->val].state = P1_STATE_RUNNING;
        int ret = P1ContextSwitch(processTable[readyQueue->next->val].cid);
        if (ret != P1_SUCCESS) {
            USLOSS_Console("Error Switching Contexts\n");
            USLOSS_Halt(1); // Ask about this later!!!!!!
        }
    }
    
}

int
P1_GetProcInfo(int pid, P1_ProcInfo *info)
{
    int         result = P1_SUCCESS;
    if (pid < 0 || P1_MAXPROC <= pid || processTable[pid].state == P1_STATE_FREE) {
        return P1_INVALID_PID;
    }
    strcpy(info->name,processTable[pid].name);
    info->sid = processTable[pid].sid;
    info->state = processTable[pid].state;
    info->priority = processTable[pid].priority;
    info->tag = processTable[pid].tag;
    info->cpu = processTable[pid].cpuTime;
    info->parent = processTable[pid].parentPid;
    info->numChildren = processTable[pid].numChildren;
    int i;
    Node *head = processTable[pid].childrenPids;
    for(i = 0; i < processTable[pid].numChildren; i++){
        info->children[i] = head->val;
        head = head->next;
    }
    return result;
}