#include "assignment2.h"


int main() {
	int msgid_client, semS_id;
	struct my_msg_st some_data;
	char req_msg[MAX_TEXT];// = "Request for the semaphore and shared memory keys.\n";
	char response[MAX_TEXT];
	
	int key;
	
	msgid_client = msgget((key_t)SINGLE_MQUEUE, 0666);
	
	if (msgid_client == -1) {
		fprintf(stderr, "msgget failed\n");
		exit(EXIT_FAILURE);
	}
	
	
	
	printf("Starting up client.\n");

	some_data.my_msg_type = 1; 
	
	
	sprintf(req_msg, "REQUEST;semX");
	printf("Client: Sending request: %s\n", req_msg);
	strcpy(some_data.some_text, req_msg);

	
	if (msgsnd(msgid_client, (void *)&some_data, MAX_TEXT, 0) == -1)
	{
	    fprintf(stderr, "msgsnd failed\n");
	    exit(EXIT_FAILURE);
	}

	
	
	if (msgrcv(msgid_client, &some_data, MAX_TEXT, 2, 0) == -1) {
		fprintf(stderr, "msgrcv failed\n");
		exit(EXIT_FAILURE);
	}
	
	
	sscanf(some_data.some_text, "RESPONSE;%[^;];%d", response, &key);
	printf("Client: Received: %s with key: %d\n", response, key); 
	
	semS_id = semget((key_t)key, 1, 0666 | IPC_CREAT);
	
	if (msgrcv(msgid_client, &some_data, MAX_TEXT, 5, 0) == -1) {
		fprintf(stderr, "msgrcv failed\n");
		exit(EXIT_FAILURE);
	}
	
	printf("Client: Received file size from Producer: %s \n", some_data.some_text);
	
	
	printf(" \n");
	

	printf("Client is terminating.\n");
	
	if (semctl(semS_id, 0, IPC_RMID) == -1) {
		fprintf(stderr, "semctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
