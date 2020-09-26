/**
 *  Authors: Bianca Lara, Ann Chang
 *  Due Date: September 16th, 2020
 *  Phase 1a
 *  Submission Type: Group
 *  Comments: The phase 1a implementation of phase 1. Implements the 
 *  outlined functions for creating, switching, and freeing contexts
 *  as well as enabling and disabling interrupts. The Context struct
 *  had two variables added to it: int wasCreated and char *stack
 *  which determines if the context was initialized and stores the
 *  context stack respectively.
 */

#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

extern  USLOSS_PTE  *P3_AllocatePageTable(int cid);
extern  void        P3_FreePageTable(int cid);
extern  void        *memset(void *str, int c, size_t n);

typedef struct Context {
    void            (*startFunc)(void *);
    void            *startArg;
    int wasCreated;
    USLOSS_Context  context;
    char *stack;
} Context;

static Context   contexts[P1_MAXPROC];

static int currentCid = -1;

/*
 * Helper function to call func passed to P1ContextCreate with its arg.
 */
static void launch(void)
{
    assert(contexts[currentCid].startFunc != NULL);
    contexts[currentCid].startFunc(contexts[currentCid].startArg);
}

// Halting the program for an illegal message
static void IllegalMessage(int n, void *arg){
    USLOSS_Halt(0);
}

static void checkInKernelMode() {
    // Checking if we are in kernal mode
    if(!(USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE)){
        USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalMessage;
        USLOSS_IllegalInstruction();
    }
}

void P1ContextInit(void)
{
    checkInKernelMode();
    // Setting memory to 0
    memset(contexts, 0, P1_MAXPROC * sizeof(Context));
}

int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {
    checkInKernelMode();
    // find a free context and initialize it
    // allocate the stack, specify the startFunc, etc.
    if (stacksize < USLOSS_MIN_STACK) {
        return P1_INVALID_STACK;
    }
    int i;
    for (i=0; i<P1_MAXPROC; i++) {
        if (contexts[i].wasCreated == 0) {
            contexts[i].startFunc = func;
            contexts[i].startArg = arg;
            contexts[i].wasCreated = 1;
            contexts[i].stack = (char *)malloc(stacksize * sizeof(char));
            
            USLOSS_ContextInit(
                /* *context= */  &contexts[i].context,
                /* *stack= */     contexts[i].stack,
                /* stackSize= */  stacksize,
                /* *pageTable= */ P3_AllocatePageTable(i),
                /* (*func)= */    launch);
            *cid = i;
            return P1_SUCCESS;
        }
    }
    return P1_TOO_MANY_CONTEXTS;
}

int P1ContextSwitch(int cid) {
    // Switching from the current context to the specified context
    checkInKernelMode();
    if (cid == currentCid) {
        return P1_SUCCESS; // already there, do nothing.
    }
    
    if (cid <= -1 || P1_MAXPROC <= cid || !contexts[cid].wasCreated) {
        return P1_INVALID_CID;
    }

    int old = currentCid;
    currentCid = cid;
    if (currentCid > -1) {
        USLOSS_ContextSwitch(&contexts[old].context, &contexts[cid].context);
    } else {
        USLOSS_ContextSwitch(NULL, &contexts[cid].context);
    }
    return P1_SUCCESS;
}

int P1ContextFree(int cid) {
    // Frees the specified context if it was previously created
    checkInKernelMode();
    int result = P1_SUCCESS;
    if (-1 >= cid || cid >= P1_MAXPROC || !contexts[cid].wasCreated) {
        result = P1_INVALID_CID;
    } else if(cid == currentCid){
        result = P1_CONTEXT_IN_USE;
    }else{
        free(contexts[cid].stack);
        P3_FreePageTable(cid);
        contexts[cid].wasCreated = 0;
    }
    return result;
}

void 
P1EnableInterrupts(void) 
{
    // set the interrupt bit in the PSR
    int bits = USLOSS_PsrGet();

    // checking if it is in kernal mode
    if(!(bits & USLOSS_PSR_CURRENT_MODE)){
        USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalMessage;
        USLOSS_IllegalInstruction();
    }

    if (!(bits & USLOSS_PSR_CURRENT_INT)) {
        int val = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
        assert(val == USLOSS_DEV_OK);
    }
}

/*
 * Returns true if interrupts were enabled, false otherwise.
 */
int 
P1DisableInterrupts(void) 
{
    checkInKernelMode();
    int enabled = FALSE;
    // set enabled to TRUE if interrupts are already enabled
    // clear the interrupt bit in the PSR
    int bits = USLOSS_PsrGet();

    // checking if it is in kernel mode
    if(!(bits & USLOSS_PSR_CURRENT_MODE)){
        USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalMessage;
        USLOSS_IllegalInstruction();
    }

    if(bits & USLOSS_PSR_CURRENT_INT) {
        int mode = bits & ~(bits & USLOSS_PSR_CURRENT_INT);
        // int mode = bits & 0xfd;

        int val = USLOSS_PsrSet(mode);
        assert(val == USLOSS_DEV_OK);
        enabled = TRUE;
    }
    return enabled;
}
