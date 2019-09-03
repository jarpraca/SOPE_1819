#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void sigint_handler(int signo)
{
 printf("In SIGINT handler ...\n");
}


int main(void)
{
    struct sigaction action = {.sa_handler = sigint_handler, .sa_flags=0};
    if (sigaction(SIGINT,&action, NULL) < 0)
    {
        fprintf(stderr,"Unable to install SIGINT handler\n");
        exit(1);
    }

    raise(SIGINT);
    //raise(SIGUSR1);
    printf("Sleeping for 30 seconds ...\n");
    int number = 30;
    while(number!=0)
        number =sleep(number);
    printf("Waking up ...\n");
    exit(0);
} 