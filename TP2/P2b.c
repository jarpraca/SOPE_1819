#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#define BUFFER_SIZE 512

int main(int argc, char *argv[])
{
int fdsource, fdestination, nr, nw;
unsigned char buffer[BUFFER_SIZE];
if (argc != 3) {
 printf("You need to use %d more argument(s) \n", (argc-3));
 return 1;
 }
 fdsource = open(argv[1], O_RDONLY);
if (fdsource == -1) {
 perror(argv[1]);
 return 2;
 }
 fdestination = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, 0644);
if (fdestination == -1) {
 perror(argv[2]);
 close(fdestination);
 return 3;
 }
while ((nr = read(fdsource, buffer, BUFFER_SIZE)) > 0)
{
    if ((nw = write(fdestination, buffer, nr)) <= 0 || nw != nr) 
    {
        perror(argv[2]);
        close(fdsource);
        close(fdestination);
        return 4;
    }
}
 
 close(fdsource);
 close(fdestination);
 return 0;
} 