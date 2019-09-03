#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h> 
#include <string.h>
#include <stdlib.h>

#define READ 0
#define WRITE 1

int main(void)
{
    int fd, fd1;
    char num3[10];
    char num4[10];
    do{
     do {
        fd=open("/tmp/fiforeq", O_RDONLY);
        if (fd==-1) sleep(1);
    } while (fd==-1);
      

    read(fd, num3, sizeof(num3));
    read(fd, num4, sizeof(num4));
    close(fd);
    if(num3 == 0 && num4 == 0)
        break;
      mkfifo("/tmp/fifores",0660);
      fd1= open("/tmp/fifores", O_WRONLY);
    printf("SON: Calculating...\n");
    int sum = atoi(num3) + atoi(num4);
    int diff = atoi(num3) - atoi(num4);
    int mult = atoi(num3) * atoi(num4);
    int di = atoi(num3) / atoi(num4);
    write(fd, &sum, sizeof(sum));
    write(fd, &diff, sizeof(diff));
    write(fd, &mult, sizeof(mult));
    write(fd, &di, sizeof(di));
   
    close(fd1);
    } while(num3 != 0 || num4 != 0);
    return 0; 
}
    
 