#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

extern  USLOSS_PTE  *P3_AllocatePageTable(int cid);
extern  void        P3_FreePageTable(int cid);
extern  void        *memset(void *str, int c, size_t n);
extern  int         memcmp(const void *str1, const void *str2, size_t n);
extern  void        *realloc(void *ptr, size_t size);

typedef struct Context {
    void            (*startFunc)(void *);
    void            *startArg;
    int wasCreated;
    USLOSS_Context  context;
    // you'll need more stuff here
    char *stack;
} Context;

static Context   contexts[P1_MAXPROC];

static int currentCid = -1;

/*
 * Helper function to call func passed to P1ContextCreate with its arg.
 */
static void launch(void)
{
    // USLOSS_Console("launching...\n");
    assert(contexts[currentCid].startFunc != NULL);
    contexts[currentCid].startFunc(contexts[currentCid].startArg);
}

static void IllegalMessage(int n, void *arg){
    USLOSS_Console("Error: Not in Kernel mode\n");
    USLOSS_Halt(0);
}

void P1ContextInit(void)
{
    // Checking if we are in kernal mode
    if(!(USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE)){
        USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalMessage;
        USLOSS_IllegalInstruction();
    }
    // Setting memory to 0
    memset(contexts, 0, P1_MAXPROC);
    // Clearing memory

    // Calling illegal message and exiting program
    // USLOSS_IntVec[USLOSS_ILLEGAL_INT] = illegalMessage;
}

int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {
    // find a free context and initialize it
    // allocate the stack, specify the startFunc, etc.
    if (stacksize < USLOSS_MIN_STACK) {
        return P1_INVALID_STACK;
    }
    int i;
    for (i=0; i<P1_MAXPROC; i++) {
        if (!contexts[i].wasCreated) {
            USLOSS_Console("Found Valid Memory at %d\n", i);
            contexts[i].startFunc = func;
            contexts[i].startArg = arg;
            contexts[i].wasCreated = 1;
            contexts[i].stack = (char *)realloc(contexts[i].stack, stacksize);
            
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
    if (cid == currentCid) {
        return P1_SUCCESS; // already there, do nothing.
    }
    // switch to the specified context
    USLOSS_Context context0;
    if (currentCid > -1) {
        context0 = contexts[currentCid].context;
    }

    USLOSS_Context context1;
    if (cid <= -1 || P1_MAXPROC <= cid || !contexts[cid].wasCreated) {
        return P1_INVALID_CID;
    }
    context1 = contexts[cid].context;
    
    currentCid = cid;
    if(currentCid == -1){
        USLOSS_ContextSwitch(NULL, &context1);
    }else{
        USLOSS_ContextSwitch(&context0, &context1);
    } 
    return P1_SUCCESS;
}

int P1ContextFree(int cid) {
    int result = P1_SUCCESS;
    if (-1 >= cid || cid >= P1_MAXPROC) {
        result = P1_INVALID_CID;
    } else if(contexts[cid].wasCreated){
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
