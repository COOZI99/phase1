#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

extern  USLOSS_PTE  *P3_AllocatePageTable(int cid);
extern  void        P3_FreePageTable(int cid);
extern  void        *memset(void *str, int c, size_t n);
extern  int         memcmp(const void *str1, const void *str2, size_t n);

typedef struct Context {
    void            (*startFunc)(void *);
    void            *startArg;
    USLOSS_Context  context;
    // you'll need more stuff here
} Context;

static Context   contexts[P1_MAXPROC];

static int currentCid = -1;

/*
 * Helper function to call func passed to P1ContextCreate with its arg.
 */
static void launch(void)
{
    USLOSS_Console("launching...\n");
    assert(contexts[currentCid].startFunc != NULL);
    contexts[currentCid].startFunc(contexts[currentCid].startArg);
}

void P1ContextInit(void)
{
    // Checking if we are in kernal mode
    if(!(USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE)){
        USLOSS_IllegalInstruction();
    }
    // Setting memory to 0
    memset(contexts, 0, sizeof(contexts));
    // Clearing memory

    // Calling illegal message and exiting program
    // USLOSS_IntVec[USLOSS_ILLEGAL_INT] = illegalMessage;
}

char stack[USLOSS_MIN_STACK];

int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {
    int result = P1_SUCCESS;
    if (stacksize < USLOSS_MIN_STACK) {
        return P1_INVALID_STACK;
    }
    USLOSS_Console("Stack is Valid\n");
    int i;
    
    for (i=0; i<P1_MAXPROC; i++) {
        unsigned char val = 0;
        USLOSS_Console("Checking for Valid Memory\n");
        if (memcmp(&contexts[i], &val, 1) == 0) {
            USLOSS_Console("Found Valid Memory at %d\n", i);
            contexts[i].startFunc = func;
            contexts[i].startArg = arg;
            // char stack[stacksize];
            USLOSS_ContextInit(&contexts[i].context, stack, USLOSS_MIN_STACK, P3_AllocatePageTable(i), launch);
            USLOSS_Console("Init Context\n");
            USLOSS_Console("Setting cid\n");
            *cid = i;
            break;
        }
        USLOSS_Console("No free mem found");
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
    currentCid = cid;
    USLOSS_ContextSwitch(&context0, &context1);
    
    // switch to the specified context
    return result;
}

int P1ContextFree(int cid) {
    int result = P1_SUCCESS;
    // free the stack and mark the context as unused
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
