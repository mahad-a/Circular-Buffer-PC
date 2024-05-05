#include "assignment2.h"

/* semaphore_p changes the semaphore by -1 (waiting). */

static int semaphore_p(int sem_id)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = 0;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_p failed\n");
        return(0);
    }
    return(1);
}

/* semaphore_v is similar except for setting the sem_op part of the sembuf structure to 1,
 so that the semaphore becomes available. */

static int semaphore_v(int sem_id)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = 0;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed\n");
        return(0);
    }
    return(1);
}

int main() {
	FILE *filename;
	int msgid_consumer;
	void *shared_memory = (void *)0;
	struct shared_use_st *shared_stuff;
	int shmid_consumer;
	char req_msg[MAX_TEXT];// = "Request for the semaphore and shared memory keys.\n";
	char response[MAX_TEXT];
	char convert_size[MAX_TEXT];
	struct my_msg_st some_data;
	size_t size;
	int running = 1;
	int semS_id, semE_id, semN_id;
	int key,key1,key2,key3;
	int current_buffer_index = 0;
	
	msgid_consumer = msgget((key_t)SINGLE_MQUEUE, 0666);
	
	if (msgid_consumer == -1) {
		fprintf(stderr, "msgget failed\n");
		exit(EXIT_FAILURE);
	}
	
	printf("Starting up consumer.\n");
	
	some_data.my_msg_type = 4;
	
	sprintf(req_msg, "REQUEST;semS,semE,semN");
	printf("Consumer: Sending request: %s\n", req_msg);
	strcpy(some_data.some_text, req_msg);

	
	if (msgsnd(msgid_consumer, (void *)&some_data, MAX_TEXT, 0) == -1)
	{
	    fprintf(stderr, "msgsnd failed\n");
	    exit(EXIT_FAILURE);
	}
	
	
	if (msgrcv(msgid_consumer, &some_data, MAX_TEXT, 2, 0) == -1) {
		fprintf(stderr, "msgrcv failed\n");
		exit(EXIT_FAILURE);
	}
	
	
	sscanf(some_data.some_text, "RESPONSE;%[^;];%d;%d;%d", response, &key1, &key2, &key3);

	printf("Consumer: Received: semS with key: %d\n", key1);
	printf("Consumer: Received: semE with key: %d\n", key2);
	printf("Consumer: Received: semN with key: %d\n", key3); 
	
	semS_id = semget((key_t)key1, 1, 0666);
	if (semS_id == -1) {
		fprintf(stderr, "semget (semS) failed\n");
		exit(EXIT_FAILURE);
	}
	printf("semS is made\n");
	
	semE_id = semget((key_t)key2, 1, 0666);
	if (semE_id == -1) {
		fprintf(stderr, "semget (semE) failed\n");
		exit(EXIT_FAILURE);
	}
	printf("semE is made\n");
	
	semN_id = semget((key_t)key3, 1, 0666);
	if (semE_id == -1) {
		fprintf(stderr, "semget (semN) failed\n");
		exit(EXIT_FAILURE);
	}
	printf("semN is made\n");
	
	if (msgrcv(msgid_consumer, &some_data, MAX_TEXT, 7, 0) == -1) {
		fprintf(stderr, "msgrcv failed\n");
		exit(EXIT_FAILURE);
	}	

	
	sscanf(some_data.some_text, "RESPONSE;%[^;];%d", response, &key);
	printf("Consumer: Received: %s with key: %d \n", response, key);
	
	printf(" \n");
	
	if (msgrcv(msgid_consumer, &some_data, MAX_TEXT, 9, 0) == -1) {
		fprintf(stderr, "msgrcv failed\n");
		exit(EXIT_FAILURE);
	}
	
	printf("Consumer: Received file size from Producer: %s \n", some_data.some_text);
	strcpy(convert_size, some_data.some_text);
	int input_file_size = atoi(convert_size);

	shmid_consumer = shmget((key_t)key, sizeof(struct shared_use_st), 0666);

	if (shmid_consumer == -1) {
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	} 

	// shared memory now accessible
	shared_memory = shmat(shmid_consumer, (void *)0, 0);
	if (shared_memory == (void *)-1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}

	shared_stuff = (struct shared_use_st *)shared_memory;

	printf("Memory attached at %X\n", (int)shared_memory);
	
	filename = fopen("output_file.txt", "w");
	if (filename == NULL) {
		fprintf(stderr, "file open failed\n");
		exit(EXIT_FAILURE);
	}
	
	while (size != input_file_size) {
	
		// WAIT (N)
		semaphore_p(semN_id);
		
		// WAIT (S)
		semaphore_p(semS_id);
		
		
		// CONSUME (W)
		// opening output file and setting to write	
		
		fseek(filename, 0, SEEK_END);
		size = ftell(filename);

		
		if (shared_stuff->member_count[current_buffer_index] != 0) {
			printf("Buffer accessing: %d\n Member count: %d\n", current_buffer_index, shared_stuff->member_count[current_buffer_index]);
			printf("Data: %s\n Sequence Number: %d\n", shared_stuff->some_text[current_buffer_index], shared_stuff->sequence_number[current_buffer_index]); 
			if (shared_stuff->sequence_number[current_buffer_index] != size) {
				fprintf(stderr, "invalid sequence number, sequence: %d does not equal size: %ld \n", shared_stuff->sequence_number[current_buffer_index], size);
				exit(EXIT_FAILURE);
			}
			
			fputs(shared_stuff->some_text[current_buffer_index], filename);
			
			(shared_stuff->member_count)[current_buffer_index] = 0;
			(shared_stuff->sequence_number)[current_buffer_index] = 0;
			strcpy((shared_stuff->some_text)[current_buffer_index], "");
			
			current_buffer_index = (current_buffer_index + 1) % NUM_BUFFER;
		}
		
		
		
		// SIGNAL (S)
		semaphore_v(semS_id);
		
		// SIGNAL (E)
		semaphore_v(semE_id);
		
	}
	
	
	printf("Final Output File Size: %ld\n", size);
	
	printf("Consumer terminating.\n");
	// msgctl(msgid_consumer, IPC_RMID, 0);
	if (shmctl(shmid_consumer, IPC_RMID, 0) == -1) {
		fprintf(stderr, "shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
