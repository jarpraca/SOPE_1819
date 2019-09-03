#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h> 
#include <string.h>

#define READ 0
#define WRITE 1

int main(void)
{
    int fd, fd1;
    mkfifo("/tmp/fiforeq",0660);
    char num1[10];
    char num2[10];

    do{
    fd=open("/tmp/fiforeq", O_WRONLY);
    printf("x y ? ");
    scanf("%s %s", num1, num2);
    write(fd, num1, sizeof(num1));
    write(fd, num2, sizeof(num2));
    close(fd);
    int sum;
    int diff;
    int mult;
    int di;
    do {
        fd1=open("/tmp/fifores", O_RDONLY);
        if (fd1==-1) sleep(1);
    } while (fd1==-1);    read(fd, &sum, sizeof(sum));
    read(fd, &diff, sizeof(diff));
    read(fd, &mult, sizeof(mult));
    read(fd, &di, sizeof(di));

    printf("Parent:  \n");
    printf("Sum: %d\n", sum);
    printf("Diff: % d\n", diff);
    printf("Mult: %d\n", mult);
    printf("Di: %d\n", di);
    close(fd1);
    } while(num1 != "0" || num2 != "0");
    return 0; 

}
