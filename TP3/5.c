#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(void){

    
    pid_t pid, pid2;
    int* status=NULL, *status2=NULL;
    pid=fork();
   
    switch (pid) 
    {
        case -1:
            perror("fork");
            break;
        case 0: //filho
            pid2=fork();
            switch(pid2)
            {
                case -1:
                    perror("fork");
                    break;
                case 0:
                    write(STDOUT_FILENO,"Hello ", 7);
                    break;
                default:
                    waitpid(pid2,status,0);
                    write(STDOUT_FILENO,"my ",4);
                    break;
            }
            break;
        default: //pai
            waitpid(pid, status2,0);
            write(STDOUT_FILENO,"friends!", 9);
            break;            
    }
    return 0;
}