#include "auxiliar.h"

// Extern variables
extern bool md5;
extern bool sha1;
extern bool sha256;
extern bool var_r;
extern bool var_v;
extern bool out;
int file=0;
int directory=0;
bool newDir=false;
extern clock_t initial_time;
extern int cout;

void printLog(int fd, char* fileName)
{
    char *buffer;
    char *aux;
    buffer = malloc(sizeof(1024));
    aux= malloc(sizeof(1024));
    clock_t sec = clock() - initial_time;
    sprintf(aux, "%0.2f", (((float)sec) / CLOCKS_PER_SEC) * 1000);
    strcat(buffer, aux);
    sprintf(aux , " - %d", getpid());
    strcat(buffer, aux);
    strcat(buffer, " - ANALIZED ");
    strcat(buffer, fileName);
    strcat(buffer, "\n");
    int j=0;
    while(buffer[j]!= '\n')
    {
        char aux2[2];
        aux2[0] = buffer[j];
        aux2[1]= '\0';
        write(fd, aux2, 1);    }

        j++;
    }
    write(fd, "\n", 1);
}

char *getFileType(char *filename)
{
    pid_t pid;
    int fd[2];
    pipe(fd);
    pid = fork();

    if (pid == 0)
    {
        dup2(fd[INPUT_END], STDOUT_FILENO);
        char *arg[] = {"file", "-b", filename, NULL};
        execvp("file", arg);
        fprintf(stderr, "Failed to execute stat \n");
        exit(1);
    }
    else
    {
        char *auxBuf;
        auxBuf = malloc(sizeof(1024));
        int status;
        read(fd[OUTPUT_END], (void *)auxBuf, 1024);
        close(fd[INPUT_END]);
        close(fd[OUTPUT_END]);
        waitpid(pid, &status, 0);
        strtok(auxBuf, "\n");
        return auxBuf;
    }
}

char *getFileSize(char *filename)
{
    pid_t pid;
    int fd[2];
    pipe(fd);
    pid = fork();

    if (pid == 0)
    {
        dup2(fd[INPUT_END], STDOUT_FILENO);
        char *arg[] = {"stat", "-c", "%s", filename, NULL};
        execvp("stat", arg);
        fprintf(stderr, "Failed to execute stat \n");
        exit(1);
    }
    else
    {
        char *auxBuf;
        auxBuf = malloc(sizeof(1024));
        int status;
        read(fd[OUTPUT_END], (void *)auxBuf, 1024);
        close(fd[INPUT_END]);
        close(fd[OUTPUT_END]);
        waitpid(pid, &status, 0);
        strtok(auxBuf, "\n");
        return auxBuf;
    }
}

char *getFileAccess(char *filename)
{
    pid_t pid;
    int fd[2];
    pipe(fd);
    pid = fork();

    if (pid == 0)
    {
        dup2(fd[INPUT_END], STDOUT_FILENO);
        char *arg[] = {"stat", "-c", "%A", filename, NULL};
        execvp("stat", arg);
        fprintf(stderr, "Failed to execute stat \n");
        exit(1);
    }
    else
    {
        char *auxBuf;
        char *access;
        auxBuf = malloc(sizeof(1024));
        access = malloc(sizeof(1024));
        int status;
        read(fd[OUTPUT_END], (void *)auxBuf, 1024);
        close(fd[INPUT_END]);
        close(fd[OUTPUT_END]);
        waitpid(pid, &status, 0);
        strncpy(access, auxBuf + 1, 2);
        //free(auxBuf);
        return access;
    }
}

char *getFileAccessDate(char *filename)
{
    pid_t pid;
    int fd[2];
    pipe(fd);
    pid = fork();

    if (pid == 0)
    {
        dup2(fd[INPUT_END], STDOUT_FILENO);
        char *arg[] = {"stat", "-c", "%x", filename, NULL};
        execvp("stat", arg);
        fprintf(stderr, "Failed to execute stat \n");
        exit(1);
    }
    else
    {
        char *auxBuf;
        char *date;
        char *time;
        auxBuf = malloc(sizeof(1024));
        date = malloc(sizeof(1024));
        time = malloc(sizeof(1024));
        int status;
        read(fd[OUTPUT_END], (void *)auxBuf, 1024);
        close(fd[INPUT_END]);
        close(fd[OUTPUT_END]);
        waitpid(pid, &status, 0);
        strncpy(date, auxBuf, 10);
        strncpy(time, auxBuf + 11, 8);
        auxBuf = date;
        strcat(auxBuf, "T");
        strcat(auxBuf, time);
        //free(date);
        //free(time);
        return auxBuf;
    }
}

