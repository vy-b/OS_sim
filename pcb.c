#include "pcb.h"

static PCB* current_running = NULL;
static List* arrayOfQueues[3];
static PCB* init = NULL;
static List* blockedOnSend;
static List* blockedOnReceive;
static List* semaphores;
static List* blockedOnSem;
static int queue0count = 0;
static int queue1count = 0;
static int queue2count = 0;
static char* msg_received = NULL;

// should find the process with the specified pid from the queue
// returns the queue containing the pid with the current pointer at the specified process
PCB* find_by_pid(int pid_input, List* searchingQueue)
{
    if (List_count(searchingQueue) == 0)
    {
        return NULL;
    }

    // search through the queue
    PCB* current_process = List_first(searchingQueue);
    void* current_pointer = List_next(searchingQueue);

    while (current_process->PID != pid_input && current_pointer!=NULL)
    {
        current_process = current_pointer;
        current_pointer = List_next(searchingQueue);
    }

    if (current_process->PID != pid_input && current_pointer == NULL)
    {
        return NULL;
    }

    PCB* found_process = List_prev(searchingQueue);

    return found_process;
}


// helper function
// chooses the next process by traversing through the list of queues and choosing the
// first process in the highest priority
int next_process()
{
    int available_ready = 0;

    // let another process run
    // look in ready queues and find the first available item
    for (int i = 0; i<3; i++)
    {
        if (List_count(arrayOfQueues[i]) > 0)
        {
            current_running = List_first(arrayOfQueues[i]);
            current_running->state = running;
            available_ready = 1;
        }
        if (available_ready == 1)
            break;
    }
    if (available_ready == 0)
    {
        current_running = init;
        init->state = running;
    }
    printf("now running process: %d\n", current_running->PID);
    return current_running->PID;
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
    blockedOnSend = List_create();
    semaphores = List_create();
    blockedOnSem = List_create();
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
    init_process->priority = -1;
    init_process->state = running;
    init = init_process;
    current_running = init_process;
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
        int queuedProcs = 0;
        // check if there are any blocked
        if (List_count(blockedOnReceive) > 0 || List_count(blockedOnSend) > 0)
        {
            queuedProcs = 1;
        }
        else 
            // if there are no processes blocked or running, check if there are any on the queues
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
            printf("init exiting\n");
            free(init);
            Sem* current = List_first(semaphores);
            while (current != NULL)
            {
                free(current);
                current = List_next(semaphores);
            }
            return 1; //to signal that init is exiting
        }
        else 
        {
            printf("other processes still exist/blocked, cannot exit.\n");
        }
    }

    // find the searching queue and the process using the find_by_pid function
    List* searchingQueue = arrayOfQueues[pid_input % 3];
    PCB* toKill = find_by_pid(pid_input, searchingQueue);

    // case: pid doesn't exist, find in blocked queues
    if (toKill == NULL)
    {
        toKill = find_by_pid(pid_input,blockedOnSend);

        if (toKill==NULL)
        {
            toKill = find_by_pid(pid_input,blockedOnReceive);
            if (toKill == NULL)
            {
                toKill = find_by_pid(pid_input,blockedOnSem);
                if (toKill == NULL)
                {
                    printf("PID does not exist.\n");
                    return -1;
                }
                else
                {
                    if (toKill->blocker != NULL)
                    {
                        List* waitlist = toKill->blocker->waiting_processes;
                        find_by_pid(pid_input,waitlist);
                        List_remove(waitlist);         
                    }
                    List_remove(blockedOnSem);
                }
            }
            else
                List_remove(blockedOnReceive);
        }
        else
        {
            List_remove(blockedOnSend);
        }
        free(toKill);
        toKill = NULL;
        printf("Killed process number %d Success\n",pid_input);
        return 0;
    }

    List_remove(searchingQueue);
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
    return kill_PCB(current_running->PID);
}

