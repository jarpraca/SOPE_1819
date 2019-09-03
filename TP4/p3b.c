#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int value=1;
int sequence =0;
void sigusr_handler(int signo)
{
    if(signo==SIGUSR1)
        value=1;
    else
        value=-1;
}

int main(void)
{
    struct sigaction action;
    action.sa_handler = sigusr_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGUSR1,&action,NULL) < 0)
    {
    fprintf(stderr,"Unable to install SIGINT handler\n");
    exit(1);
    }
    if (sigaction(SIGUSR2,&action,NULL) < 0)
    {
    fprintf(stderr,"Unable to install SIGINT handler\n");
    exit(1);
    }
    pid_t pid;
    srand(time(NULL));
    switch(pid=fork())
    {
        case 0: 
        {
            sequence+=value;
            printf("%d seconds \n", sequence);
            break;
        }
        default:
        {
            int counter=0, val;
            while(counter<50)
            {
                sleep(1);
                val = rand()%2;
                if(val==0)
                    kill(pid, SIGUSR1);
                else
                    kill(pid, SIGUSR2);
            }
            break;
        }
    }

    exit(0);
} 