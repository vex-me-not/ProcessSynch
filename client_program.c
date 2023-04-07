#include <time.h>

#include "misc.h"


int main(int argc, char* argv[]){

    //Δεν θα κανουμε ελεγχο στην εισοδο οπως στο server_program γιατι φροντιζουμε να εχουμε σωστη εισοδο στη κληση της execv στο server_program

    srand(getpid());  // Καλουμε την srand με seed το id του παιδιου για να αλλαζει σε καθε παιδι και να εχουμε καλυτερη τυχαιοτητα

    int n_tactions = atoi(argv[1]); // το πληθος των δοσοληψιων για καθε παιδι
    int server_sem = atoi(argv[2]);  // O σημαφορος που διαχειριζεται τον server
    int client_sem = atoi(argv[3]);  //Ο σημαφορος που διαχειριζεται ποτε μπορει καποιο παιδι να κανει αιτημα/διαβασει απο τη μνημη
    int inter_client_sem =atoi(argv[4]);  // Ο σημαφορος που διαχειριζεται ποιο παιδι μπορει να κανει αιτημα/διαβασει απο τη μνημη
    int shmid = atoi(argv[5]);  // Η διαμειραζομενη μνημη
    int x_file_lines = atoi(argv[6]);  // Το πληθος των γραμμων του αρχειου Χ

    memory* com_mem = shmat(shmid, NULL, 0);  // Κανουμε attach την διαμοιραζομενη μνημη
    
    if (com_mem == (memory *)-1) {  // Ελεγχος σφαλματος κατα το attachment
		perror("Memory attachment");
		exit(EXIT_FAILURE);
	} 
    
    int* demand = &(com_mem->get_line);       //Παιρνουμε τη διευθυνση των αιτησεων
    char* answer = com_mem->line;             //Παιρνουμε τη διευθυνση των απαντησεων

    int get_line;                           // Η γραμμη που θα ζητησουμε

    clock_t el_time, begin, end;           //Διαφοροι χρονοι που θα χρειαστουμε στο τελος

    el_time=0;
    // Συμμετεχουμε σε Ν δοσοληψιεσ
    for(int i = 0; i< n_tactions;i++){
        
        //Προλαβαινουμε τα αλλα παιδια πριν θελησουν αυτα να λαβουν μερος σε δοσοληψια
        if(sem_P(inter_client_sem) < 0){
            exit(EXIT_FAILURE);
        }

        //Καταθετουμε το αιτημα μας
        if(sem_P(client_sem) < 0){
            exit(EXIT_FAILURE);
        }

        get_line = (rand() % x_file_lines) + 1;  //Ποια γραμμη θα αιτηθει το παιδι,+1 για να μην βγει ποτε 0 αφου δεν υπαρχει γραμμη 0
        begin = clock();                         //Καναμε αιτημα , αρα χρονομετρουμε 
        
        *demand = get_line;                      //Καταθετουμε το αιτημα στη διαμοιραζόμενη μνημη
        printf("Child %i demands line %i\n",getpid(),get_line);

        //Επιστρεφουμε τον ελεγχο στον γονεα ετσι ωστε να μπορεσει να απαντησει
        if(sem_V(server_sem) < 0){
            exit(EXIT_FAILURE);
        }

        //Αιτουμαστε να διαβασουμε την απαντηση στο αιτημα μας
        if(sem_P(client_sem) < 0){
            exit(EXIT_FAILURE);
        }

        //Πηραμε απαντηση αρα το αιτημα μας ικανοποιηθηκε
        end = clock();
        clock_t dt = end - begin;  // Ο χρονος που πηρε μια δοσοληψια
        
        //Εκτυπωνουμε την απαντηση στο αιτημα μας
        printf("Child %i demanded line %i. This line reads: %s",getpid(),get_line,answer);
        
        // Στον συνολικο χρονο που "τρεχει" το παιδι προσθέτουμε το χρόνο της δοσοληψιας
        el_time += dt;

        //Δεν χρειαζομαστε αλλο την μνημη αρα
        //Επιτρέπουμε στα υπολοιπα παιδια να συναγωνιστουν για το ποιο θα αποκτησει προσβαση στη μνημη
        if(sem_V(inter_client_sem) < 0){
            exit(EXIT_FAILURE);
        }

        //Και κανουμε up τον client_sem έτσι ώστε το νικηφορο παιδι να μπορεί να αλληλεπιδρασει με την μνημη
        if(sem_V(client_sem) < 0){
            exit(EXIT_FAILURE);
        }        

    }
    
    //Θα μετατρεψουμε τον συνολικο χρονο σε seconds αφου προς το παρον ειναι σε CLOCKS
    double total_time;
    total_time = (double) el_time/CLOCKS_PER_SEC;  // ο συνολικος χρονος για Ν δοσοληψιες σε seconds
    
    double av_time;       
    av_time = total_time/n_tactions; // Ο μεσος χρονος για καθε δοσοληψια
    printf("\nEach transcaction of child %i takes on average %f seconds\n",getpid(),av_time);

    exit(EXIT_SUCCESS);
}