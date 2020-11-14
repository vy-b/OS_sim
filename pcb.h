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
    // place to store message
};

#define MAX_PID 2046;
// creates 3 ready queues, one for each priority
void initialize_queues();

// initializes the "init" process, which will always be available to run
int start_simulation();

// Initializes a process and places it on the ready queue
int create_PCB(int priority_input);

int fork_PCB();

int kill_PCB(int pid_input);

int PCB_exit();

int generate_newPID(int priority_input);
#endif