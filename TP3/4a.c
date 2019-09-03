#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(void){

    
    pid_t pid;
    int* status=NULL;
    pid=fork();
    switch (pid) 
    {
        case -1:
            perror("fork");
            break;
        case 0: //filho
            write(STDOUT_FILENO,"Hello ", 7);
            break;
        default: //pai
            wait(status);
            write(STDOUT_FILENO,"world",6);
            break;
            
    }
    return 0;
}