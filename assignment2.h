// header file for assignment 2
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <sys/stat.h>

#define SINGLE_MQUEUE 1234
#define MAX_TEXT 2048
#define NUM_BUFFER 10
#define BUFFER_SIZE 1024

struct my_msg_st
{
    long int my_msg_type; 
    char some_text[MAX_TEXT];
};

struct KeyValuePair {
    char key[50];
    int value;
};



#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
    /* union semun is defined by including <sys/sem.h> */
#else
    /* according to X/OPEN we have to define it ourselves */
    union semun {
        int val;                    /* value for SETVAL */
        struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
        unsigned short int *array;  /* array for GETALL, SETALL */
        struct seminfo *__buf;      /* buffer for IPC_INFO */
    };
#endif

struct shared_use_st {
    char some_text[NUM_BUFFER][BUFFER_SIZE]; // buffer
    int sequence_number[NUM_BUFFER];
    int member_count[NUM_BUFFER];
};


