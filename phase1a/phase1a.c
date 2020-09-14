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
}

int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {
    // find a free context and initialize it
    // allocate the stack, specify the startFunc, etc.
    if (stacksize < USLOSS_MIN_STACK) {
        return P1_INVALID_STACK;
    }
    int i;
    for (i=0; i<P1_MAXPROC; i++) {
        if (contexts[i].wasCreated == 0) {
            USLOSS_Console("Found Valid Memory at %d\n", i);
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
    // USLOSS_Console("Calling Switch\n");
    if (cid == currentCid) {
        return P1_SUCCESS; // already there, do nothing.
    }

    if (cid <= -1 || P1_MAXPROC <= cid || !contexts[cid].wasCreated) {
        return P1_INVALID_CID;
    }

    int old = currentCid;
    currentCid = cid;
<<<<<<< HEAD
    if (currentCid > -1) {
        USLOSS_ContextSwitch(&contexts[old].context, &contexts[cid].context);
    } else {
        USLOSS_ContextSwitch(NULL, &contexts[cid].context);
    }
=======
    if(currentCid == -1){
        USLOSS_ContextSwitch(NULL, &context1);
    }else{
        USLOSS_ContextSwitch(&context0, &context1);
    } 
>>>>>>> fbcdef18c2356d51d338064ccd17d0ee96a1b0af
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
