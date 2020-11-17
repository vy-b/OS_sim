#include "pcb.h"
#include <string.h>
int main()
{
    initialize_queues();
    int init = start_simulation();
    while(init == 0)
    {
        printf("enter command: ");
        char userCommand[1];
        scanf("%s",userCommand);
        if (strcmp("C",userCommand) == 0 || strcmp("c",userCommand) == 0)
        {
            printf("enter priority: ");
            int priority;
            scanf("%d",&priority);
            while (priority < 0 || priority >2)
            {
                printf("re-enter priority (0, 1 or 2): ");
                scanf("%d",&priority);
            }
            int newPCB = create_PCB(priority);
            if (newPCB == -1)
            {
                printf("New process creation failed");
            }
            else
                printf("Create success. Process ID = %d\n", newPCB);
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
        }

        else if (strcmp("K",userCommand) == 0 || strcmp("k",userCommand) == 0)
        {
            printf("enter PID of the process to kill: ");
            int pid;
            scanf("%d",&pid);
            int ret = kill_PCB(pid);
            if (ret == 1) break;
        }
        else if (strcmp("E",userCommand) == 0 || strcmp("e",userCommand) == 0)
        {
            int ret = PCB_exit();
            if (ret == 1) break;
        }
        else if (strcmp("Q",userCommand) == 0 || strcmp("q",userCommand) == 0)
        {
            printf("time quantum expired. now running process: %d\n",PCB_quantum());
        }
        else if (strcmp("T",userCommand) == 0 || strcmp("t",userCommand) == 0)
        {
            printf("enter pid of process to check: ");
            int input;
            scanf("%d",&input);
            test_prints(input);
        }
        else if (strcmp("R",userCommand) == 0 || strcmp("r",userCommand) == 0)
        {
            test_current_running();
        }
        else if (strcmp("V",userCommand) == 0 || strcmp("v",userCommand) == 0)
        {
            print_everything_inQueue();
        }
    }
}