// CREDITS: LARA HOSSAMELDIN MOSTAFA ABDOU
// ID: 6853

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
#include <sys/time.h>
#include <ctime>
#include <random>
#include <math.h>

#define MAX_BUFFERS 1000 // Buffer size
using namespace std;


struct Commodity{
    char name[20];
    double currPrice;
    double histPrice[5];
    int histIndex;
    double avgPrice;
    int priceColor;
    int avgColor;
};
typedef struct Commodity Commodity;



struct shared_memory {
    int in;
    int out;
    int size;
    int terminated;
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
    key = ftok ("./d", 12222);
    if (key == -1)
    {
        cout << "\033[1;31mError in ftok\033[0m\n";
    }

    /* Get shared memory, shmget gets you a shared memory segment
    associated with the given key obtained with ftok. IPC_CREAT a new
    shared memory segment is created. */
    int shm_id;
    shm_id = shmget (key, sizeof(shared_memory), 0660);
    if (shm_id == -1)
    {
        cout << "\033[1;31mError in shmget\033[0m\n";
    }

    /* shmat, the calling process can attach the shared memory segment
    identified by shm_id*/
    shared_memory *mem_ptr;
    mem_ptr = (shared_memory *) shmat (shm_id, NULL, 0);
    if (mem_ptr == (shared_memory *) -1)
    {
        cout << "\033[1;31mError in shmat\033[0m\n";
    }

    /* Now, we define actual shared memory*/

    key_t key2;  
    key2 = ftok ("./d", 1);
    if (key2 == -1)
    {
        cout << "\033[1;31mError in ftok\033[0m\n";
    }

    /* Get shared memory, shmget gets you a shared memory segment
    associated with the given key obtained with ftok. IPC_CREAT a new
    shared memory segment is created. */
    int shm_id2;
    shm_id2 = shmget (key2, sizeof(Commodity)*mem_ptr->size, 0660);
    if (shm_id2 == -1)
    {
        cout << "\033[1;31mError in shmget\033[0m\n";
    }

    /* shmat, the calling process can attach the shared memory segment
    identified by shm_id*/
    Commodity *buf_ptr;
    buf_ptr = (Commodity*) shmat (shm_id2, NULL, 0);
    if (buf_ptr == (Commodity *) -1)
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
    if (full_sem == -1)
    {
        cout << "\033[1;31mError in semget\033[0m\n";
    }

    /*
    ########################################
    ###########  INITIALIZATION   ##########
    ########################################
    */

    struct sembuf wait, signal;
    wait.sem_num = signal.sem_num = 0;
    wait.sem_flg = signal.sem_flg = 0;
    wait.sem_op = -1;   // Value that should be added to the semaphore
    signal.sem_op = 1;  // Value that should be added to the semaphore
    
 



     if(argc != 6 || (strcmp(argv[1],"GOLD") && strcmp(argv[1],"SILVER") && strcmp(argv[1],"CRUDEOIL") && strcmp(argv[1],"NATURALGAS") && strcmp(argv[1],"ALUMINUM") && strcmp(argv[1],"COPPER") && strcmp(argv[1],"NICKEL") && strcmp(argv[1],"LEAD") && strcmp(argv[1],"ZINC") && strcmp(argv[1],"METHANOIL") && strcmp(argv[1],"COTTON") ))
    {
        printf("Incorrect format. Please use:\n./producer <commodity-name>> <price-mean> <price-std> <sleep-interval> <buonded-buffer-size>\n");
        return 1;
    }
    if (atoi(argv[5])!=mem_ptr->size)
    {
        printf("Bounded Buffer Size doesn't match limit set by consumer (%d) Try again.\n",mem_ptr->size);
        return 1;
    }

    

    /*
    ########################################
    ###########  PRODUCER CODE   ###########
    ########################################
    */


    while (true) {
    
    if(mem_ptr->terminated == 1)
    {
        cout << "\n\nTerminated Consumer. All shared memory segments deleted."  << endl;
        exit(1);
    }
        
    struct timeval tv;
    struct timezone tz;
    struct tm *tm;
    default_random_engine generator;
    generator.seed(time(0)); // To produce different random numbers according to a variable seed
    double var = sqrt(stod(argv[3]));
    normal_distribution<double> distribution(stod(argv[2]),var);
    double currPrice = abs(distribution(generator));
    string temp;
    gettimeofday(&tv, &tz);
    tm=localtime(&tv.tv_sec);
    temp = to_string(tv.tv_usec).substr(0, 3);
    string msg = "\033[1;31m";
    msg+= "[" + to_string(tm->tm_mday) + "/" + to_string(tm->tm_mon) +"/" + to_string(tm->tm_year+1900)+ " " + to_string(tm->tm_hour)+":"+to_string(tm->tm_min)+":"+to_string(tm->tm_sec) + "."+temp +"] ";
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
    gettimeofday(&tv, &tz);
    tm=localtime(&tv.tv_sec);
    temp = to_string(tv.tv_usec).substr(0, 3);
    msg = "\033[1;31m";
    msg+= "[" + to_string(tm->tm_mday) + "/" + to_string(tm->tm_mon) +"/" + to_string(tm->tm_year+1900)+ " " + to_string(tm->tm_hour)+":"+to_string(tm->tm_min)+":"+to_string(tm->tm_sec) + "."+temp +"] ";
    msg += argv[1];
    msg+= ": trying to get mutex on shared buffer";
    msg+= "\033[0m\n";
    cout << msg;

    fn = semop(mutex_sem,&wait,1);
    if (fn == -1)
    {
        cout << "\033[1;31mError in semop\033[0m\n";
    }        
    gettimeofday(&tv, &tz);
    tm=localtime(&tv.tv_sec);
    temp = to_string(tv.tv_usec).substr(0, 3);
    msg = "\033[1;31m";
    msg+= "[" + to_string(tm->tm_mday) + "/" + to_string(tm->tm_mon) +"/" + to_string(tm->tm_year+1900)+ " " + to_string(tm->tm_hour)+":"+to_string(tm->tm_min)+":"+to_string(tm->tm_sec) + "."+temp +"] ";
    msg += argv[1];
    msg+= ": placing ";
    msg += to_string(currPrice);
    msg += " on shared buffer";
    msg+= "\033[0m\n";
    cout << msg;
    
    strcpy(buf_ptr[mem_ptr -> in].name,argv[1]);
    buf_ptr[mem_ptr -> in].currPrice=currPrice;

    (mem_ptr -> in)++;
    if (mem_ptr -> in == mem_ptr->size)
        mem_ptr -> in = 0;



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

    gettimeofday(&tv, &tz);
    tm=localtime(&tv.tv_sec);
    temp = to_string(tv.tv_usec).substr(0, 3);
    msg = "\033[1;31m";
    msg+= "[" + to_string(tm->tm_mday) + "/" + to_string(tm->tm_mon) +"/" + to_string(tm->tm_year+1900)+ " " + to_string(tm->tm_hour)+":"+to_string(tm->tm_min)+":"+to_string(tm->tm_sec) + "."+temp +"] ";
    msg += argv[1];
    msg+= ": sleeping for ";
    msg += argv[4];
    msg +=" ms";
    msg+= "\033[0m\n";
    cout << msg;
        
    usleep(stoi(argv[4])*1000);
}

    
    return 0;
}