#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#define STDERR 2
#define NUMITER 10000

int numIter=NUMITER;
void * thrfunc(void * arg)
{
    int *iter;
    iter=malloc(sizeof(int));
    * iter=0;
    fprintf(stderr, "Starting thread %s\n", (char *) arg);
    while(numIter>0)
    {
        printf("%s",(char*)arg);
        numIter--;
        (*iter)++;
    }
    return iter;
}

int main()
{
    pthread_t ta, tb;
    char a='a';
    char b= 'b';
    int *c, *d;
    pthread_create(&ta, NULL, thrfunc, &a);
    pthread_create(&tb, NULL, thrfunc, &b);
    pthread_join(ta, (void**)&c);
    pthread_join(tb, (void**)&d);

    printf("Num iter thread 1: %d       Num iter thread 2: %d ", *c,*d);
    return 0;
}