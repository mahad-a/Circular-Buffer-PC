#include "assignment2.h"

static int sem_id;


int main() {
	int msgid_server, semS_id, semE_id, semN_id;

	int running = 1;
	struct my_msg_st some_data;
	char request[MAX_TEXT], request1[MAX_TEXT], request2[MAX_TEXT], request3[MAX_TEXT];
	char resp_msg[MAX_TEXT];
	char msg[MAX_TEXT];
	int shm_key, key, key1,key2,key3;

	struct KeyValuePair semaphores[] = {
		{"semS", -1},
		{"semE", -1},
		{"semN", -1}
	};
	
	msgid_server = msgget((key_t)SINGLE_MQUEUE, 0666 | IPC_CREAT); // creates the message queue
	if (msgid_server == -1) { // check if message queue was properly initailized and connected
		fprintf(stderr, "msgget failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	
	shm_key = rand() % 9000 + 1000; // generate random number between 1000 and 9000
	
	printf("Starting up server.\n");
	
	while (running) {
		// client requests, msg type 1
		if (msgrcv(msgid_server, &some_data, MAX_TEXT, 1, 0) == -1) {
			fprintf(stderr, "msgrcv failed\n");
			exit(EXIT_FAILURE);
		}
	
		some_data.my_msg_type = 2;

		sscanf(some_data.some_text, "REQUEST;%s", request);

		printf("Server: Receiving Client's request: %s\n", request);
		
		key = rand() % 9000 + 1000; // generate random number between 1000 and 9000
		
		sprintf(resp_msg, "RESPONSE;%s;%d", request, key);
		printf("Server: Sending response to Client: %s\n", resp_msg);
		strcpy(some_data.some_text, resp_msg);

		if (msgsnd(msgid_server, (void *)&some_data, MAX_TEXT, 0) == -1)
		{
		    fprintf(stderr, "msgsnd failed\n");
		    exit(EXIT_FAILURE);
		} 
		
		printf("\n");
		
		// producer request, msg type 3
		if (msgrcv(msgid_server, &some_data, MAX_TEXT, 3, 0) == -1) {
			fprintf(stderr, "msgrcv failed\n");
			exit(EXIT_FAILURE);
		}
		some_data.my_msg_type = 2;
	
		sscanf(some_data.some_text, "REQUEST;%s", request);

		printf("Server: Receiving Producer's request: %s\n", request);
		
		key1 = rand() % 9000 + 1000; // generate random number between 1000 and 9000
		key2 = rand() % 9000 + 1000; // generate random number between 1000 and 9000
		key3 = rand() % 9000 + 1000; // generate random number between 1000 and 9000
		
		// Create Semaphore S, wait for buffer to be free
		semS_id = semget((key_t)key1, 1, 0666 | IPC_CREAT);
		
		// Create Semaphore E, wait for buffer to be empty
		semE_id = semget((key_t)key2, 1, 0666 | IPC_CREAT);
		
		// Create Semaphore N, wait for buffer to have something in it
		semN_id = semget((key_t)key3, 1, 0666 | IPC_CREAT);
		

		sprintf(resp_msg, "RESPONSE;%s;%d;%d;%d", request,key1, key2, key3);
		printf("Server: Sending response to Producer: %s\n", resp_msg);
		strcpy(some_data.some_text, resp_msg);

		if (msgsnd(msgid_server, (void *)&some_data, MAX_TEXT, 0) == -1)
		{
		    fprintf(stderr, "msgsnd failed\n");
		    exit(EXIT_FAILURE);
		} 
		
		printf("\n");
		
		// create shared memory and pass shared memory key to producer
		some_data.my_msg_type = 6;
		printf("Sending key for shared memory to Producer\n");
		
		sprintf(resp_msg, "RESPONSE;ShMem;%d", shm_key);
		printf("Server: Sending response to Producer: %s\n", resp_msg);
		strcpy(some_data.some_text, resp_msg);

		if (msgsnd(msgid_server, (void *)&some_data, MAX_TEXT, 0) == -1)
		{
		    fprintf(stderr, "msgsnd failed\n");
		    exit(EXIT_FAILURE);
		} 
		
		printf("\n");
		
		// consumer request, msg type 4
		if (msgrcv(msgid_server, &some_data, MAX_TEXT, 4, 0) == -1) {
			fprintf(stderr, "msgrcv failed\n");
			exit(EXIT_FAILURE);
		}
		some_data.my_msg_type = 2;
		
		sscanf(some_data.some_text, "REQUEST;%s", request);

		printf("Server: Receiving Consumer's request: %s\n", request);

		sprintf(resp_msg, "RESPONSE;%s;%d;%d;%d", request,key1, key2, key3);
		printf("Server: Sending response to Producer: %s\n", resp_msg);
		strcpy(some_data.some_text, resp_msg);

		if (msgsnd(msgid_server, (void *)&some_data, MAX_TEXT, 0) == -1)
		{
		    fprintf(stderr, "msgsnd failed\n");
		    exit(EXIT_FAILURE);
		} 
		
		printf("\n");
		
		// create shared memory and pass shared memory key to consumer 
		some_data.my_msg_type = 7;
		printf("Sending key for shared memory to Consumer\n");
		
		sprintf(resp_msg, "RESPONSE;ShMem;%d", shm_key);
		printf("Server: Sending response to Consumer: %s\n", resp_msg);
		strcpy(some_data.some_text, resp_msg);

		if (msgsnd(msgid_server, (void *)&some_data, MAX_TEXT, 0) == -1)
		{
		    fprintf(stderr, "msgsnd failed\n");
		    exit(EXIT_FAILURE);
		} 
		
		
	}


	
	printf("Server is terminating\n");
	/*if (shmdt(shared_memory) == -1) {
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
    	}*/
    	
    	msgctl(msgid_server, IPC_RMID, 0);

	exit(EXIT_SUCCESS);
	
}
