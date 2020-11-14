#include "pcb.h"

static PCB* current_running = NULL;
static List* arrayOfQueues[3];

void initialize_queues()
{
    for (int i = 0; i < 3; i++)
    {
        arrayOfQueues[i] = List_create();
    }
}


int start_simulation()
{
    PCB* init_process = malloc(sizeof(PCB));
    if (!init_process)
    {
        return -1;
    }
    init_process->PID = 0;
    init_process->priority = 0;
    init_process->state = ready;
    return init_process->PID;
}

// create a new PCB with the priority specified and place on ready queue
int create_PCB(int priority_input)
{
    PCB* newPCB = malloc(sizeof(PCB));

    if (!newPCB)
        return -1;

    int newPID = generate_newPID(priority_input);

    newPCB->PID = newPID;
    newPCB->priority = priority_input;
    newPCB->state = ready;

    List_add(arrayOfQueues[priority_input], newPCB);

    if (current_running == NULL)
    {
        newPCB->state = running;
        current_running = newPCB;
    }
    return newPCB->PID;
}

// forks the currently running process, give the same priority as the running process
int fork_PCB()
{
    if (current_running == NULL)
    {
        printf("no running processes to fork\n");
        return -1;
    }
    PCB* forkedPCB = malloc(sizeof(PCB));
    if (!forkedPCB)
        return -1;
    
    int forked_priority = current_running->priority;
    int newPID = generate_newPID(forked_priority);

    forkedPCB->PID = newPID;
    forkedPCB->priority = forked_priority;
    forkedPCB->state = ready;

    List_add(arrayOfQueues[forked_priority], forkedPCB);
    printf("Forked priority = %d\n", forkedPCB->priority);
    return forkedPCB->PID;
}

// helper function!
// generates a new pid based on priority - 
// if priority = 0, PID % 3 = 0
// if priority = 1, PID % 3 = 1
int generate_newPID(int priority_input)
{
    List* priorityQueue = arrayOfQueues[priority_input];

    int newPID;
    if (priority_input == 0)
    {
        newPID = (List_count(priorityQueue)+1)*3;
    }
    else if (priority_input == 1)
    {
        newPID = (List_count(priorityQueue)+1)*3 + 1;
    }
    else
    {
        newPID = (List_count(priorityQueue)+1)*3 + 2;
    }
    return newPID;
}

// Kills the process with the PID entered by user
// searches the appropriate ready queue and free the process if pid is found
// if pid not found, report error
int kill_PCB(int pid_input)
{
    int priorityQueueNo = pid_input % 3;

    List* searchingQueue = arrayOfQueues[priorityQueueNo];
    PCB* current_process = List_first(searchingQueue);
    void* current_pointer = List_next(searchingQueue);

    while (current_process->PID != pid_input && current_pointer!=NULL)
    {
        current_process = current_pointer;
        current_pointer = List_next(searchingQueue);
    }

    if (current_process->PID != pid_input && current_pointer == NULL)
    {
        printf("Process not found. PID does not exist. Please execute K again with correct ID\n");
        return -1;
    }

    PCB* toKill = List_remove(searchingQueue);

    free(toKill);

    if (toKill!=NULL)
    {
        printf("Failed to kill process %d\n",pid_input);
        return -1;
    }

    printf("Killed process number %d Success\n",pid_input);
    return 0;
}

// Kills the currently running process
int PCB_exit()
{
    int ret_pid = current_running->PID;
    free(current_running);
    if (current_running!=NULL)
    {
        printf("Failed to kill the currently executing process\n");
        return -1;
    }
    printf("Currently running process with pid %d has been killed\n",ret_pid);
    return 0;
}