#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h> 
#include <string.h>

int readline(int fd, char *str);
int main(void)
{
 int fd;
 char str[100];
 mkfifo("/tmp/myfifo",0660);
 fd=open("/tmp/myfifo", O_RDWR);
 while(readline(fd,str)) printf("%s",str);
 close(fd);
 return 0; return 0;

}
int readline(int fd, char *str)
{
 int n;
 do {
 n = read(fd,str,1);
 } while (n>0 && *str++ != '\0');
 return (n>0);
} 
