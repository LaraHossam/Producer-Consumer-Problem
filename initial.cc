// Lara HossamElDin Mostafa Abdou 
// ID: 6853

/*
  --------------
 | System V IPC |
  --------------
There are three types of asynchronous IP C originated from System V:
1- Semaphores : Counter to provide access to a shared data object for multiple processes.
2- Shared Memory
3- Message Queues
All three use IPC structures, referred to by an identifier and a key.
These structures only exist in kernel space and are not manifested in the file system.
We use special system calls.
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <err.h>
#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>

#define Wait(s) semop(s,&wait,1)       //Structure we pass for the wait operation
#define Signal(s) semop(s, &signal,1) //Structure we pass for the signal operation
#define N 11 // Maximum buffer size

using namespace std;



class Commodity{
    string name;
    vector<int> price;
};


int main(int argc, char* argv[]){
    int items = 0;
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

    // Initializing buffer
    int in = 0;
    int out = 0;
    int buffer[N];


    while(true){
    cout << "1. Producer or 2. Consumer? "<< endl;
    int x;
    cin >> x;
    if(x==1){
        cout << "Before empty check" << endl;
        Wait(empty);
        cout << "There is an empty slot, yay!" <<endl;
        Wait(sem);
        sleep(rand() % 100 * 0.01);
        buffer[in]=++items;
        cout << "Producer produces item = " << buffer[in] << endl;
        in = (in+1)%(N);
        Signal(sem);
        Signal(full);
        sleep(rand() % 100 * 0.01);
    }
    if(x==2){
        Wait(full);
        Wait(sem);
        sleep(rand() % 100 * 0.01);
        cout << "Consumer consumes item = " << buffer[out] << endl;
        out = (out+1)%(N);
        Signal(sem);
        Signal(empty);
        sleep(rand() % 100 * 0.01);
    }

    }




    return 0;
}
