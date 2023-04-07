#include "misc.h"


int get_file_lines(FILE* in_file){
	
    int num_of_lines=0;
	char ch=0;
	
	if(in_file == NULL){
		printf("File has not been opened\n");
		return num_of_lines;
	}
	
	num_of_lines++;

	while(!feof(in_file)){
		
		ch =fgetc(in_file);
		
		if(ch == '\n'){
			num_of_lines++;
		}
	}

	return num_of_lines;
	
}

void free_resources(int shmid, int semid){
    
    //Καταστροφη της μνημης
	if(shmctl(shmid,IPC_RMID,NULL) < 0){
		perror("Memory Destruction");
		exit(EXIT_FAILURE);
	}

    //Καταστροφη του σημαφορου
    if (semctl(semid, 0, IPC_RMID, 0) < 0 ) { 
		perror("Semaphore Destruction");
		exit(EXIT_FAILURE);
	}
}

int sem_Init(int sem_id, int val) {
    union semun arg;

    arg.val = val;
    if (semctl(sem_id, 0, SETVAL, arg) == -1) {
        perror(" Semaphore setting value ");
        return -1;
    }
    return 0;
}

int sem_V(int sem_id) {
    struct sembuf sem_d;

    sem_d.sem_num = 0;
    sem_d.sem_op = 1;
    sem_d.sem_flg = 0;
    if (semop(sem_id, &sem_d, 1) == -1) {
        perror(" Semaphore up (sem_V) operation ");
        return -1;
    }
    return 0;
}

int sem_P(int sem_id) {
    struct sembuf sem_d;

    sem_d.sem_num = 0;
    sem_d.sem_op = -1;
    sem_d.sem_flg = 0;
    if (semop(sem_id, &sem_d, 1) == -1) {
        perror(" Semaphore down (sem_P) operation ");
        return -1;
    }
    return 0;
}