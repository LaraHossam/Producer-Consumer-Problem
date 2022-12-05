#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>


int main(int argc,char*argv[])
{
    if(argc != 5 || strcmp(argv[1],"hi"))
    {
        printf("Incorrect format. Please use:\n./producer <commodity-name>> <price-mean> <price-std> <sleep-interval>\n");
        return 1;
    }
    return 0;
}