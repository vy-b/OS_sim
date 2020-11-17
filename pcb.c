#include "pcb.h"

static PCB* current_running = NULL;
static List* arrayOfQueues[3];
static PCB* init = NULL;
static List* blockedOnSend;
static List* blockedOnReceive;
static int queue0count = 0;
static int queue1count = 0;
static int queue2count = 0;

// should find the process with the specified pid from the queue
// returns the queue containing the pid with the current pointer at the specified process
List* find_by_pid(int pid_input)
{
    int priorityQueueNo = pid_input % 3;
    // store the search queue inside the parameter
    List* searchingQueue = arrayOfQueues[priorityQueueNo];
    if (List_count(searchingQueue) == 0)
    {
        printf("PID does not exist.\n");
        return NULL;
    }
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

    PCB* found_process = List_prev(searchingQueue);

    return searchingQueue;
}

// helper function
// chooses the next process by traversing through the list of queues and choosing the
// first process in the highest priority
int next_process()
{
    int available_ready = 0;

    // let another process run
    for (int i = 0; i<3; i++)
    {
        if (List_count(arrayOfQueues[i]) > 0)
        {
            current_running = List_first(arrayOfQueues[i]);
            current_running->state = running;
            available_ready = 1;
            break;
        }
    }
    if (available_ready == 0)
    {
        current_running = init;
        init->state = running;
    }
}


// helper function!
// generates a new pid based on priority - 
// if priority = 0, PID % 3 = 0
// if priority = 1, PID % 3 = 1
int generate_newPID(int priority_input)
{
    int newPID;
    if (priority_input == 0)
    {
        newPID = (queue0count+1)*3;
        queue0count++;
    }
    else if (priority_input == 1)
    {
        newPID = (queue1count+1)*3 + 1;
        queue1count++;
    }
    else
    {
        newPID = (queue2count+1)*3 + 2;
        queue2count++;
    }
    return newPID;
}

void initialize_queues()
{
    for (int i = 0; i < 3; i++)
    {
        arrayOfQueues[i] = List_create();
    }
    blockedOnReceive = List_create();
    blockedOnReceive = List_create();
}


// start the init process
// init process does not belong on any queue

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
    init = init_process;
    return init_process->PID;
}

void cancel_simulation(){
    free(init);
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

    List_append(arrayOfQueues[priority_input], newPCB);

    if (current_running == NULL || current_running == init)
    {
        newPCB->state = running;
        current_running = newPCB;
    }
    return newPCB->PID;
}

// forks the currently running process, give the same priority as the running process
int fork_PCB()
{
    if (current_running == NULL || current_running == init)
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

    List_append(arrayOfQueues[forked_priority], forkedPCB);
    printf("Forked priority = %d\n", forkedPCB->priority);
    return forkedPCB->PID;
}



// Kills the process with the PID entered by user
// uses helper function find_by_pid to find and remove the specified pid
// if pid not found, report error
int kill_PCB(int pid_input)
{
    if (pid_input == 0)
    // perform a check if there are any others on queues
    {
        for (int i = 0; i<3; i++)
        {
            if (List_count(arrayOfQueues[i] > 0));
            else 
                printf("nothing to exit from.\n");
        }
        // if there aren't any on queues,
        free(init);
        return 1;
    }

    // find the searching queue and the process using the find_by_pid function
    List* containingQueue = find_by_pid(pid_input);
    PCB* toKill = List_remove(containingQueue);
    if (current_running == toKill)
    {
        current_running = NULL;
        next_process();
    }
    free(toKill);
    toKill = NULL;

    printf("Killed process number %d Success\n",pid_input);
    return 0;
}

// Kills the currently running process
int PCB_exit()
{
    if (current_running == init)
    {
        int queuedProcs = 0;
        // if there are no processes running, check if there are any on the queues
        for (int i = 0; i<3; i++)
        {
            // if there are items on queue, break loop
            if (List_count(arrayOfQueues[i]) > 0)
            {
                queuedProcs = 1;
                break;
            }
        }
        // if there aren't any on queues, exit simulation
        if (queuedProcs == 0)
        {
            free(init);
            return 1; //to signal that init is exiting
        }
    }

    int ret_pid = current_running->PID;
    List* containingQueue = find_by_pid(ret_pid);
    List_remove(containingQueue);
    free(current_running);
    current_running = NULL;

    printf("Currently running process with pid %d has been killed\n",ret_pid);
    next_process();
    return 0;
}