char *getFileModificationDate(char *filename)
{
    pid_t pid;
    int fd[2];
    pipe(fd);
    pid = fork();

    if (pid == 0)
    {
        dup2(fd[INPUT_END], STDOUT_FILENO);
        char *arg[] = {"stat", "-c", "%y", filename, NULL};
        execvp("stat", arg);
        fprintf(stderr, "Failed to execute stat \n");
        exit(1);
    }
    else
    {
        char *auxBuf;
        char *date;
        char *time;
        auxBuf = malloc(sizeof(1024));
        date = malloc(sizeof(1024));
        time = malloc(sizeof(1024));
        int status;
        read(fd[OUTPUT_END], (void *)auxBuf, 1024);
        close(fd[INPUT_END]);
        close(fd[OUTPUT_END]);
        waitpid(pid, &status, 0);
        strncpy(date, auxBuf, 10);
        strncpy(time, auxBuf + 11, 8);
        auxBuf = date;
        strcat(auxBuf, "T");
        strcat(auxBuf, time);
        //free(date);
        //free(time);
        return auxBuf;
    }
}

bool isDirectory(char *filename)
{
    pid_t pid;
    int fd[2];
    pipe(fd);
    pid = fork();

    if (pid == 0)
    {
        dup2(fd[INPUT_END], STDOUT_FILENO);
        char *arg[] = {"stat", "-c", "%F", filename, NULL};
        execvp("stat", arg);
        fprintf(stderr, "Failed to execute stat \n");
        exit(1);
    }
    else
    {
        char *auxBuf;
        auxBuf = malloc(sizeof(1024));
        int status;
        read(fd[OUTPUT_END], (void *)auxBuf, 1024);
        close(fd[INPUT_END]);
        close(fd[OUTPUT_END]);
        waitpid(pid, &status, 0);
        return (strstr(auxBuf, "directory") != NULL);
    }
}

char *getMd5(char *filename)
{
    pid_t pid;
    int fd[2];

    pipe(fd);
    pid = fork();

    char *auxBuf;
    char *md5;
    auxBuf = malloc(sizeof(1024));
    md5 = malloc(sizeof(1024));

    if (pid == 0)
    {
        dup2(fd[INPUT_END], STDOUT_FILENO);
        char *arg[] = {"md5sum", filename, NULL};
        execvp("md5sum", arg);
        fprintf(stderr, "Failed to execute md5sum \n");
        exit(1);
    }
    else
    {
        int status;
        read(fd[OUTPUT_END], (void *)auxBuf, 1024);
        strncpy(md5, auxBuf, 32);
        close(fd[INPUT_END]);
        close(fd[OUTPUT_END]);
        waitpid(pid, &status, 0);
        //free(auxBuf);
    }
    return md5;
}

char *getSha1(char *filename)
{
    pid_t pid;
    int fd[2];

    pipe(fd);
    pid = fork();

    char *auxBuf;
    char *sha1;
    auxBuf = malloc(sizeof(1024));
    sha1 = malloc(sizeof(1024));

    if (pid == 0)
    {
        //close(fd[OUTPUT_END]);
        dup2(fd[INPUT_END], STDOUT_FILENO);
        char *arg[] = {"sha1sum", filename, NULL};
        execvp("sha1sum", arg);
        fprintf(stderr, "Failed to execute sha1sum \n");
        exit(1);
    }
    else
    {
        int status;
        read(fd[OUTPUT_END], (void *)auxBuf, 1024);
        strncpy(sha1, auxBuf, 40);
        close(fd[INPUT_END]);
        close(fd[OUTPUT_END]);
        waitpid(pid, &status, 0);
        //free(auxBuf);
    }
    return sha1;
}

char *getSha256(char *filename)
{
    pid_t pid;
    int fd[2];

    pipe(fd);
    pid = fork();

    char *auxBuf;
    char *sha256;
    auxBuf = malloc(sizeof(1024));
    sha256 = malloc(sizeof(1024));

    if (pid == 0)
    {
        //close(fd[OUTPUT_END]);
        dup2(fd[INPUT_END], STDOUT_FILENO);
        char *arg[] = {"sha256sum", filename, NULL};
        execvp("sha256sum", arg);
        fprintf(stderr, "Failed to execute sha256sum \n");
        exit(1);
    }
    else
    {
        int status;
        read(fd[OUTPUT_END], (void *)auxBuf, 1024);
        strncpy(sha256, auxBuf, 64);
        close(fd[INPUT_END]);
        close(fd[OUTPUT_END]);
        waitpid(pid, &status, 0);
        //free(auxBuf);
    }
    return sha256;
}

char *getFileInfo(char *filename)
{
    char *comma = ",";
    char *result;
    result = malloc(sizeof(1024));

    strcat(result, getFileType(filename));
    strcat(result, comma);
    strcat(result, getFileSize(filename));
    strcat(result, comma);
    strcat(result, getFileAccess(filename));
    strcat(result, comma);  
    strcat(result, getFileAccessDate(filename));
    strcat(result, comma);     
    strcat(result, getFileModificationDate(filename));
    if(md5 && !isDirectory(filename)){
        strcat(result, comma);
        strcat(result, getMd5(filename));
    }
    if (sha1 && !isDirectory(filename))
    {
        strcat(result, comma);
        strcat(result, getSha1(filename));
    }
    if (sha256 && !isDirectory(filename))
    {
        strcat(result, comma);
        strcat(result, getSha256(filename));
    }
    strcat(result, "\n");
     
    return result;
}

