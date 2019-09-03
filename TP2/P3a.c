#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#define BUFFER_SIZE 512

int main(int argc, char *argv[])
{
int fdsource, nr, nw;
unsigned char buffer[BUFFER_SIZE];
if (argc != 2) {
 printf("You need to use %d more argument(s) \n", (argc-3));
 return 1;
 }
 fdsource = open(argv[1], O_RDONLY);
if (fdsource == -1) {
 perror(argv[1]);
 return 2;
 }

while ((nr = read(fdsource, buffer, BUFFER_SIZE)) > 0)
{
    if ((nw = write(STDOUT_FILENO, buffer, nr)) <= 0 || nw != nr) 
    {
        perror(argv[2]);
        close(fdsource);
        return 4;
    }
}
 
 close(fdsource);
 return 0;
} 