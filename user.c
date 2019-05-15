#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h> 
#include <string.h>
#include <stdlib.h>
#include "sope.h"

#define READ 0
#define WRITE 1

int main(int argc, char *argv[])
{
    int fd1, fd2;
    pid_t pidN = getpid();
    char fifoName[]="/tmp/secure_";
    char pid[WIDTH_ID];

    if (argc < 5){
        printf("Insufficient number of arguments\n");
        return 1;
    }

    if (atoi(argv[1]) < 0 || atoi(argv[1]) > 4095)
    {
        printf("id must be a number between 0 and 4095\n");
        return 1;
    }

    int id = atoi(argv[1]);

    if (strlen(argv[2]) <= MIN_PASSWORD_LEN || strlen(argv[2]) > MAX_PASSWORD_LEN + 1 )
    {
        printf("password must have between 8 and 20 characters\n");
        return 1;
    }

    char * password = argv[2];

    if (atoi(argv[3]) < 0)
    {
        printf("delay cannot be a negative number\n");
        return 1;
    }

    int delay = atoi(argv[3]);

    if(atoi(argv[4]) < 0 || atoi(argv[4]) > 4){
        printf("operation must correspond to a number between 0 and 4\n");
        return 1;
    }

    int operation = atoi(argv[4]);

    char * args = argv[5];

    sprintf(pid, "%d", pidN);
    printf("pid: %s \n", pid);
    strcat(fifoName, pid);
    printf("fifoname: %s \n", fifoName);

    printf("\nid: %d\n", id);
    printf("password: %s\n", password);
    printf("delay: %d\n", delay);
    printf("operation: %d\n", operation);
    printf("args: %s\n", args);

    mkfifo("/tmp/secure_srv",0660);
    char num1[10];
    char num2[10];

    fd1=open("/tmp/secure_srv", O_WRONLY);

    do {
        fd2=open(fifoName, O_RDONLY);
        if (fd2 == -1) sleep(1);
    } while (fd2 == -1);

    printf("PARENT:\n");
    close(fd2);

    return 0;
}