void sigusr_handler(int signo)
{
    if(signo==SIGUSR1)
    {
        directory+=1;
        newDir=true;
    }    
    else
    {
        file+=1;
        newDir=false;
    }
}

void sendSignal(int signal, int fdLog, int fd)
 {
    char buffer1[]= "SIGUSR1";
    char buffer2[]="SIGUSR2";
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
    pid=fork();
    switch(pid)
    {
        case 0: 
        {
            if(newDir)
                {
                    char new[52];
                    sprintf(new, "New Directory: %d/%d directories/files at this time \n", directory, file);
                    dup2(cout, STDOUT_FILENO);
                    int j = 0;
                    while (new[j] != '\n')
                    {
                        char aux2[2];
                        aux2[0] = new[j];
                        aux2[1] = '\0';
                        write(STDOUT_FILENO, aux2, 1);
                        j++;
                    }
                    write(STDOUT_FILENO, "\n", 1);
                    dup2(fd, STDOUT_FILENO);
                }
            // else {
            //         char new[52];
            //         sprintf(new, "New File: %d/%d directories/files at this time \n", directory, file);
            //         dup2(cout, STDOUT_FILENO);
            //         int j = 0;
            //         while (new[j] != '\n')
            //         {
            //             char aux2[2];
            //             aux2[0] = new[j];
            //             aux2[1] = '\0';
            //             write(STDOUT_FILENO, aux2, 1);
            //             j++;
            //         }
            //         write(STDOUT_FILENO, "\n", 1);
            //         dup2(fd, STDOUT_FILENO);
            // }
            break;
        }
        default:
        {
            if(signal==1)
            {
                if(var_v)
                {
                    char *aux;
                    char *aux2;
                    aux= malloc(sizeof(22));
                    aux2= malloc(sizeof(22));
                    clock_t sec = clock() - initial_time;
                    sprintf(aux, "%0.2f", (((float)sec) / CLOCKS_PER_SEC) * 1000);
                    strcat(aux2, aux);
                    sprintf(aux , " - %d - ", getpid());
                    strcat(aux2, aux);
                    strcat(aux2, buffer1);
                    write(fdLog, aux2,21);
                }
                kill(pid, SIGUSR1);
            }
            else if(signal ==2)
            {
                if(var_v)
                {
                    char *aux;
                    char *aux2;
                    aux= malloc(sizeof(22));
                    aux2= malloc(sizeof(22));
                    clock_t sec = clock() - initial_time;
                    sprintf(aux, "%0.2f", (((float)sec) / CLOCKS_PER_SEC) * 1000);
                    strcat(aux2, aux);
                    sprintf(aux , " - %d - ", getpid());
                    strcat(aux2, aux);
                    strcat(aux2, buffer2);
                    write(fdLog, aux2,21);
                    
                }
                kill(pid, SIGUSR2);
            }
            int status;
            waitpid(pid,&status,0);
            break;
        }
    }
     oc(sizeof(22));
                    aux2= malloc(sizeof(22));
     
}

void printAllDirectoryInfo(char *root, char* outfile, int fdLog)
{
    char path[1024] = "./";
    strcat(path,root);
    struct dirent *dp;
    DIR *dir;
    int fd;
    if (outfile != NULL)
    {
        fd = open(outfile, O_WRONLY | O_CREAT, 0644);
        dup2(fd, STDOUT_FILENO);
    }
    if(isDirectory(root))
        dir = opendir(root);
    else
    {
        if(var_v)
            printLog(fdLog, root);
        if(out)
        {
            sendSignal(2, fdLog, fd);
        }
        printf("%s,%s", root, getFileInfo(root));
        if(outfile != NULL)
            close(fd);
        return;
    }
    pid_t pid;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            if(var_v)
                printLog(fdLog, dp->d_name);
            printf("%s,%s", dp->d_name, getFileInfo(dp->d_name));
            fflush(stdout);
            if (isDirectory(dp->d_name) && var_r)
            {   
                if(out)
                    sendSignal(1, fdLog, fd);             
                pid = fork();

                if(pid==0)
                {
                    strcat(strcat(path, "/"), dp->d_name);
                    printAllDirectoryInfo(path, outfile, fdLog);
                    break;
                }
                else
                {
                    int status;
                    waitpid(pid, &status, 0);
                }           
            }
            else if(out)
            {
                if(isDirectory(dp->d_name))
                    sendSignal(1, fdLog, fd);
                else
                    sendSignal(2, fdLog, fd);
            }
                
        }
    }
    closedir(dir);
    if(outfile != NULL)
        close(fd);
}
