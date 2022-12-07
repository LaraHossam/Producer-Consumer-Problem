/*
 *
 *       spooler.c: Print strings in the shared memory segment
 *                  (Consumer process)
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <sys/sem.h>

// Buffer data structures
#define MAX_BUFFERS 10
#define SHARED_MEMORY_KEY "/tmp/shared_memory_key"
#define SEM_MUTEX_KEY "/tmp/sem-mutex-key"
#define SEM_BUFFER_COUNT_KEY "/tmp/sem-buffer-count-key"
#define SEM_SPOOL_SIGNAL_KEY "/tmp/sem-spool-signal-key"
#define PROJECT_ID 'K'

struct shared_memory {
    char buf [MAX_BUFFERS] [256];
    int buffer_index;
    int buffer_print_index;
};

void error (char *msg);

int main (int argc, char **argv)
{
    key_t s_key;
    union semun  
    {
        int val;
        struct semid_ds *buf;
        ushort array [1];
    } sem_attr;
    int shm_id;
    struct shared_memory *shared_mem_ptr;
    int mutex_sem, buffer_count_sem, spool_signal_sem;
    
    printf ("spooler: hello world\n");
    //  mutual exclusion semaphore
    /* generate a key for creating semaphore  */
    if ((s_key = ftok (SEM_MUTEX_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((mutex_sem = semget (s_key, 1, 0660 | IPC_CREAT)) == -1)
        error ("semget");
    // Giving initial value. 
    sem_attr.val = 0;        // locked, till we finish initialization
    if (semctl (mutex_sem, 0, SETVAL, sem_attr) == -1)
        error ("semctl SETVAL");
    
    // Get shared memory 
    if ((s_key = ftok (SHARED_MEMORY_KEY, PROJECT_ID)) == -1)
        error ("ftok");    
    if ((shm_id = shmget (s_key, sizeof (struct shared_memory), 
         0660 | IPC_CREAT)) == -1)
        error ("shmget");
    if ((shared_mem_ptr = (struct shared_memory *) shmat (shm_id, NULL, 0)) 
         == (struct shared_memory *) -1) 
        error ("shmat");
    // Initialize the shared memory
    shared_mem_ptr -> buffer_index = shared_mem_ptr -> buffer_print_index = 0;

    // counting semaphore, indicating the number of available buffers.
    /* generate a key for creating semaphore  */
    if ((s_key = ftok (SEM_BUFFER_COUNT_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((buffer_count_sem = semget (s_key, 1, 0660 | IPC_CREAT)) == -1)
        error ("semget");
    // giving initial values
    sem_attr.val = MAX_BUFFERS;    // MAX_BUFFERS are available
    if (semctl (buffer_count_sem, 0, SETVAL, sem_attr) == -1)
        error ("semctl SETVAL");

    // counting semaphore, indicating the number of strings to be printed.
    /* generate a key for creating semaphore  */
    if ((s_key = ftok (SEM_SPOOL_SIGNAL_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((spool_signal_sem = semget (s_key, 1, 0660 | IPC_CREAT)) == -1)
        error ("semget");
    // giving initial values
    sem_attr.val = 0;    // 0 strings are available initially.
    if (semctl (spool_signal_sem, 0, SETVAL, sem_attr) == -1)
        error ("semctl SETVAL");
    
    // Initialization complete; now we can set mutex semaphore as 1 to 
    // indicate shared memory segment is available
    sem_attr.val = 1;
    if (semctl (mutex_sem, 0, SETVAL, sem_attr) == -1)
        error ("semctl SETVAL"); 

    struct sembuf asem [1];

    asem [0].sem_num = 0;
    asem [0].sem_op = 0;
    asem [0].sem_flg = 0;

    while (1) {  // forever
        // Is there a string to print? P (spool_signal_sem);
        asem [0].sem_op = -1;
        if (semop (spool_signal_sem, asem, 1) == -1)
	    perror ("semop: spool_signal_sem");
    
        printf ("%s", shared_mem_ptr -> buf [shared_mem_ptr -> buffer_print_index]);

        /* Since there is only one process (the spooler) using the 
           buffer_print_index, mutex semaphore is not necessary */
        (shared_mem_ptr -> buffer_print_index)++;
        if (shared_mem_ptr -> buffer_print_index == MAX_BUFFERS)
           shared_mem_ptr -> buffer_print_index = 0;

        /* Contents of one buffer has been printed.
           One more buffer is available for use by producers.
           Release buffer: V (buffer_count_sem);  */
        asem [0].sem_op = 1;
        if (semop (buffer_count_sem, asem, 1) == -1)
	    perror ("semop: buffer_count_sem");
    }
}

// Print system error and exit
void error (char *msg)
{
    perror (msg);
    exit (1);
}


/*
 *
 *       client.c: Write strings for printing in the shared memory segment
 *                 (Producer process)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <sys/sem.h>

// Buffer data structures
#define MAX_BUFFERS 10
#define SHARED_MEMORY_KEY "/tmp/shared_memory_key"
#define SEM_MUTEX_KEY "/tmp/sem-mutex-key"
#define SEM_BUFFER_COUNT_KEY "/tmp/sem-buffer-count-key"
#define SEM_SPOOL_SIGNAL_KEY "/tmp/sem-spool-signal-key"
#define PROJECT_ID 'K'

struct shared_memory {
    char buf [MAX_BUFFERS] [256];
    int buffer_index;
    int buffer_print_index;
};

void error (char *msg);

int main (int argc, char **argv)
{
    key_t s_key;
    union semun  
    {
        int val;
        struct semid_ds *buf;
        ushort array [1];
    } sem_attr;
    int shm_id;
    struct shared_memory *shared_mem_ptr;
    int mutex_sem, buffer_count_sem, spool_signal_sem;
    
    //  mutual exclusion semaphore
    /* generate a key for creating semaphore  */
    if ((s_key = ftok (SEM_MUTEX_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((mutex_sem = semget (s_key, 1, 0)) == -1)
        error ("semget");
    
    // Get shared memory 
    if ((s_key = ftok (SHARED_MEMORY_KEY, PROJECT_ID)) == -1)
        error ("ftok");    
    if ((shm_id = shmget (s_key, sizeof (struct shared_memory), 0)) == -1)
        error ("shmget");
    if ((shared_mem_ptr = (struct shared_memory *) shmat (shm_id, NULL, 0)) 
         == (struct shared_memory *) -1) 
        error ("shmat");

    // counting semaphore, indicating the number of available buffers.
    /* generate a key for creating semaphore  */
    if ((s_key = ftok (SEM_BUFFER_COUNT_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((buffer_count_sem = semget (s_key, 1, 0)) == -1)
        error ("semget");

    // counting semaphore, indicating the number of strings to be printed.
    /* generate a key for creating semaphore  */
    if ((s_key = ftok (SEM_SPOOL_SIGNAL_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((spool_signal_sem = semget (s_key, 1, 0)) == -1)
        error ("semget");
    
    struct sembuf asem [1];

    asem [0].sem_num = 0;
    asem [0].sem_op = 0;
    asem [0].sem_flg = 0;

    char buf [200];

    printf ("Please type a message: ");

    while (fgets (buf, 198, stdin)) {
        // remove newline from string
        int length = strlen (buf);
        if (buf [length - 1] == '\n')
           buf [length - 1] = '\0';

        // get a buffer: P (buffer_count_sem);
        asem [0].sem_op = -1;
        if (semop (buffer_count_sem, asem, 1) == -1)
	    error ("semop: buffer_count_sem");
    
        /* There might be multiple producers. We must ensure that 
            only one producer uses buffer_index at a time.  */
        // P (mutex_sem);
        asem [0].sem_op = -1;
        if (semop (mutex_sem, asem, 1) == -1)
	    error ("semop: mutex_sem");

	    // Critical section
            sprintf (shared_mem_ptr -> buf [shared_mem_ptr -> buffer_index], "(%d): %s\n", getpid (), buf);
            (shared_mem_ptr -> buffer_index)++;
            if (shared_mem_ptr -> buffer_index == MAX_BUFFERS)
                shared_mem_ptr -> buffer_index = 0;

        // Release mutex sem: V (mutex_sem)
        asem [0].sem_op = 1;
        if (semop (mutex_sem, asem, 1) == -1)
	    error ("semop: mutex_sem");
    
	// Tell spooler that there is a string to print: V (spool_signal_sem);
        asem [0].sem_op = 1;
        if (semop (spool_signal_sem, asem, 1) == -1)
	    error ("semop: spool_signal_sem");

        printf ("Please type a message: ");
    }
 
    if (shmdt ((void *) shared_mem_ptr) == -1)
        error ("shmdt");
    exit (0);
}

// Print system error and exit
void error (char *msg)
{
    perror (msg);
    exit (1);
}





// We can compile and run the spooler as below.
$ gcc spooler.c -o spooler
$ >/tmp/shared_memory_key
$ >/tmp/sem-mutex-key
$ >/tmp/sem-buffer-count-key
$ >/tmp/sem-spool-signal-key
$ ./spooler
spooler: hello world
(5942): hello spooler
(5980): Hello spooler
(6012): Hello world

// And compiling and running a client from another terminal,

$ gcc client.c -o client
$ ./client
Please type a message: Hello spooler
...