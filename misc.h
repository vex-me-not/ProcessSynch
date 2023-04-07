#ifndef _MISC_H_
#define _MISC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

#define LINE_SIZE 101     //101 γιατι ειναι και το \0

//Θα χρησιμοποιησουμε SYS V για αυτο πρεπει να εχουμε δηωσει το απο κατω enum 
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

typedef struct {
    char line[LINE_SIZE];     // τα περιεχομενα της γραμμης που πρεπει να φερει ο γονεας
    int get_line;   // ποια γραμμη πρεπει να φερει ο γονεας
}memory;            // Η δομη της διαμοιραζομενης μνημης


void free_resources(int shmid, int semid); // συναρτηση που απελευθερωνει τους δεσμευμενους πορους(απο το φροντιστηριο)
int get_file_lines(FILE* in_file);         // συναρτηση που επιστρεφει το πληθος των γραμμων ενος αρχειου 

int sem_Init(int sem_id, int val);        //Semaphore Init - set a semaphore's value to val(απο το φροντιστηριο)
int sem_V(int sem_id);                    //Semaphore V - up operation, using semop(απο το φροντιστηριο)
int sem_P(int sem_id);                    //Semaphore P - down operation, using semop(απο το φροντιστηριο)

#endif