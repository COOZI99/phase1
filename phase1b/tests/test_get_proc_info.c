#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Tests getting process information by printing out P1_ProcInfo fields
 */
// print information from P1_GetProcInfo()
void printInfo(int index){
	int rv, count;
        P1_ProcInfo *info = malloc(sizeof(P1_ProcInfo));

        rv = P1_GetProcInfo(index, info);
        assert(rv == P1_SUCCESS);
        USLOSS_Console("Name: %s, State: %d, Sid: %d, Priority: %d, Tag: %d, CpuTime: %d, ParentPID: %d, ChildrenPIDS: ",
                   info->name, (int)info->state, info->sid, info->priority, info->tag, info->cpu, info->parent);
        count = info->numChildren;
        int i;
        for(i = 0; i < count; i++){
            USLOSS_Console("%d, ", info->children[i]);
        }
        USLOSS_Console("# of Children: %d\n", count);
	free(info);
}

int Child(void *arg){
	
	USLOSS_Console("In Child%d, printing proc info\n", (int) arg);
	printInfo(2);
	printInfo(1);
	printInfo(0);
 	USLOSS_Halt(0);	
	return 0;	
}


int 
Parent(void *arg)
{   
    int     rc;
    int     child;
    int     priority = (int) arg; 
    char name[100];
    int  x = priority - 1;
    snprintf(name, sizeof(name), "Child%d", x);
    
    // parent forks a child 
    rc = P1_Fork(name, Child, (void *) x, USLOSS_MIN_STACK, x, 0, &child);
    assert(rc == P1_SUCCESS);
    // clean up any children that have quit
    return 0;
}

int P6Proc(void *arg){
    int pid;
    USLOSS_Console("Parent forks Child\n");
    int rc = P1_Fork("Parent", Parent, (void *) 2, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);
    return 0;
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    P1ProcInit();
    USLOSS_Console("startup\n");
    USLOSS_Console("Forks P6Proc with priority 6, P6Proc forks Parent\n");
    rc = P1_Fork("P6Proc", P6Proc, (void *) 2, USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS); 
    // should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
