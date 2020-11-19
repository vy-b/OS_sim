#ifndef _PCB_H_
#define _PCB_H_
#include "list.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum state
{
    running,
    ready,
    blocked
}state_s;

typedef struct PCB_s PCB;
struct PCB_s
{
    int PID;
    int priority;
    state_s state;
    char* msg_received;
    PCB* sender;
    
};
typedef struct semaphore_s Sem;
struct semaphore_s
{
    int ID;
    int val;
    List* waiting_processes;
};
// generate new pid based on its priority
int generate_newPID(int priority_input);

// find a process given its ID and containing queue.
// returns PCB of the process with List_curr pointer at the position of that process in its queue
PCB* find_by_pid(int pid_input, List* searchingQueue);

// chooses the next process to execute based on priority
int next_process();

// creates 3 ready queues, one for each priority
void initialize_queues();

// initializes the "init" process, which will always be available to run
int start_simulation();

// Initializes a process and places it on the ready queue
int create_PCB(int priority_input);

// fork the currently running process with the same priority
// place it on the appropriate ready queue
int fork_PCB();

// kills the process with the given PID
int kill_PCB(int pid_input);

// kills the currently running process
int PCB_exit();

// time quantum runs out. execute next process
// returns scheduling info
int PCB_quantum();

// send a message to another process - block until reply
// return success or failure, scheduling info
int sendto_PCB(int sendto_pid, char* msg);

// receives a message - block until one arrives
// checks if there is a waiting message to receive
// return success or failure, scheduling info
// message text and source of message
int recvfrom_PCB();

// unblocks sender and delivers reply.
// return success or failure
int reply_PCB(int replyto_pid, char* msg);

// find a process in the pool of semaphores.
// after execution, current pointer of the pool is at the found process
Sem* find_sem(int sem_ID);

// creates new semaphore
// uses find_sem to check if sem_ID already exists
// fails if sem_ID already exists or out of range
// returns 0 on success, -1 on failure
int new_sem(int sem_ID, int initial_value);

// executes P on behalf of the current process
int semaphore_P(int sem_ID);

// executes V on behalf of the current process
int semaphore_V(int sem_ID);


/*display priority queues, blocked on send, 
blocked on receive and blocked on semaphore queues.*/
void print_everything_inQueue();

/*displays PID, state, priority and place in priority queue of the process whose ID is provided in the parameter. If blocked, 
display if blocked on send, receive or semaphore.*/
void test_prints(int pid_input);

 /*display procinfo of the currently running process*/
void test_current_running();

#endif