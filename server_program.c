#include "misc.h"


int main(int argc, char* argv[]){

	// Πρεπει να εχουμε ακριβως 4 ορισματα στην κληση του server_program.Για καθε περίπτωση τυπωνουμε το αντιστοιχο μηνυμα
	if(argc < 4){
		printf("Not enough arguments! Try again with EXACTLY 3 arguments\n");
		exit(EXIT_FAILURE);
	}
	if (argc > 4){
		printf("Too many arguments! Try again with EXACTLY 3 arguments\n");
		exit(EXIT_FAILURE);
	}
	
	FILE * in_file = fopen(argv[1],"r");  // ανοιγουμε το αρχειο προς διαβασμα
	
	//Αν δεν μπορεσαμε να ανοιξουμε το αρχειο, τερματιζουμε
	if (in_file == NULL){
		printf("Can't open file\n");
		exit(EXIT_FAILURE);
	}	
    
	int k_children = atoi(argv[2]);  // ο αριθμος των παιδιων που θα δημιουργηθουν
	int n_tactions = atoi(argv[3]);  // ο αριθμος των δοσολειψιων για καθε παιδι
	int x_file_lines = get_file_lines(in_file);  // οι γραμμες του αρχειου Χ

	//Σε περιπτωη που δωθει αρνητικος αριθμός παιδιων
	if(k_children < 0 ){
		printf("Number of children MUST be NON-NEGATIVE(greater than or equal to ZERO)!Try again\n");
		exit(EXIT_FAILURE);
	}

	//Σε περιπτωη που δωθει αρνητικος ή μηδενικος(το client_program κανει διαιρεση με το n_tactions) αριθμος δοσοληψιων πρεπει να τερματισουμε
	if(n_tactions <= 0){
		printf("Number transactions MUST be POSITIVE(greater than ZERO)!Try again\n");
	exit(EXIT_FAILURE);
	}

	//Αν το αρχειο δεν εχει καμια γραμμη, πρεπει παλι να τερματισουμε
	if(x_file_lines <= 0){
		printf("File %s is empty! Please give a non-empty file!",argv[1]);
		exit(EXIT_FAILURE);
	}

	//Flavour text
	printf("Parent will create %i children\n",k_children);
	printf("Each child will participate in %i transactions\n",n_tactions);
	printf("File with name %s has %i lines\n",argv[1],x_file_lines);

	in_file = fopen(argv[1],"r");  // ξανανοιγουμε το αρχειο προς διαβασμα,γιατι το εκλεισε η get_file_lines
	
	//Αν δεν μπορεσαμε να το ξανανοιξουμε πρεπει να  τερματισουμε
	if (in_file == NULL){
		printf("Can't open file\n");
		exit(EXIT_FAILURE);
	}	
	
	memory* com_mem;  // δεικτης στη δομη κοινης μνημης που θελουμε 

    key_t key;  // κλειδι με το οποιο θα μπορεσουμε να "παρουμε" διαμοιραζομενη μνημη
    int shmid;  // το shmid της διαμοιραζομενης μνημης που θέλουμε να παρουμε
 	
	// δημιουργια του κλειδιου
	key = ftok("server_program.c", 'R'); 

	//Έλεγχος σφαλματος
	if(key == -1){
		perror("Key creation");
		exit(EXIT_FAILURE);
	}

    // Συνδεση και πιθανη δημιουργια της μνημης 
	shmid = shmget(key, sizeof(memory), 0660 | IPC_CREAT);
	if (shmid == -1) {
		perror("Memory creation");
		exit(EXIT_FAILURE);
	}
    
	// κανουμε attach τη διαμοιραζομενη μνημη

    com_mem =shmat(shmid, NULL, 0);
	
    if (com_mem == (memory *)-1) { //ελεγχος σφαλματος κατα το attachment
		perror("Memory attachment");
		exit(EXIT_FAILURE);
	}    
	
    printf("Shared memory segment with id %d attached at %p\n", shmid, com_mem);

	int server_sem;   // Ο σεμαφορος που καθοριζει την επικοινωνια του γονεα(ποτε διαβαζει,ποτε απανταει) με τη διαμοιραζομενη μνημη
	int client_sem;  // Ο σεμαφορος που καθοριζει την επιικοινωνια ενος παιδιου(ποτε αιτειται,ποτε διαβαζει) με τη διαμοιραζομενη μνημη
	int inter_client_sem;  // Ο σεμαφορος που καθοριζει ποιο απο τα Κ παιδια θα αλληλεπιδρασει με την μνημη

	//Δημιουργια του πρωτου σημαφορου
	server_sem = semget(key+1, 1, IPC_CREAT | 0660);

	//Αν αποτυχει η δημιουργια του 1ου σεμαφορου πρεπει να αποδεσμευσουμε την διαμοιραζομενη μνημη και να τερματισουμε
    if (server_sem < 0) {
        perror("Semaphore creation");

			//Κανουμε detach την διαμοιραζομενη μνημη
    	if (shmdt(com_mem) == -1) {
			perror("Memory detachment");
			exit(EXIT_FAILURE);
		}
		
		if(shmctl(shmid,IPC_RMID,NULL) < 0){
			perror("Memory Destruction");
			exit(EXIT_FAILURE);
		}
        
		exit(EXIT_FAILURE);
    }
	
	//Αν αποτυχει η αρχικοποιηση του 1ου σεμαφορου πρεπει να αποδεσμευσουμε την διαμοιραζομενη μνημη και τον 1ο σεμαφορο και να τερματισουμε    
	if (sem_Init(server_sem, 0) < 0) {
		perror("Semaphore Initiation");

			//Κανουμε detach την διαμοιραζομενη μνημη
    	if (shmdt(com_mem) == -1) {
			perror("Memory detachment");
			exit(EXIT_FAILURE);
		}

		if(shmctl(shmid,IPC_RMID,NULL) < 0){
			perror("Memory Destruction");
			exit(EXIT_FAILURE);
		}

		    //Καταστροφη του 1ου σημαφορου
    	if (semctl(server_sem, 0, IPC_RMID, 0) < 0 ) { 
			perror("Semaphore Destruction");
			exit(EXIT_FAILURE);
		}
        
		exit(EXIT_FAILURE);
    }

	//Δημιουργια του 2ου σημαφορου
    client_sem = semget(key+2, 1, IPC_CREAT | 0660);
    
	//Αν αποτυχει η δημιουργια του 2ου σεμαφορου πρεπει να αποδεσμευσουμε την διαμοιραζομενη μνημη και τον 1ο σεμαφορο και να τερματισουμε	
	if (client_sem < 0) {
        perror("Semaphore creation");
		
		//Κανουμε detach την διαμοιραζομενη μνημη
    	if (shmdt(com_mem) == -1) {
			perror("Memory detachment");
			exit(EXIT_FAILURE);
		}
		
		free_resources(shmid,server_sem);

        exit(EXIT_FAILURE);
    }

	//Αν αποτυχει η αρχικοποιηση του 2ου σεμαφορου πρεπει να αποδεσμευσουμε την διαμοιραζομενη μνημη,τον 1ο και τον 2ο σεμαφορο και να τερματισουμε	
    if (sem_Init(client_sem, 1) < 0) {
		
		perror("Semaphore Initiation");
		
		//Κανουμε detach την διαμοιραζομενη μνημη
    	if (shmdt(com_mem) == -1) {
			perror("Memory detachment");
			exit(EXIT_FAILURE);
		}
		
		//Καταστροφη του 2ου σημαφορου
    	if (semctl(client_sem, 0, IPC_RMID, 0) < 0 ) { 
			perror("Semaphore Destruction");
			exit(EXIT_FAILURE);
		}

		free_resources(shmid,server_sem);

        exit(EXIT_FAILURE);
    }
	
	//Δημιουργια του 3ου σημαφορου    
	inter_client_sem = semget(key+3, 1, IPC_CREAT | 0660);
    
	//Αν αποτυχει η δημιουργια του 3ου σεμαφορου πρεπει να αποδεσμευσουμε την διαμοιραζομενη μνημη,τον 1ο και τον 2o σεμαφορο και να τερματισουμε	
	if (inter_client_sem < 0) {
        perror("Semaphore creation ");
		
		//Κανουμε detach την διαμοιραζομενη μνημη
    	if (shmdt(com_mem) == -1) {
			perror("Memory detachment");
			exit(EXIT_FAILURE);
		}
		
		//Καταστροφη του 2ου σημαφορου
    	if (semctl(client_sem, 0, IPC_RMID, 0) < 0 ) { 
			perror("Semaphore Destruction");
			exit(EXIT_FAILURE);
		}

		free_resources(shmid,server_sem);
                
		exit(EXIT_FAILURE);
    }

	//Αν αποτυχει η αρχικοποιηση του 3ου σεμαφορου πρεπει να αποδεσμευσουμε την διαμοιραζομενη μνημη,τον 1ο ,τον 2o και τον 3ο σεμαφορο και να τερματισουμε
    if (sem_Init(inter_client_sem, 1) < 0) {
		perror("Semaphore Initiation ");
		
		//Κανουμε detach την διαμοιραζομενη μνημη
    	if (shmdt(com_mem) == -1) {
			perror("Memory detachment");
			exit(EXIT_FAILURE);
		}
		
		//Καταστροφη του 3ου σημαφορου
    	if (semctl(inter_client_sem, 0, IPC_RMID, 0) < 0 ) { 
			perror("Semaphore Destruction");
			exit(EXIT_FAILURE);
		}	
		//Καταστροφη του 2ου σημαφορου
    	if (semctl(client_sem, 0, IPC_RMID, 0) < 0 ) { 
			perror("Semaphore Destruction");
			exit(EXIT_FAILURE);
		}	
		free_resources(shmid,server_sem);
        exit(EXIT_FAILURE);
    }		

	// θα χρησιμοποιησουμε αυτες τις συμβολοσειρες για να γνωστοποιησουμε απαραιτητες πληροφοριες στα παιδια
    char s1[12];
	char s2[12];
	char s3[12];
	char s4[12];
	char s5[12];

	//Μετατρεπουμε τις πληροφοριες που ειναι int σε char* αφου μονο ετσι μπορουμε να τις περασουμε μεσω execv
	sprintf(s1, "%d", server_sem);  
	sprintf(s2, "%d", client_sem);  
	sprintf(s3, "%d", inter_client_sem); 
	sprintf(s4, "%d", shmid);  
	sprintf(s5, "%d", x_file_lines);  

	// το char* argv[] που απαιτει η execv
	char* arg[8];
	
	arg[0] = "client_program";  // το ονομα του αρχειου για το execv
	arg[1] = argv[3];    // γνωστοποιουμε τον αριθμο των δοσοληψιων στα παιδια
	arg[2] = s1;         // γνωστοποιουμε τον σημαφορο που διαχειριζεται τον server
	arg[3] = s2;         // γνωστοποιουμε τον σημαφορο που διαχειριζεται ποτε μπορει καποιο παιδι να κανει αιτημα/διαβασει απο τη μνημη
	arg[4] = s3;          // γνωστοποιουμε τον σημαφορο που διαχειριζεται ποιο παιδι θα κανει αιτημα/διαβασει απο τη μνημη
	arg[5] = s4;          // γνωστοποιουμε την κοινη μνημη
	arg[6] = s5;          // γνωστοποιουμε τον αριθμο των γραμμων του αρχειου Χ
	arg[7] = NULL;       // NULL γιατι ετσι πρεπει να τελειωνει ο πινακας για την execv



	
	//Δημιουργουμε τα Κ παιδια με την execv
	for(int k=0;k<k_children;k++){
		
		pid_t pid = fork();
		
		if(pid == 0){ // αν εχουμε παιδι καλουμε την execv να εκτλεσει το client_program με εισοδο το arg
			execv("./client_program",arg);
		}else if(pid < 0){ // Εχουμε error 
			perror("Fork error");			
			exit(EXIT_FAILURE);
		}
	}


	int* demand = &(com_mem->get_line);  // το αιτημα του παιδιου που διαβαζει ο γονεας
	char* answer = com_mem->line;        // Η απαντηση του γονεα που διαβαζει το παιδι
	
	char text[LINE_SIZE];                // το κειμενο της αιτουσας γραμμης,μεγεθους LINE_SIZE

	int total_tactions = k_children*n_tactions; // Το συνολικο πληθος δοσοληψιων στις οποιες πρεπει να απαντησει ο γονεας
	
	//Ο γονεας ικανοποιει ολα τα αιτηματα/δοσοληψιες
	for(int i =0;i<total_tactions;i++){
		
		// Διαβαζουμε το αιτημα
		if(sem_P(server_sem)<0){
			exit(EXIT_FAILURE);
		}

		//Εντοπιζουμε τη γραμμη του αιτηματος στο αρχειο
		for(int j = 0; j< *demand;j++){
			fgets(text,LINE_SIZE,in_file);
		}
		//Επιστρεφουμε την απάντηση μας , δηλαδη τη γραμμη που διαβασαμε πιο πανω
		strcpy(answer,text);

		
		in_file = fopen(argv[1],"r");  // ξανανοιγουμε το αρχειο προς διαβασμα,γιατι το εκλεισε η get_file_lines

		//Αφηνουμε τα παιδια να αλληλεπιδρασουν με τη μνημη
		if(sem_V(client_sem)<0){
			exit(EXIT_FAILURE);
		}
	}
	
	int status_id;
	int child;
	
	//Περιμενουμε ολα τα παιδια να ολοκληρωσουν
	for(int k=0;k<k_children;k++){
		child = wait(&status_id);
		printf("Child %i exited with status %i\n", child,status_id);
	}

	//Κανουμε detach την διαμοιραζομενη μνημη
    if (shmdt(com_mem) == -1) {
		perror("Memory detachment");
		exit(EXIT_FAILURE);
	}

 	//Καταστρεφουμε τον 3ο σεμαφορο   
    if (semctl(inter_client_sem, 0, IPC_RMID, 0) < 0 ) { 
		perror("Semaphore Destruction");
		exit(EXIT_FAILURE);
	}

	//Καταστρεφουμε τον 2ο σεμαφορο
    if (semctl(client_sem, 0, IPC_RMID, 0) < 0 ) { 
		perror("Semaphore Destruction");
		exit(EXIT_FAILURE);
	}

	//Καταστρεφουμε την μνημη και τον 1ο σημαφορο
	free_resources(shmid,server_sem);
	
	//Κλεινουμε το αρχειο που ανοιξαμε
	fclose(in_file);
   
   	printf("Parent closed all open files, deallocated all resources and semaphores and is now exiting. Auf wiedersehen!\n");

    exit(EXIT_SUCCESS);

}