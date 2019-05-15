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

void getAccountArgs(char *args, char *acc_args[])
{
    printf("dsakdal \n");
    char *id = NULL;

    printf("XXXXXXXXXXXXXXXaquiXXXXXXXXXXXXXXXXXXXXXXXXXx");
    char * balance = NULL;
    char * password = NULL;
    printf("XXXXXXXXXXXXXXXaquiXXXXXXXXXXXXXXXXXXXXXXXXXx");
    for(int i=0; i < strlen(args); i++){
        printf("%d", i);
        if(args[i] == ' ')
            break;
        id[i]=args[i];
    }

    for(int i=strlen(id); i < strlen(args); i++){

         if(args[i] == ' ')
             break;
         balance[i-strlen(id)]=args[i];
    }
    
    for(int i=(strlen(id)+strlen(balance)); i < strlen(args); i++){

        if(args[i] == ' ')
            break;
        password[strlen(id)+strlen(balance)]=args[i];
    }

    acc_args[0] = id;
    acc_args[1]= password;
    acc_args[2] = balance;
}

void getTransferArgs(char* args, char* acc_args[])
{
 char* id = NULL;
 char* amount = NULL;
    for(int i=0; i < strlen(args); i++){

        if(args[i] == ' ')
            break;
        id[i]=args[i];
    }

    for(int i=strlen(id); i < strlen(args); i++){

        if(args[i] == ' ')
            break;
        amount[i-strlen(id)]=args[i];
    }

    acc_args[0] = id;
    acc_args[1] = amount;
}

int main(int argc, char *argv[])
{
    int fd1, fd2;
    pid_t pidN = getpid();
    char fifoName[USER_FIFO_PATH_LEN];
    char pid[WIDTH_ID+1];

    if (argc < 6){
        printf("Insufficient number of arguments\n");
        return 1;
    }

    int id = atoi(argv[1]);
    char *password = argv[2];
    int delay = atoi(argv[3]);

    if (id < 0 || id > 4095)
    {
        printf("id must be a number between 0 and 4095\n");
        return 1;
    }

    if (strlen(password) < MIN_PASSWORD_LEN || strlen(password) > MAX_PASSWORD_LEN + 1 )
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

    fd1=open(SERVER_FIFO_PATH, O_WRONLY);

    req_header_t req_header;

    req_header.pid = pidN;
    req_header.account_id = id;
    req_header.op_delay_ms = delay;
    strcpy(req_header.password, password);
    
    req_value_t req_value;

    req_value.header = req_header;

    if(operation == 0){

        // req_create_account_t account;
        
        char** acc_args = NULL;
        getAccountArgs(argv[5], acc_args);
        //account.account_id = *(int*)acc_args[0];
        // account.balance = *(int*)acc_args[1];
        // strcpy(account.password,acc_args[2]);
        //req_value.create = account;
    }
    else if (operation == 2){

        req_transfer_t transfer;
        char **transf_args = NULL;
        getTransferArgs(argv[5], transf_args);
        transfer.account_id = *(int*)transf_args[0];
        transfer.amount = *(int*)transf_args[1];
        req_value.transfer = transfer;
    }

    tlv_request_t request;

    request.type = operation;
    request.value = req_value;
    request.length = sizeof(req_value);

    write(fd1, &request, sizeof(request));
    close(fd1);
      
    do {
        fd2=open(fifoName, O_RDONLY);
        if (fd2 == -1) sleep(1);
    } while (fd2 == -1);

    close(fd2);
    return 0;
}

