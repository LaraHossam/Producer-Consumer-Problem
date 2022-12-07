    // CREDITS: LARA HOSSAMELDIN MOSTAFA ABDOU
    // ID: 6853

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <sys/sem.h>
#define N 11
#define MAX_BUFFERS 11
using namespace std;

struct Commodity{
    char name[20];
    vector<double> price;
    double currPrice = 0.00;
    double avgPrice = 0.00;
    // Commodity(char* name): name(name)
    // {
    // };
};




/*
########################################
########  SHARED MEMORY STRUCT  ########
########################################
*/

struct shared_memory {
    struct Commodity buf[MAX_BUFFERS];
    int buffer_index;
    int buffer_print_index;
};



int main(int argc,char*argv[])
{
     cout << "\033[1;32mCONSUMER STARTED WORKING.\033[0m\n";

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
    key = ftok (".", 12345);
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
    /* Giving mutex semaphore initial value of MAX_BUFFERS. */
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
    shared_mem_ptr -> buffer_index = shared_mem_ptr -> buffer_print_index = 0;
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
    
 


     while (1) {  


        int fn;
        fn = semop(full_sem,&wait,1);
        if (fn == -1)
        {
            cout << "\033[1;31mError in semop\033[0m\n";
        }

        /*
        ########################################
        ###########  ACCESSING MEMORY ##########
        ########################################
        */
        // cout << "Commodity name: " <<shared_mem_ptr -> buf[shared_mem_ptr -> buffer_print_index].name << endl;
        // cout << "Commodity currPrice: " <<shared_mem_ptr -> buf[shared_mem_ptr -> buffer_print_index].currPrice << endl;
        cout << "+-------------------------------------+" << endl;
        cout << "| Currency     | Price     | AvgPrice |" << endl;
        for(int i =0;i<N;i++)
        {
            shared_mem_ptr -> buffer_print_index = i;
            cout << "| " << shared_mem_ptr -> buf[shared_mem_ptr -> buffer_print_index].name << "     |" << shared_mem_ptr -> buf[shared_mem_ptr -> buffer_print_index].currPrice << "  |" << shared_mem_ptr ->  buf[shared_mem_ptr -> buffer_print_index].avgPrice << "    |" << endl;
        }
        cout << "+-------------------------------------+" << endl;
        printf("\e[1;1H\e[2J");


        /*
        ########################################
        ###########  PRINTING RESULTS ##########
        ########################################
        */

    // vector<string> commodities_names {"GOLD", "SILVER","CRUDEOIL","NATURALGAS","ALUMINUM","COPPER","NICKEL","LEAD","ZINC","METHANOIL","COTTON"};
    // vector<Commodity> commodities;
    // for(int i=0;i<N;i++)
    // {
    //  Commodity c = Commodity(commodities_names[i]);
    //  commodities.insert(commodities.begin(),c);
    // }




        fn = semop(mutex_sem,&wait,1);
        if (fn == -1)
        {
            cout << "\033[1;31mError in semop\033[0m\n";
        }        
        
        (shared_mem_ptr -> buffer_print_index)++;
        if (shared_mem_ptr -> buffer_print_index == MAX_BUFFERS)
           shared_mem_ptr -> buffer_print_index = 0;





        fn = semop(mutex_sem,&signal,1);
        if (fn == -1)
        {
            cout << "\033[1;31mError in semop\033[0m\n";
        }         
        fn = semop(empty_sem,&signal,1);
        if (fn == -1)
        {
            cout << "\033[1;31mError in semop\033[0m\n";
        }    
    }






    
    return 0;
}