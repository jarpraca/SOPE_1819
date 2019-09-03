#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void sigint_handler(int signo)
{
    printf("Entering SIGINT handler ...\n");
    sleep(10);
    printf("Exiting SIGINT handler ...\n");
} 

void sigterm_handler (int signo)
{
    printf("In SIGTERM handler ...\n");
    printf("Leaving SIGTERM handler ...\n");
}

int main(void)
{
    struct sigaction action;
    action.sa_handler = sigint_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGINT,&action,NULL) < 0)
    {
    fprintf(stderr,"Unable to install SIGINT handler\n");
    exit(1);
    }
    struct sigaction action2 = {.sa_handler = sigterm_handler, .sa_flags=0};
    if (sigaction(SIGTERM,&action2, NULL) < 0)
    {
        fprintf(stderr,"Unable to install SIGTERM handler\n");
        exit(1);
    }
    printf("Try me with CTRL-C ...\n");
    while(1) pause();
    exit(0);
} 