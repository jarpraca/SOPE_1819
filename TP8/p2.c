#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#define STDERR 2
#define NUMITER 10000

int numIter=NUMITER;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
void * thrfunc(void * arg)
{
    int *iter;
    iter=malloc(sizeof(int));
    * iter=0;
    fprintf(stderr, "Starting thread %s\n", (char *) arg);
    while(numIter>1)
    {
        pthread_mutex_lock(&mutex);
        printf("%s",(char*)arg);
        numIter--;
        (*iter)++;
        pthread_mutex_unlock(&mutex);

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
    pthread_mutex_destroy(&mutex);
    return 0;
}