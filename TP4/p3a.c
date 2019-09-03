#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int value=1;
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
    int counter=0;
   while(1)
   {
       sleep(1);
        counter+=value;
       printf("%d seconds \n", counter);
   }
    exit(0);
} 