int PCB_quantum()
{
    // first check if this is the ONLY process running
    if (List_count(arrayOfQueues[current_running->priority]) == 1)
    {
        int available_ready = 0;
        for (int i = 0; i<3; i++)
        {
            if (i == current_running->priority)
                continue;
            // if there are items on queue, break loop
            else if (List_count(arrayOfQueues[i]) > 0)
            {
                available_ready = 1;
                break;
            }
        }
        if (available_ready == 0)
        {
            printf("No other processes to run, continue current process.\n");
            // if this is the ONLY running process, do nothing
            return 0;
        }
    }
    List* searchingQueue = arrayOfQueues[current_running->priority];

    // remove current running process and put it in the back of its queue
    PCB* toRotate = find_by_pid(current_running->PID, searchingQueue);
    if (toRotate == NULL)
    {
        printf("error in quantum\n");
        return -1;
    }
    List_remove(searchingQueue);
    current_running = NULL;
    toRotate->state = ready;
    // pick the next process BEFORE appending (give lower priorities a chance if high priority just got quantum-ed)
    next_process();
    List_append(arrayOfQueues[toRotate->priority],toRotate);
    
    return current_running->PID;
}

// param: pid of process to send to, null terminated char msg
// send a message to another process - block until reply
// return success or failure, scheduling info (what process runs while this is blocked)
// reply source and text (prints when reply arrives)
int sendto_PCB(int sendto_pid, char* msg)
{
    if (current_running == init)
    {
        printf("No running processes.\n");
    }
    if (sendto_pid == current_running->PID)
    {
        printf("SEND FAILURE: cannot send message to self!\n");
        return -1;
    }
    else if (sendto_pid == 0)
    {
        printf("SEND FAILURE: Process does not exist.\n");
    }
    else if (current_running->PID == 0)
    {
        printf("SEND FAILURE: No running processes\n");
    }
    // if receiver is found in blockedonreceive, unblock receiver
    // put in the back of its priority queue
    else if (find_by_pid(sendto_pid, blockedOnReceive) != NULL)
    {
        PCB* blockedReceiver = List_remove(blockedOnReceive);
        List_append(arrayOfQueues[blockedReceiver->priority],blockedReceiver);
        blockedReceiver->state = ready;
        blockedReceiver->sender = current_running;
        if (current_running == init)
        {
            current_running = blockedReceiver;
            blockedReceiver->state = ready;
        }
        printf("Message received on process %d from currently running sender %d: %s\n",
            blockedReceiver->PID,current_running->PID,msg);
        printf("Receiver %d previously waiting now unblocked.\n", blockedReceiver->PID);
    }
    else
    {
        PCB* receiver = find_by_pid(sendto_pid,arrayOfQueues[sendto_pid % 3]);
        if (receiver == NULL)
        {
            printf("SEND FAILURE: PID does not exist or process currently blocked.\n");
            return -1;
        }
        msg_received = msg;
        receiver->sender = current_running;
    }
    // block the sender until a reply is received
    printf("Blocking %d on send.\n", current_running->PID);
    List* senderQueue = arrayOfQueues[current_running->priority];
    PCB* toBlock = find_by_pid(current_running->PID,senderQueue);
    List_remove(senderQueue);
    List_append(blockedOnSend, toBlock);
    toBlock->state = blocked;
    
    next_process();
    return 0;
}

//Receive (R) checks if there is a message waiting for the currently executing process,
// if there is it receives it, otherwise it gets blocked
// receives a message - block until one arrives
// scheduling information and (once a message is received)
// the message text and source of message
// no parameters
int recvfrom_PCB()
{
    if (current_running == init)
    {
        printf("No running processes.\n");
    }
    PCB* receiver = current_running;
    if (msg_received != NULL)
    {
        printf("message received on currently running process with ID %d from sender %d: %s\n",
            receiver->PID, receiver->sender->PID,msg_received);
        free(msg_received);
        msg_received = NULL;
        return 0;
    }
    // block the process if no message is received
    receiver->state = blocked;
    find_by_pid(current_running->PID, arrayOfQueues[current_running->priority]);
    List_remove(arrayOfQueues[receiver->priority]);
    List_append(blockedOnReceive, receiver);
    printf("Process %d now blocked on receive.\n",receiver->PID);
    next_process();
    return 0;
}

