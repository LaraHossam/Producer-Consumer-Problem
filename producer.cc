#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <random>
#include <iostream>
#include <ctime>
#define SHMSIZE 2048 // Shared memory size between producers and consumer
#define Wait(s) semop(s,&wait,1)       //Structure we pass for the wait operation
#define Signal(s) semop(s, &signal,1) //Structure we pass for the signal operation
#define N 11
using namespace std;



int main(int argc,char*argv[])
{
    if(argc != 5 || (strcmp(argv[1],"GOLD") && strcmp(argv[1],"SILVER") && strcmp(argv[1],"CRUDEOIL") && strcmp(argv[1],"NATURALGAS") && strcmp(argv[1],"ALUMINIUM") && strcmp(argv[1],"COPPER") && strcmp(argv[1],"NICKEL") && strcmp(argv[1],"LEAD") && strcmp(argv[1],"ZINC") && strcmp(argv[1],"MENTHAOIL") && strcmp(argv[1],"COTTON") ))
    {
        printf("Incorrect format. Please use:\n./producer <commodity-name>> <price-mean> <price-std> <sleep-interval>\n");
        return 1;
    }
    default_random_engine generator;
    generator.seed(time(0)); // To produce different random numbers according to a variable seed
    normal_distribution<double> distribution(stod(argv[2]),stod(argv[3]));
    double currPrice = distribution(generator);


    int empty, full,sem;
    // Initialization for Wait and Signal system calls
    struct sembuf wait, signal;
    wait.sem_num = signal.sem_num = 0;
    wait.sem_flg = signal.sem_flg = 0;
    wait.sem_op = -1; // Value that should be added to the semaphore
    signal.sem_op = 1; // Value that should be added to the semaphore

    // Initializing semaphores; empty and full
    empty = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    full = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    sem = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    semctl(empty, 0, SETVAL, N); // Number of commodities available
    semctl(full, 0, SETVAL, 0);
    semctl(sem, 0, SETVAL, 1);
    return 0;
}