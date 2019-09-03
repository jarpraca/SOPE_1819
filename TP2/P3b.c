#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#define BUFFER_SIZE 512

int main(int argc, char *argv[])
{
    int fdsource, fdestination, nr, nw;
    unsigned char buffer[BUFFER_SIZE];
    if ((argc != 2) && (argc != 3)) 
    {
        printf("Wrong number of argument(s) \n");
        return 1;
    }
    fdsource = open(argv[1], O_RDONLY);
    if (fdsource == -1) 
    {
        perror(argv[1]);
        return 2;
    }

    if(argc==3)
    {
        fdestination = open(argv[2], O_WRONLY | O_CREAT , 0644);
        if (fdestination == -1) 
        {
            perror(argv[2]);
            close(fdestination);
            return 3;
        }
        dup2(fdestination, STDOUT_FILENO);
    }
    while ((nr = read(fdsource, buffer, BUFFER_SIZE)) > 0)
    {
        if ((nw = write(STDOUT_FILENO, buffer, nr)) <= 0 || nw != nr) 
        {
            perror(argv[2]);
            close(fdsource);
            return 3;
        }   
    }
    close(fdsource);
    return 0;
} 