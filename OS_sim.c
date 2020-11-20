#include "pcb.h"
#include <string.h>
int main()
{
    initialize_queues();
    int init = start_simulation();
    if (init == -1)
    {
        printf("error starting simulation\n");
        return 0;
    }
    char* send = malloc(sizeof(char)*41);
    char* reply = malloc(sizeof(char)*41);
    while(init == 0)
    {
        printf("enter command: ");
        char userCommand[2];
        scanf("%s",userCommand);
        if (strcmp("C",userCommand) == 0 || strcmp("c",userCommand) == 0)
        {
            printf("enter priority: ");
            int priority;
            scanf("%d",&priority);
            if (priority < 0 || priority >2)
            {
                printf("Priority must be 0,1 or 2\n ");
                continue;
            }
            int newPCB = create_PCB(priority);
            if (newPCB == -1)
            {
                printf("New process creation failed");
            }
            else
                printf("Create success. Process ID = %d\n", newPCB);
            printf("\n");
        }

        else if (strcmp("F",userCommand) == 0 || strcmp("f",userCommand) == 0)
        {
            int forked = fork_PCB();
            if (forked == -1)
            {
                printf("New process fork failed\n");
            }
            else
                printf("Process forked successfully. Process ID = %d\n",forked);
            printf("\n");
        }

        else if (strcmp("K",userCommand) == 0 || strcmp("k",userCommand) == 0)
        {
            printf("enter PID of the process to kill: ");
            int pid;
            scanf("%d",&pid);
            int ret = kill_PCB(pid);
            if (ret == 1) break;
            printf("\n");
        }
        else if (strcmp("E",userCommand) == 0 || strcmp("e",userCommand) == 0)
        {
            int ret = PCB_exit();
            if (ret == 1) break;
            printf("\n");
        }
        else if (strcmp("Q",userCommand) == 0 || strcmp("q",userCommand) == 0)
        {
            PCB_quantum();
            printf("Quantum executed successfully.\n");
            printf("\n");
        }

        else if (strcmp("S",userCommand) == 0 || strcmp("s",userCommand) == 0)
        {
            printf("Which process would you like to send to? Enter PID: ");
            int pid_input;
            scanf("%d",&pid_input);
            printf("what is your message? ");
            scanf("%s",send);
            printf("\n");
            sendto_PCB(pid_input, send);
            printf("\n");
        }
        else if (strcmp("R",userCommand) == 0 || strcmp("r",userCommand) == 0)
        {
            recvfrom_PCB();
            printf("\n");
        }
        else if (strcmp("Y",userCommand) == 0 || strcmp("y",userCommand) == 0)
        {
            printf("Which process would you like to reply to? ");
            int pid_input;
            scanf("%d", &pid_input);
            printf("Enter your reply message: ");
            scanf("%s",reply);
            reply_PCB(pid_input,reply);
            printf("\n");
        }
        else if (strcmp("I",userCommand) == 0 || strcmp("i",userCommand) == 0)
        {
            printf("enter pid of process to check info: ");
            int input;
            scanf("%d",&input);
            test_prints(input);
            printf("\n");
        }
        else if (strcmp("N",userCommand) == 0 || strcmp("n",userCommand) == 0)
        {
            printf("enter semaphore ID (0-4): ");
            int sem_ID;
            scanf("%d",&sem_ID);
            printf("enter initial value for semaphore %d: ",sem_ID);
            int initial_value;
            scanf("%d",&initial_value);
            if (new_sem(sem_ID, initial_value) == 0)
                printf("Create new semaphore success.\n");
            printf("\n");
        }
        else if (strcmp("P",userCommand) == 0 || strcmp("p",userCommand) == 0)
        {
            printf("enter semaphore ID to execute: ");
            int sem_id_input;
            scanf("%d",&sem_id_input);
            semaphore_P(sem_id_input);
            printf("\n");
        }
        else if (strcmp("V",userCommand) == 0 || strcmp("v",userCommand) == 0)
        {
            printf("enter semaphore ID to execute: ");
            int sem_id_input;
            scanf("%d",&sem_id_input);
            semaphore_V(sem_id_input);
            printf("\n");
        }
        else if (strcmp("U",userCommand) == 0 || strcmp("u",userCommand) == 0)
        {
            printf("The currently running process is as following:\n");
            test_current_running();
            printf("\n");
        }
        else if (strcmp("T",userCommand) == 0 || strcmp("t",userCommand) == 0)
        {
            print_everything_inQueue();
            printf("\n");
        }
    }
    free(send);
    free(reply);
    return 0;
}