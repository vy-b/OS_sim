#include "pcb.h"
#include <string.h>
int main()
{
    initialize_queues();
    int init = start_simulation();
    PCB* current_running = NULL;
    while(1)
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
            kill_PCB(pid);
        }
        else if (strcmp("E",userCommand) == 0 || strcmp("e",userCommand) == 0)
        {
            PCB_exit();
        }
    }
}