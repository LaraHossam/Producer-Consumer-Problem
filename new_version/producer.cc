#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <sys/sem.h>
#include <ctime>
#include <random>
#include <math.h>
#define MAX_BUFFERS 200 // Buffer size
using namespace std;

struct Commodity{
    char name[20];
    double currPrice;
    double histPrice[4];
    int histIndex;
    double avgPrice;
    int priceColor;
    int avgColor;
};
typedef struct Commodity Commodity;



struct shared_memory {
    struct Commodity buf[MAX_BUFFERS];
    int buffer_index;
    int buffer_print_index;
};



int main(int argc,char*argv[])
{
    string msg = "\033[1;32m";
    msg += argv[1];
    msg+= ": Started Working.";
    msg+= "\033[0m\n";
    cout << msg;


    /*
    ########################################
    ######## CREATING SHARED MEMORY ########
    ########################################
    */

    /* Defining a key of type key_t defined in file sys/types.h 
    for requesting resources such as shared memory, message queues
    and semaphores. Length of key is system dependent, so we don't 
    use int.*/   
    key_t key;  
    key = ftok ("./d", 12345);
    if (key == -1)
    {
        cout << "\033[1;31mError in ftok\033[0m\n";
    }

    /* Get shared memory, shmget gets you a shared memory segment
    associated with the given key obtained with ftok. IPC_CREAT a new
    shared memory segment is created. */
    int shm_id;
    shm_id = shmget (key, sizeof (struct shared_memory), 0660 | IPC_CREAT);
    if (shm_id == -1)
    {
        cout << "\033[1;31mError in shmget\033[0m\n";
    }

    /* shmat, the calling process can attach the shared memory segment
    identified by shm_id*/
    struct shared_memory *shared_mem_ptr;
    shared_mem_ptr = (struct shared_memory *) shmat (shm_id, NULL, 0);
    if (shared_mem_ptr == (struct shared_memory *) -1)
    {
        cout << "\033[1;31mError in shmat\033[0m\n";
    }

    /*
    ########################################
    ############ CREATING MUTEX ############
    ########################################
    */
    key = ftok ("./a", 12345);
    if (key == -1)
    {
        cout << "\033[1;31mError in ftok\033[0m\n";
    }
    int mutex_sem;
    mutex_sem = semget (key, 1, 0660 | IPC_CREAT);
    if (mutex_sem == -1)
    {
        cout << "\033[1;31mError in semget\033[0m\n";
    }
    /* Giving mutex semaphore initial value of 0 to lock it. */
    int init =  semctl (mutex_sem, 0, SETVAL, 0) ;        
    if (init == -1)
    {
        cout << "\033[1;31mError in semctl\033[0m\n";
    }


    /*
    ########################################
    ######  CREATING EMPTY SEMAPHORE  ######
    ########################################
    */
    /* Semaphore indicating number of available buffers. */
    key = ftok ("./b", 12345);
    if (key == -1)
    {
        cout << "\033[1;31mError in ftok\033[0m\n";
    }
    int empty_sem;
    empty_sem = semget (key, 1, 0660 | IPC_CREAT);
    if (empty_sem == -1)
    {
        cout << "\033[1;31mError in semget\033[0m\n";
    }
    /* Giving mutex semaphore initial value of MAX_BUFFERS. */
    init =  semctl (empty_sem, 0, SETVAL, MAX_BUFFERS) ;        
    if (init == -1)
    {
        cout << "\033[1;31mError in semctl\033[0m\n";
    }


    /*
    ########################################
    #######  CREATING FULL SEMAPHORE  ######
    ########################################
    */
    /* Semaphore indicating number of full buffers. */
    key = ftok ("./b", 12389);
    if (key == -1)
    {
        cout << "\033[1;31mError in ftok\033[0m\n";
    }
    int full_sem;
    full_sem = semget (key, 1, 0660 | IPC_CREAT);
    if (empty_sem == -1)
    {
        cout << "\033[1;31mError in semget\033[0m\n";
    }
    init =  semctl (full_sem, 0, SETVAL, 0) ;        
    if (init == -1)
    {
        cout << "\033[1;31mError in semctl\033[0m\n";
    }


    /*
    ########################################
    ###########  INITIALIZATION   ##########
    ########################################
    */
    // shared_mem_ptr -> buffer_index = shared_mem_ptr -> buffer_print_index = 0;
    init = semctl (mutex_sem, 0, SETVAL, 1);
    if (init == -1)
    {
        cout << "\033[1;31mError in semctl\033[0m\n";
    }
    /* INITIALIZATION of WAIT and SIGNAL system calls*/
    struct sembuf wait, signal;
    wait.sem_num = signal.sem_num = 0;
    wait.sem_flg = signal.sem_flg = 0;
    wait.sem_op = -1;   // Value that should be added to the semaphore
    signal.sem_op = 1;  // Value that should be added to the semaphore
    
 



     if(argc != 5 || (strcmp(argv[1],"GOLD") && strcmp(argv[1],"SILVER") && strcmp(argv[1],"CRUDEOIL") && strcmp(argv[1],"NATURALGAS") && strcmp(argv[1],"ALUMINIUM") && strcmp(argv[1],"COPPER") && strcmp(argv[1],"NICKEL") && strcmp(argv[1],"LEAD") && strcmp(argv[1],"ZINC") && strcmp(argv[1],"MENTHAOIL") && strcmp(argv[1],"COTTON") ))
    {
        printf("Incorrect format. Please use:\n./producer <commodity-name>> <price-mean> <price-std> <sleep-interval>\n");
        return 1;
    }


    /*
    ########################################
    ###########  PRODUCER CODE   ###########
    ########################################
    */

    while (true) {
        
        struct timespec start;


        default_random_engine generator;
        generator.seed(time(0)); // To produce different random numbers according to a variable seed
        double var = sqrt(stod(argv[3]));
        normal_distribution<double> distribution(stod(argv[2]),var);
        double currPrice = abs(distribution(generator));

        clock_gettime( CLOCK_REALTIME, &start);
        string msg = "\033[1;31m";
        // msg += to_string(start.nsec);
        msg += argv[1];
        msg+= ": generating a new value ";
        msg += to_string(currPrice);
        msg+= "\033[0m\n";
        cout << msg;




        int fn;
        fn = semop(empty_sem,&wait,1);
        if (fn == -1)
        {
            cout << "\033[1;31mError in semop\033[0m\n";
        }

        clock_gettime( CLOCK_REALTIME, &start);
        msg = "\033[1;31m";
        // msg += to_string(start.nsec);
        msg += argv[1];
        msg+= ": trying to get mutex on shared buffer";
        msg+= "\033[0m\n";
        cout << msg;

        fn = semop(mutex_sem,&wait,1);
        if (fn == -1)
        {
            cout << "\033[1;31mError in semop\033[0m\n";
        }        
        

        clock_gettime( CLOCK_REALTIME, &start);
        msg = "\033[1;31m";
        // msg += to_string(start.nsec);
        msg += argv[1];
        msg+= ": placing ";
        msg += to_string(currPrice);
        msg += " on shared buffer";
        msg+= "\033[0m\n";
        cout << msg;
        
        strcpy(shared_mem_ptr -> buf[shared_mem_ptr -> buffer_index].name,argv[1]);
        shared_mem_ptr -> buf[shared_mem_ptr -> buffer_index].currPrice=currPrice;

        (shared_mem_ptr -> buffer_index)++;
        if (shared_mem_ptr -> buffer_index == MAX_BUFFERS)
            shared_mem_ptr -> buffer_index = 0;



        fn = semop(mutex_sem,&signal,1);
        if (fn == -1)
        {
            cout << "\033[1;31mError in semop\033[0m\n";
        }        
    
	    fn = semop(full_sem,&signal,1);
        if (fn == -1)
        {
            cout << "\033[1;31mError in semop\033[0m\n";
        }      


        clock_gettime( CLOCK_REALTIME, &start);
        msg = "\033[1;31m";
        // msg += to_string(start.nsec);
        msg += argv[1];
        msg+= ": sleeping for ";
        msg += argv[4];
        msg +=" ms";
        msg+= "\033[0m\n";
        cout << msg;
         
        sleep(stoi(argv[4]));
    }
 
    
    return 0;
}