// reply: unblocks sender and delivers reply
// param: int pid (pid of process to reply to) - assumption: can't reply to something that didn't send you a message; replying only to the most recent sender
// paramL char* msg (null terminated with 40 char max)
// returns success or failure
int reply_PCB(int replyto_pid,char* msg)
{
    if (current_running == init)
    {
        printf("No running processes.\n");
    }
    if (replyto_pid != current_running->sender->PID)
    {
        printf("cannot reply to a process that hasn't sent anything!\n");
        return -1;
    }

    PCB* sender = current_running->sender;
    sender->state = ready;
    find_by_pid(sender->PID,blockedOnSend);
    List_remove(blockedOnSend);
    // edge case: if init is the running process
    if (current_running == init)
    {
        current_running = sender;
        sender->state = running;
    }
    List_add(arrayOfQueues[sender->priority],sender);
    printf("reply received from process %d, message is: %s\n",current_running->PID,msg);
    printf("sender %d unblocked.\n", sender->PID);
    return 0;
}

// search for semaphore
Sem* find_sem(int sem_ID)
{
    if (List_count(semaphores) == 0)
    {
        return NULL;
    }
    // search through the queue
    Sem* current_sem = List_first(semaphores);
    void* current_pointer = List_next(semaphores);

    while (current_sem->ID != sem_ID && current_pointer!=NULL)
    {
        current_sem = current_pointer;
        current_pointer = List_next(semaphores);
    }

    if (current_sem->ID != sem_ID && current_pointer == NULL)
    {
        return NULL;
    }

    Sem* found_sem = List_prev(semaphores);

    return found_sem;
}

