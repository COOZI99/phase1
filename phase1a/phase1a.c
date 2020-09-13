#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

extern  USLOSS_PTE  *P3_AllocatePageTable(int cid);
extern  void        P3_FreePageTable(int cid);
// static  void        illegalMessage(int n,void *arg);

typedef struct Context {
    void            (*startFunc)(void *);
    void            *startArg;
    USLOSS_Context  context;
    // you'll need more stuff here
    int isFree;
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

void P1ContextInit(void)
{
    // Checking if we are in kernal mode
    if(USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE){
        USLOSS_IllegalInstruction();
    }
    // Setting memory to 0
    memset(contexts, 0, sizeof(contexts));
    // Clearing memory

    // Calling illegal message and exiting program
    // USLOSS_IntVec[USLOSS_ILLEGAL_INT] = illegalMessage;
}

// static void illegalMessage(int n, void *arg){
//     USLOSS_Console("Error: Not in Kernel mode");
//     USLOSS_Halt(0);
// }

int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {
    int result = P1_SUCCESS;
    if (stacksize < USLOSS_MIN_STACK) {
        return P1_INVALID_STACK;
    }
    int i;
    for (i=0; i<P1_MAXPROC; i++) {
        unsigned char val = 0;
        if (memcmp(&contexts[i], &val, 1) == 0) {
        // if (contexts[i] == val) {
            contexts[i].startFunc = func;
            contexts[i].startArg = arg;
            char stack[stacksize];
            USLOSS_ContextInit(&contexts[i].context, stack, stacksize, P3_AllocatePageTable(i), launch);
            break;
        }
    }
    if (i == P1_MAXPROC) {
        return P1_TOO_MANY_CONTEXTS;
    }
    // find a free context and initialize it
    // allocate the stack, specify the startFunc, etc.
    return result;
}

int P1ContextSwitch(int cid) {
    USLOSS_Context context0;
    if (currentCid != -1) {
        context0 = contexts[currentCid].context;
    }
    USLOSS_Context context1;
    unsigned char val = 0;
    if (-1 < cid && cid < P1_MAXPROC && memcmp(&contexts[cid], &val, 1) != 0) {
        context1 = contexts[cid].context;
    } else {
        return P1_INVALID_CID;
    }
    int result = P1_SUCCESS;
    USLOSS_ContextSwitch(&context0, &context1);
    currentCid = cid;
    // switch to the specified context
    return result;
}

int P1ContextFree(int cid) {
    int result = P1_SUCCESS;
    
    return result;
}


void 
P1EnableInterrupts(void) 
{
    // set the interrupt bit in the PSR
}

/*
 * Returns true if interrupts were enabled, false otherwise.
 */
int 
P1DisableInterrupts(void) 
{
    int enabled = FALSE;
    // set enabled to TRUE if interrupts are already enabled
    // clear the interrupt bit in the PSR
    return enabled;
}
