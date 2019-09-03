#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/shm.h> 

#define MAXELEMS 10000000 // nr. max de posicoes
#define MAXTHREADS 100 // nr. max de threads
#define min(a, b) (a)<(b)?(a):(b)
int npos;
int buf[MAXELEMS], pos=0, val=0; // variaveis partilhadas
sem_t sem;

void *fill(void *nr)
{
    while (1) {
        sem_wait(&sem);
        if (pos >= npos) {
            return NULL;
        }
        buf[pos] = val;
        pos++; 
        val++;
        *(int *)nr += 1;
        sem_post(&sem);
    }
}
void *verify(void *arg)
{
    int k;
    for (k=0; k<npos; k++)
    if (buf[k] != k) // detecta valores errados
    printf("ERROR: buf[%d] = %d\n", k, buf[k]);
    return NULL;
}

int main(int argc, char *argv[])
{
    sem_init(&sem, 0, 1);
    
    int k, nthr, count[MAXTHREADS]; // array para contagens
    int total;
    if (argc != 3) {
        printf("Usage: %s <nr_pos> <nr_thrs>\n",argv[0]);
        return 1;
    } 
    npos = min(atoi(argv[1]), MAXELEMS); //no. efectivo de posicoes
    nthr = min(atoi(argv[2]), MAXTHREADS); //no. efectivo de threads
    pid_t pids[MAXTHREADS];
    int *status;
    for (k=0; k<nthr; k++) { // criacao das threads 'fill'
        count[k] = 0;
        pids[k]=fork();
        switch(pids[k])
        {
            case 0:
                fill(&count[k]);
                break;
            default:
                break;
        }
    }
    total=0;
    for (k=0; k<nthr; k++) { //espera threads 'fill'
        waitpid(pids[k], status, 0);
        printf("count[%d] = %d\n", k, count[k]);
        total += count[k];
    }
    printf("total count = %d\n",total); // mostra total
    pid_t pid;
    pid=fork();
    switch(pid)
    {
        case 0:
            verify(NULL);
            break;
        default:
        {
            int *status;
            waitpid(pid,status, 0);
            break;
        }
    }
    sem_destroy(&sem);
    return 0;
} 