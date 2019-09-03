#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#define BUFFER_SIZE 512

int main(int argc, char *argv[])
{

FILE* source;
FILE* destination;
int nr, nw;
char* read="r";
char* write ="w";
unsigned char buffer[BUFFER_SIZE];
if (argc != 3) {
 printf("You need to use %d more argument(s) \n", (argc-3));
 return 1;
 }
 source = fopen(argv[1], read);
if (source == NULL) {
 perror(argv[1]);
 return 2;
 }
 destination = fopen(argv[2], write);
if (destination == NULL)
{
 perror(argv[2]);
 fclose(destination);
 return 3;
}
while ((nr = fread(buffer,1,BUFFER_SIZE,source)) > 0)
{ 
    if ((nw = fwrite(buffer,1, nr, destination)) <= 0 || nw != nr) 
    {
        perror(argv[2]);
        fclose(source);
        fclose(destination);
        return 4;
    }
}
 fclose(source);
 fclose(destination);
 return 0;
} 