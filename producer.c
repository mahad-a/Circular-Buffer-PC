#include "assignment2.h"

/* The function set_semvalue initializes the semaphore using the SETVAL command in a
 semctl call. We need to do this before we can use the semaphore. */

static int set_semvalue(int sem_id, int value)
{
    union semun sem_union;

    sem_union.val = value;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);
    return(1);
}

/* The del_semvalue function has almost the same form, except the call to semctl uses
 the command IPC_RMID to remove the semaphore's ID. */

static void del_semvalue(int sem_id)
{
    union semun sem_union;
    
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore\n");
}

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
	int msgid_producer, semS_id, semE_id, semN_id;
	int running = 1;
	struct my_msg_st some_data;
	struct stat buf;
	size_t size;
	void *shared_memory = (void *)0;
	struct shared_use_st *shared_stuff;
	int shmid_producer;
	int key,key1,key2,key3;
	char req_msg[MAX_TEXT];
	char response[MAX_TEXT], response1[MAX_TEXT], response2[MAX_TEXT], response3[MAX_TEXT];
	char buffer[BUFFER_SIZE];
	int seq = 0;
	int current_buffer_index = 0;
	int buffer_count = 0; // cannot be more than 10
	
	// join shared message queue
	msgid_producer = msgget((key_t)SINGLE_MQUEUE, 0666);
	
	if (msgid_producer == -1) {
		fprintf(stderr, "msgget failed\n");
		exit(EXIT_FAILURE);
	}
	
	printf("Starting up producer.\n");

	some_data.my_msg_type = 3;
	
	sprintf(req_msg, "REQUEST;semS,semE,semN");
	printf("Producer: Sending request: %s\n", req_msg);
	strcpy(some_data.some_text, req_msg);

	
	if (msgsnd(msgid_producer, (void *)&some_data, MAX_TEXT, 0) == -1)
	{
	    fprintf(stderr, "msgsnd failed\n");
	    exit(EXIT_FAILURE);
	}
	
	if (msgrcv(msgid_producer, &some_data, MAX_TEXT, 2, 0) == -1) {
		fprintf(stderr, "msgrcv failed\n");
		exit(EXIT_FAILURE);
	}
	

	sscanf(some_data.some_text, "RESPONSE;%[^;];%d;%d;%d", response, &key1, &key2, &key3);

	printf("Producer: Received: semS with key: %d\n", key1);
	printf("Producer: Received: semE with key: %d\n", key2);
	printf("Producer: Received: semN with key: %d\n", key3); 
	
	semS_id = semget((key_t)key1, 1, 0666);
	if (semS_id == -1) {
		fprintf(stderr, "semget (semS) failed\n");
		exit(EXIT_FAILURE);
	}
	printf("semS is made\n");
	set_semvalue(semS_id, 1);
	
	semE_id = semget((key_t)key2, 1, 0666);
	if (semE_id == -1) {
		fprintf(stderr, "semget (semE) failed\n");
		exit(EXIT_FAILURE);
	}
	printf("semE is made\n");
	set_semvalue(semE_id, NUM_BUFFER);
	
	semN_id = semget((key_t)key3, 1, 0666);
	if (semE_id == -1) {
		fprintf(stderr, "semget (semN) failed\n");
		exit(EXIT_FAILURE);
	}
	printf("semN is made\n");
	set_semvalue(semN_id, 0);
	
	if (msgrcv(msgid_producer, &some_data, MAX_TEXT, 6, 0) == -1) {
		fprintf(stderr, "msgrcv failed\n");
		exit(EXIT_FAILURE);
	}
	
	sscanf(some_data.some_text, "RESPONSE;%[^;];%d", response, &key);
	printf("Producer: Received: %s with key: %d \n", response, key);
	
	
	printf(" \n");

	printf("Now beginning connection to shared memory\n");
	
	shmid_producer = shmget(key, sizeof(struct shared_use_st), 0666 | IPC_CREAT); // create shared memory
	
	if (shmid_producer == -1) {
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}
	
	// shared memory now accessible
	shared_memory = shmat(shmid_producer, (void *)0, 0);
	if (shared_memory == (void *)-1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}
	
	shared_stuff = (struct shared_use_st *)shared_memory;

	printf("Memory attached at %X\n", (int)shared_memory); 
	
	
	// opening input file and setting to read	
	filename = fopen("input_file.txt", "r");
	if (filename == NULL) {
		fprintf(stderr, "file open failed\n");
		exit(EXIT_FAILURE);
	}
	
	// checking file size and sending number to client
	fseek(filename, 0, SEEK_END);
	size = ftell(filename);
	fseek(filename, 0, SEEK_SET);
	
	some_data.my_msg_type = 5;
	char pass_size[MAX_TEXT];
	
	// passing the file size through message queues
	sprintf(pass_size, "%zu", size); 
	strcpy(some_data.some_text, pass_size);
	printf("Producer: Sending file size to Client: %s\n", pass_size);
	
	if (msgsnd(msgid_producer, (void *)&some_data, MAX_TEXT, 0) == -1)
	{
		fprintf(stderr, "msgsnd failed\n");
		exit(EXIT_FAILURE);
	}
	
	some_data.my_msg_type = 9;
	if (msgsnd(msgid_producer, (void *)&some_data, MAX_TEXT, 0) == -1)
	{
		fprintf(stderr, "msgsnd failed\n");
		exit(EXIT_FAILURE);
	}
	
	for (int i =0; i<NUM_BUFFER; i++) {
		(shared_stuff->member_count)[i] = 0;
		(shared_stuff->sequence_number)[i] = 0;
		strcpy((shared_stuff->some_text)[i], "");
	} 
	
	// now that file size has been passed to client, now we will start reading file and outputing into shared memory
	
	// PRODUCE V;
	
	while (!feof(filename)) {
		
		// WAIT (E)
		semaphore_p(semE_id);
		// WAIT (S)
		semaphore_p(semS_id);


		// APPEND V
		
		// check if buffer available

		if (shared_stuff->member_count[current_buffer_index] == 0) { 
			printf("We are now using Buffer: %d\n", buffer_count);
			// Copy the new text into the buffer
			printf("Copying text into buffer.\n");
			fgets(buffer, BUFFER_SIZE, filename);
			strcpy(shared_stuff->some_text[current_buffer_index], buffer);
			

			// Update sequence number and member count after copying the text
			shared_stuff->sequence_number[current_buffer_index] = seq;
			shared_stuff->member_count[current_buffer_index] = strlen(shared_stuff->some_text[current_buffer_index]);
			printf("Member count: %d\n",  shared_stuff->member_count[current_buffer_index]);
			printf("Ideal count: %ld\n",  strlen(shared_stuff->some_text[current_buffer_index]));
			printf("Message: %s\n", shared_stuff->some_text[current_buffer_index]);
			
			// recalculate offset / next sequence number
			seq += shared_stuff->member_count[current_buffer_index];
			printf("Sequence Number currently: %d\n", shared_stuff->sequence_number[current_buffer_index]);
			
			// increment buffer index (mod 10)
			current_buffer_index = (current_buffer_index + 1) % NUM_BUFFER;
			buffer_count++;
			if (buffer_count == 10) {
				buffer_count = 0;
			}
		}
		
		// SIGNAL (S)
		semaphore_v(semS_id);

		// SIGNAL (N)
		semaphore_v(semN_id);
	}
	
	
	
	
	
	
	printf("Producer terminating.\n");

	exit(EXIT_SUCCESS);
	
}
