# SYSC 4001 Assignment 2 README

# Mahad Ahmed 101220427

The assignment's objective is to apply message queue and semaphore/shared memory to the finite circular buffer for the producer/consumer problem, also know as P/C Problem.


# Usage
Compiling - You will be required to create four separate terminals, one for the Server, one for Client, one for the Consumer, and one for the Producer. As well, you will require an input file with text stored within it and an output file that is empty (program will create output file for you if you do not already have)

1. Launch terminal and navigate (use cd) to where the programs are located
2. **YOU MUST FOLLOW THE COMPILE ORDER** 
3. Compile the Server on its own terminal by inputting:
gcc server_msg.c -o server
4. Compile the Client on its own terminal by inputting:
gcc client_msg.c -o client
5. Compile the Producer on its own terminal by inputting:
gcc producer.c -o producer
6. Compile the Consumer on its own terminal by inputting:
gcc consumer.c -o consumer
7. Execute the Server first in its own terminal by:
./server
8. Once Server has been established, execute the Client in its own terminal by:
./client
9. Once Client has been established, execute the Producer in its own terminal by:
./producer
10. Once Producer has been established, execute the Consumer in its own terminal by:
./consumer
11. Now that all are running, you will see Client, Producer and Consumer sending requests to the Server. Server replies to their requests by generating random keys and creating the desired semaphores and shared memory (depending on their request). After semaphores and shared memory is created, the keys needed to access the semaphores/shared memory will be sent to the processes.
12. Producer gets key to access the shared memory, reads the input file, and writes to the shared memory
13. **DISCLAIMER** Shared memory contains 10 buffers, each buffer has space capacity of 1024 bytes. If your input file surpasses the amount of buffers stored, it will placed on wait, then after consumer is ran and cleans out buffers, the remaining data from input file will continue to flow into buffers
14. Consumer gets key to access the shared memory, reads the buffers in shared memory and writes to an output file until the output file size is the same as input file size.