int PCB_quantum()
{
    // remove current running process and put it in the back of its queue
    List* containingQueue = find_by_pid(current_running->PID);
    PCB* toRotate = List_remove(containingQueue);
    if (toRotate == NULL)
    {
        printf("error in quantum\n");
        return -1;
    }
    current_running = NULL;
    toRotate->state = ready;
    // pick the next process BEFORE appending (give lower priorities a chance if high priority just got quantum-ed)
    next_process();
    List_append(arrayOfQueues[toRotate->priority],toRotate);
    
    return current_running->PID;
}

int sendto_PCB(int sendto_pid, char* msg)
{
    List* receiverQueue = find_by_pid(sendto_pid);
    // remove the receiver from the containing queue and add to sendblock list
    PCB* receiver = List_remove (receiverQueue);
    receiver->msg_received = msg;
    receiver->sender = current_running;

    // block the sender until a reply is received
    List* senderQueue = find_by_pid(current_running->PID);
    PCB* toBlock = List_curr(senderQueue);
    List_append(blockedOnSend, toBlock);
    toBlock->state = blocked;
    

    nextprocess();
}

int recvfrom_PCB()
{
    List* receiverQueue = find_by_pid(current_running);
    PCB* receiver = List_curr(receiverQueue);
    receiver->state = blocked;
    List_append(blockedOnReceived, receiver);

    if (receiver->msg_received != NULL)
    {
        printf("message received on process with ID %d from sender %d: %s\n",
            receiver->PID, receiver->sender->PID,msg_received);
        
    }
}


//_________________________testing functions beyond this point_____________________-
//____________________________________________________________________________________
//___________________________________________________________________________________//____________________________________________________________________________________
//___________________________________________________________________________________//____________________________________________________________________________________
//___________________________________________________________________________________//____________________________________________________________________________________
//___________________________________________________________________________________//____________________________________________________________________________________
//___________________________________________________________________________________//____________________________________________________________________________________
//___________________________________________________________________________________//____________________________________________________________________________________
//___________________________________________________________________________________//____________________________________________________________________________________
//___________________________________________________________________________________//____________________________________________________________________________________
//___________________________________________________________________________________//____________________________________________________________________________________
//___________________________________________________________________________________//____________________________________________________________________________________
//___________________________________________________________________________________//____________________________________________________________________________________
//___________________________________________________________________________________
void test_prints(int pid_input)
{
    int priorityQueueNo = pid_input % 3;

    List* searchingQueue = arrayOfQueues[priorityQueueNo];
    if (List_count(searchingQueue) == 0)
    {
        printf("PID does not exist.\n");
        return NULL;
    }
    PCB* current_process = List_first(searchingQueue);
    void* current_pointer = List_next(searchingQueue);
    int placeInQueue = 0;
    while (current_process->PID != pid_input && current_pointer!=NULL)
    {
        placeInQueue++;
        current_process = current_pointer;
        current_pointer = List_next(searchingQueue);
    }

    if (current_process->PID != pid_input && current_pointer == NULL)
    {
        printf("Process not found. PID does not exist. Please execute K again with correct ID\n");
        return -1;
    }
    PCB* toPrint = List_prev(searchingQueue);
    printf("PID: %d\n",toPrint->PID);
    printf("state: %d\n",toPrint->state);
    printf("priority: %d\n",toPrint->priority);
    printf("place in queue: %d\n",placeInQueue);
}

void test_current_running()
{
    printf("PID: %d\n",current_running->PID);
    printf("state: %d\n",current_running->state);
    printf("priority: %d\n",current_running->priority);
}

void print_everything_inQueue()
{
    for (int i = 0; i < 3; i++)
    {
        if (List_count(arrayOfQueues[i]) == 0)
            continue;
        printf("Queue %d:\n",i);
        PCB* current = List_first(arrayOfQueues[i]);
        while (current != NULL )
        {
            printf("%d ",current->PID);
            current = List_next(arrayOfQueues[i]);
        }
        printf("\n");
    }
}