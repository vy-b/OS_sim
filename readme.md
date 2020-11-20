# Interactive Operating System simulation
## To run:
**./OS_sim**
## some notes on intentional implementation choices/assumptions:
- To exit, all processes must be killed/exited. Easiest way to do so would be to use T to see all the existing processes, and use K on each of them. Init is PID 0 if you wish to kill it, else just exit after killing all other processes.

- A currently running process is NOT removed from its priority queue, but rather stay in the first position. When it gets quantum-ed, it will move to the back of the queue.

- I implemented 3 separate blocked queues for each type of blocked - blocked on send, on receive or on semaphore.

- When a process is blocked, it is removed from its queue and moved to the correct blocked queue (send, receive or semaphore)

- When it is unblocked, the process is placed at the back of its original priority queue.

- Reply (Y) does not handle multiple senders, but only the most recent one.

- Send (S) cannot send multiple times. It is blocked immediately after sending.

- Send(S) CAN send to a process that is blocked on send or on semaphore. The message will be waiting.

- Receive (R) only checks for the most recent message.

- A process of priority 0, when quantum-ed, will give CPU to the next process in the same queue, OR a process of lower priorty if there are no other priority 0 processes. If it is the only process running, then it continues running when the time quantum runs out.

### EXTRA:
**U**: display procinfo of the currently running process

## commands:
**C**: create a process and the ready queue with the appropriate priority. Three ready queues are available: 0, 1, 2.

 PIDs are generated based on priorities. The modulo (PID % 3) with priority 0 = 0, priority 1 = 1 and priority 2 = 2. This way it is easier to know which queue a process belongs to.
____________________________________________
 **F**: Fork the currently running process with a new ID. Attempting to fork init should fail. (Program does not recognize init as a process - user does not know of init).

______________________________________________
 **K**: Kill the named process and remove it from the system. If the named process is the currently running process, choose the next process to run and display scheduling info.

______________________________________________
 **E**: Kill the currently running process. Display scheduling info.

______________________________________________
 **Q**: Signals the end of a time quantum. Display scheduling info.

______________________________________________
 **S**: Send a message to another process from the currently running process. If the target process is currently receiving, display the message and sender ID on behalf of the receiver.
 
 Param: PID of the target process, null-terminated message. 
 
 Block the sender until reply. Display scheduling information.

______________________________________________
 **R**: Receive a message. If there is a waiting message, receive it and display message text and sender ID. If there is none, block.

 If block, display scheduling information.

______________________________________________
 **Y**: Unblocks sender and deliver a reply. Replying to a sender that hasn't sent anything should fail. Only handles the *most recent* sender and message. All others are disregarded.

______________________________________________
**N**: creates new semaphore with an ID (0-4) and initial value. Display error if sem ID already exists

______________________________________________
**P**: execute semaphore P on behalf of the running process - decrements the value of the semaphore. If the value is <= 0, block process and put it on the waiting list of this semaphore.
______________________________________________
**V**: executes semaphore V. Increments value of semaphore, if it becomes positive, remove a process from the waiting list **in FIFO order.**

______________________________________________
**I**: displays PID, state, priority and place in priority queue of the process whose ID is provided in the parameter. If blocked, display if blocked on send, receive or semaphore.

______________________________________________
**T**: display priority queues, blocked on send, blocked on receive and blocked on semaphore queues.

_______________________________________________
