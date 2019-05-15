#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h> 
#include <string.h>
#include <semaphore.h>
#include <stdlib.h>
#include "sope.h"

#define READ 0
#define WRITE 1

int main(int argc, char *argv[])
{
    int fd1, fd2;
    pid_t pidN = getpid();
    int id = atoi(argv[1]);
    char fifoName[USER_FIFO_PATH_LEN];
    char pid[WIDTH_ID+1];
    char * password = argv[2];
    int delay = atoi(argv[3]);

    if (argc < 5){
        printf("Insufficient number of arguments\n");
        return 1;
    }

    if (id < 0 || id > 4095)
    {
        printf("id must be a number between 0 and 4095\n");
        return 1;
    }

    if (strlen(password) <= MIN_PASSWORD_LEN || strlen(password) > MAX_PASSWORD_LEN + 1 )
    {
        printf("password must have between 8 and 20 characters\n");
        return 1;
    }

    if (delay < 0)
    {
        printf("delay cannot be a negative number\n");
        return 1;
    }


    if(atoi(argv[4]) < 0 || atoi(argv[4]) > 4){
        printf("operation must correspond to a number between 0 and 4\n");
        return 1;
    }

    int operation = atoi(argv[4]);

    char * args = argv[5];

    sprintf(pid, "%d", pidN);
    printf("pid: %s \n", pid);
    strcpy(fifoName, USER_FIFO_PATH_PREFIX);

    strcat(fifoName, pid);
    printf("fifoname: %s \n", fifoName);

    printf("\nid: %d\n", id);
    printf("password: %s\n", password);
    printf("delay: %d\n", delay);
    printf("operation: %d\n", operation);
    printf("args: %s\n", args);

    mkfifo(SERVER_FIFO_PATH,0660);

    sem=sem_open(SEM_NAME, O_CREAT, 0770);

    fd1=open(SERVER_FIFO_PATH, O_WRONLY);


    printf("PARENT:\n");
    close(fd1);
      
    do {
        fd2=open(fifoName, O_RDONLY);
        if (fd2 == -1) sleep(1);
    } while (fd2 == -1);

    close(fd2);
    return 0;
}