// new semaphore
int new_sem(int sem_ID,int initial_value)
{
    if ( find_sem(sem_ID)!=NULL)
    {
        printf("This ID already exists.\n");
        return -1;
    }
    if (sem_ID < 0 || sem_ID > 4)
    {
        printf("Creating new semaphore FAILED. Invalid ID, enter any number 0-4\n");
        return -1;
    }
    Sem* newSem = malloc(sizeof(Sem));
    newSem->ID = sem_ID;
    newSem->val = initial_value;
    newSem->waiting_processes = List_create();
    List_append(semaphores,newSem);
    return 0;
}
// semaphore P
int semaphore_P(int sem_ID)
{
    Sem* semaphoreP = find_sem(sem_ID);
    if (semaphoreP == NULL)
    {
        printf("Semaphore P failed. Sem does not exist/Invalid ID.\n");
        return -1;
    }
    semaphoreP->val--;
    if (semaphoreP->val <= 0) //block
    {
        if (current_running == init)
        {
            printf("No running processes to block.\n");
        }
        List_prepend(semaphoreP->waiting_processes,current_running);
        List_prepend(blockedOnSem, current_running);
        find_by_pid(current_running->PID,arrayOfQueues[current_running->priority]);
        List_remove(arrayOfQueues[current_running->priority]);
        current_running->state = blocked;
        current_running->blocker = semaphoreP;
        printf("Blocking process %d, semaphore value negative.\n",current_running->PID);
        next_process();
    }
    return 0;
}
// asusmption: waking up from semaphore goes to the back of ready queue
int semaphore_V(int sem_ID)
{
    Sem* semaphoreV = find_sem(sem_ID);
    if (semaphoreV == NULL)
    {
        printf("Semaphore V failed. Sem does not exist/Invalid ID.\n");
        return -1;
    }

    semaphoreV->val++;
    if (semaphoreV->val > 0)
    {
        if (List_count(semaphoreV->waiting_processes) == 0)
        {
            printf("Nothing waiting on this semaphore right now.\n");
            return 0;
        }
        List_trim(semaphoreV->waiting_processes);
        PCB* wakeProcess = List_trim(blockedOnSem);
        List_append(arrayOfQueues[wakeProcess->priority], wakeProcess);
        wakeProcess->state = ready;
        if (current_running == init)
        {
            current_running = wakeProcess;
            wakeProcess->state = running;
        }
        wakeProcess->blocker = NULL;
        printf("Waking process %d, semaphore value now positive.\n",wakeProcess->PID);
    }
    return 0;
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
int test_prints(int pid_input)
{
    char* queueName = NULL;
    PCB* toPrint = find_by_pid(pid_input, arrayOfQueues[pid_input%3]);
    if (toPrint == NULL)
    {
        toPrint = find_by_pid(pid_input, blockedOnReceive);
        if (toPrint == NULL)
        {
            toPrint = find_by_pid(pid_input, blockedOnSend);
            if (toPrint == NULL)
            {
                toPrint = find_by_pid(pid_input, blockedOnSem);
                if (toPrint == NULL)
                {
                    printf("Process not found!\n");
                    free(queueName);
                    return -1;
                }
                else
                {
                    queueName = "Blocked on a semaphore";
                }
            }
                
            else
            {
                queueName = "Blocked on Send";
            }
        }
        else
        {
            queueName = "Blocked on Receive";
        }
    }
    else
    {
        queueName = "Priority";
    }
    printf("PID: %d\n",toPrint->PID);

    char* state = NULL;
    if (toPrint->state == 0)
        state = "running";
    else if (toPrint->state == 1)
        state = "ready";
    else if (toPrint->state == 2)
        state = "blocked";

    printf("state: %s\n",state);
    printf("priority: %d\n",toPrint->priority);
    printf("process is currently in queue: %s ", queueName);

    if (toPrint->state == 0 || toPrint->state == 1)
    {
        printf("%d\n",toPrint->priority);
    }
    else
    {
        printf("\n");
    }
    return 0;
}

void test_current_running()
{
    printf("PID: %d\n",current_running->PID);
    printf("state: %d\n",current_running->state);
    printf("priority: %d\n",current_running->priority);
}

void print_everything_inQueue()
{
    printf("======================\n");
    printf("printing all queues and their contents.\n");
    printf("\n");
    for (int i = 0; i < 3; i++)
    {
        if (List_count(arrayOfQueues[i]) == 0)
            continue;
        printf("Priority Queue %d:\n",i);
        PCB* current = List_first(arrayOfQueues[i]);
        while (current != NULL )
        {
            printf("%d ",current->PID);
            current = List_next(arrayOfQueues[i]);
        }
        printf("\n");
    }
    printf("Blocked on receive:\n");
        PCB* current = List_first(blockedOnReceive);
        while (current != NULL )
        {
            printf("%d ",current->PID);
            current = List_next(blockedOnReceive);
        }
    printf("\n");
    printf("Blocked on send:\n");
        current = List_first(blockedOnSend);
        while (current != NULL )
        {
            printf("%d ",current->PID);
            current = List_next(blockedOnSend);
        }
    printf("\n");
    printf("Blocked on semaphores:\n");
        Sem* current_sem = List_first(semaphores);
        while (current_sem != NULL)
        {
            List* waitlist = current_sem->waiting_processes;
            PCB* current_waiting_proc = List_first(waitlist);
            printf("Processes blocked on semaphore %d:\n", current_sem->ID);
            while (current_waiting_proc != NULL )
            {
                printf("%d ",current_waiting_proc->PID);
                current_waiting_proc = List_next(waitlist);
            }
            current_sem = List_next(semaphores);
            printf("\n");
        }
    printf("\n");
    printf("======================\n");
}