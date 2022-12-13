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
#include <string.h>
#include<sstream>
#include <signal.h> 

#define N 11
#define MAX_BUFFERS 1000 // Buffer size
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
    int in;
    int out;
    int size;
    int terminated;
};

int shm_id;
int shm_id2;
shared_memory *mem_ptr;

// Define the function to be called when ctrl-c (SIGINT) is sent to process
void signal_callback_handler(int i) {
    mem_ptr->terminated=1;
    cout << "\nTerminated Consumer. All shared memory segments deleted."  << endl;
    if(shmctl(shm_id2, IPC_RMID, 0)==-1)
    {
    cout << "\033[1;31mError in removing shmid2.\033[0m\n";
    }
    // Terminate program
    exit(1);
}


int main(int argc,char*argv[])
{
     
    cout << "\033[1;32mCONSUMER STARTED WORKING.\033[0m\n";
    signal(SIGINT, signal_callback_handler);

    if(argc!=2)
    {
         printf("Incorrect format. Please use:\n./consumer <bounded-buffer-size>\n");
        return 1;
    }
    int size = stoi(argv[1]);

    /*
    ########################################
    ######## CREATING SHARED MEMORY ########
    ########################################
    */

    /* Defining a key of type key_t defined in file sys/types.h 
    for requesting resources such as shared memory, message queues
    and semaphores. Length of key is system dependent, so we don't 
    use int.*/   

    /* Following memory is to share index in, index out and size of buffer */

    key_t key;  
    key = ftok ("./d", 12222);
    if (key == -1)
    {
        cout << "\033[1;31mError in ftok\033[0m\n";
    }

    /* Get shared memory, shmget gets you a shared memory segment
    associated with the given key obtained with ftok. IPC_CREAT a new
    shared memory segment is created. */
    
    shm_id = shmget (key, sizeof(shared_memory), 0660 | IPC_CREAT);
    if (shm_id == -1)
    {
        cout << "\033[1;31mError in shmget\033[0m\n";
    }

    /* shmat, the calling process can attach the shared memory segment
    identified by shm_id*/
    mem_ptr = (shared_memory *) shmat (shm_id, NULL, 0);
    if (mem_ptr == (shared_memory *) -1)
    {
        cout << "\033[1;31mError in shmat\033[0m\n";
    }
    mem_ptr -> size = size;

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
    shm_id2 = shmget (key2, sizeof(Commodity)*mem_ptr -> size, 0660 | IPC_CREAT);
    if (shm_id2 == -1)
    {
        cout << "\033[1;31mError in shmget2\033[0m\n";
    }

    /* shmat, the calling process can attach the shared memory segment
    identified by shm_id*/
    Commodity *buf_ptr;
    buf_ptr = (Commodity*) shmat (shm_id2, NULL, 0);
    if (buf_ptr == (Commodity *) -1)
    {
        cout << "\033[1;31mError in shmat2\033[0m\n";
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
    int init =  semctl (mutex_sem, 0, SETVAL, 1) ;        
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
    /* Giving mutex semaphore initial value of mem_ptr->size. */
    init =  semctl (empty_sem, 0, SETVAL, mem_ptr->size) ;        
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
    if (full_sem == -1)
    {
        cout << "\033[1;31mError in semget\033[0m\n";
    }
    /* Giving mutex semaphore initial value of mem_ptr->size. */
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
    mem_ptr -> in = mem_ptr -> out = 0;
    mem_ptr ->terminated = 0;
    init = semctl (mutex_sem, 0, SETVAL, 1);
    if (init == -1)
    {
        cout << "\033[1;31mError in semctl\033[0m\n";
    }
    struct sembuf wait, signal;
    wait.sem_num = signal.sem_num = 0;
    wait.sem_flg = signal.sem_flg = 0;
    wait.sem_op = -1;   // Value that should be added to the semaphore
    signal.sem_op = 1;  // Value that should be added to the semaphore
    

    vector<string> names = {"ALUMINUM     ","COPPER       ","COTTON       ","CRUDEOIL     ","GOLD         ","LEAD         ","METHANOIL    ","NATURALGAS   ","NICKEL       ","SILVER       ","ZINC         "};
    struct Commodity com[N];
    for(int i=0;i<N;i++)
    {
        strcpy(com[i].name,names[i].c_str());
        com[i].currPrice=0.00;
        for (int j = 0; j < 4;j++)
        {
            com[i].histPrice[j]=0.00;
        }
        com[i].histIndex = 0;
        com[i].avgPrice = 0.00;
        com[i].priceColor = -1;
        com[i].avgColor = -1;
    }


     while (1) {  

        int fn;
        fn = semop(full_sem,&wait,1);
        if (fn == -1)
        {
            cout << "\033[1;31mError in semop\033[0m\n";
        }


        fn = semop(mutex_sem,&wait,1);
        if (fn == -1)
        {
            cout << "\033[1;31mError in semop\033[0m\n";
        }        
        
        /*
        ########################################
        ###########  ACCESSING MEMORY ##########
        ########################################
        */
        char name [20];
        strcpy(name,buf_ptr[mem_ptr->out].name);
        double currPrice = buf_ptr[mem_ptr->out].currPrice;
        
        // Updating array
        int j;
        if(strcmp(name,"ALUMINUM")==0) j = 0;
        if(strcmp(name,"COPPER")==0) j = 1;
        if(strcmp(name,"COTTON")==0) j = 2;
        if(strcmp(name,"CRUDEOIL")==0) j = 3;
        if(strcmp(name,"GOLD")==0) j = 4;
        if(strcmp(name,"LEAD")==0) j = 5;
        if(strcmp(name,"METHANOIL")==0) j = 6;
        if(strcmp(name,"NATURALGAS")==0) j = 7;
        if(strcmp(name,"NICKEL")==0) j = 8;
        if(strcmp(name,"SILVER")==0) j = 9;
        if(strcmp(name,"ZINC")==0) j = 10;

        if (com[j].currPrice < currPrice)
            com[j].priceColor = 1; // GREEN
        else
            com[j].priceColor = 0; // RED
        com[j].currPrice = currPrice;

        com[j].histPrice[com[j].histIndex]=currPrice;
        com[j].histIndex = (com[j].histIndex+1)%4;
        int count = 0;
        double sum=0;
        for (int k=0;k<4;k++)
        {
            if(com[j].histPrice[k]!=0)
            {
                count++;
                sum += com[j].histPrice[k];
            }
        }
        double newAvg = sum/count;
        if (com[j].avgPrice < newAvg)
            com[j].avgColor = 1; // GREEN
        else
            com[j].avgColor = 0; // RED
        com[j].avgPrice = newAvg;
        

       
        (mem_ptr -> out)++;
        if (mem_ptr -> out == mem_ptr->size)
           mem_ptr -> out = 0;

        /*
        ########################################
        ###########  PRINTING RESULTS ##########
        ########################################
        */

        cout << "+---------------------------------------+" << endl;
        cout << "| Currency     | Price    |  AvgPrice   |" << endl;
        string cPrice;
        string cAvg;
        string cCodeP;
        string cCodeA;
        string cCodeEnd = "";
        string price;
        string avgPrice;
        for(int i =0;i<N;i++)
        {
            if (com[i].priceColor == 1){
                cPrice = "↑";
                cCodeP = "\033[1;32m";
                cCodeEnd = "\033[0m";}
            else if(com[i].priceColor == 0){
                cPrice =  "↓";
                cCodeP = "\033[1;31m";
                cCodeEnd = "\033[0m";;}
            else{
                cPrice = "-";
                cCodeP = "\033[1;34m";
                cCodeEnd = "\033[0m";}
            if (com[i].avgColor == 1){
                cAvg = "↑";
                cCodeA = "\033[1;32m";
                cCodeEnd = "\033[0m";}
            else if(com[i].avgColor == 0){
                cAvg =  "↓";
                cCodeA = "\033[1;31m";
                cCodeEnd = "\033[0m";}
            else{
                cAvg = "-";
                cCodeA = "\033[1;34m";
                cCodeEnd = "\033[0m";}

            char priceC[10];
            char avgC[10];
            sprintf(priceC,"%7.2lf",com[i].currPrice);
            sprintf(avgC,"%7.2lf",com[i].avgPrice);

            price = cCodeP + priceC + cPrice + cCodeEnd;

            avgPrice = cCodeA + avgC + cAvg + cCodeEnd;

            cout << "| " << com[i].name << "|" << price << "  |    " << avgPrice << " |\n" ;
        }
        cout << "+---------------------------------------+" << endl;
        string msg = "\033[1;31m";
        msg += to_string(mem_ptr -> out);
        msg += "\033[0m\n";
        cout << msg;
        printf("\e[1;1H\e[2J");

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