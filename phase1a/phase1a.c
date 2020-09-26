#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

extern  USLOSS_PTE  *P3_AllocatePageTable(int cid);
extern  void        P3_FreePageTable(int cid);
<<<<<<< HEAD
extern  void        *memset(void *str, int c, size_t n);
extern  int         memcmp(const void *str1, const void *str2, size_t n);
extern  void        *realloc(void *ptr, size_t size);
=======
// static  void        illegalMessage(int n,void *arg);
>>>>>>> b869cc8dc2f8308758462534e80833063134c85f

typedef struct Context {
    void            (*startFunc)(void *);
    void            *startArg;
    int wasCreated;
    USLOSS_Context  context;
    // you'll need more stuff here
<<<<<<< HEAD
    char *stack;
=======
    int isFree;
>>>>>>> b869cc8dc2f8308758462534e80833063134c85f
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
<<<<<<< HEAD
    checkInKernelMode();
=======
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
>>>>>>> b869cc8dc2f8308758462534e80833063134c85f
    // find a free context and initialize it
    // allocate the stack, specify the startFunc, etc.
    if (stacksize < USLOSS_MIN_STACK) {
        return P1_INVALID_STACK;
    }
    int i;
    for (i=0; i<P1_MAXPROC; i++) {
        if (contexts[i].wasCreated == 0) {
            // USLOSS_Console("Found Valid Memory at %d\n", i);
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
<<<<<<< HEAD
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
=======
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
>>>>>>> b869cc8dc2f8308758462534e80833063134c85f
}

int P1ContextFree(int cid) {
    checkInKernelMode();
    int result = P1_SUCCESS;
<<<<<<< HEAD
    if (-1 >= cid || cid >= P1_MAXPROC || !contexts[cid].wasCreated) {
        result = P1_INVALID_CID;
    } else if(cid == currentCid){
        result = P1_CONTEXT_IN_USE;
    }else{
        free(contexts[cid].stack);
        P3_FreePageTable(cid);
        contexts[cid].wasCreated = 0;
    }
=======
    
    // checking if the cid is valid
    // if(cid > currentCid){
    //     result = P1_INVALID_CID;
    // }
    // // checking if the cid is currently running
    // if(contexts[cid] != NULL){
    //     result = P1_CONTEXT_IN_USE;
    // }
    // contexts[cid].context.pageTable = P3_FreePageTable;
>>>>>>> b869cc8dc2f8308758462534e80833063134c85f
    return result;
}


void 
P1EnableInterrupts(void) 
{
    // set the interrupt bit in the PSR
    int bits = USLOSS_PsrGet();

    // checking if it is in kernal mode
<<<<<<< HEAD
    if(!(bits & USLOSS_PSR_CURRENT_MODE)){
        USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalMessage;
        USLOSS_IllegalInstruction();
    }

    if (!(bits & USLOSS_PSR_CURRENT_INT)) {
        int val = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
=======
    if((bits & 0x1)  == 0){
        USLOSS_IllegalInstruction();
        USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalMessage;
    }

    if((bits & 0x2) == 0){
        int val = USLOSS_PsrSet(USLOSS_PsrGet() | 0x2);
>>>>>>> b869cc8dc2f8308758462534e80833063134c85f
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
<<<<<<< HEAD
    if(!(bits & USLOSS_PSR_CURRENT_MODE)){
        USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalMessage;
        USLOSS_IllegalInstruction();
    }

    if(bits & USLOSS_PSR_CURRENT_INT) {
        int mode = bits & ~(bits & USLOSS_PSR_CURRENT_INT);
        // int mode = bits & 0xfd;
=======
    if((bits & 0x1)  == 0){
        USLOSS_IllegalInstruction();
    }

    if((bits & 0x2)  == 0x2){
        int mode = bits & 0xfd;
>>>>>>> b869cc8dc2f8308758462534e80833063134c85f

        int val = USLOSS_PsrSet(mode);
        assert(val == USLOSS_DEV_OK);
        enabled = TRUE;
    }
    return enabled;
}
