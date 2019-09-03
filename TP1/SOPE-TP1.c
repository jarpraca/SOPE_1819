#include <stdio.h>
#include<errno.h>
#include <stdlib.h>   
#include <string.h>

// #define MAX_LENGTH 100
// int main(int argc, char* argv[], char* envp[])
// {
//   //P5 a)
//   // int i=0; 
//   // while(envp[i]!=NULL)
//   // {
//   //   printf("%s \n", envp[i]);
//   //   i++;
//   // }

//   //P5 b)

//   // if(argc==1)
//   // {
//   //   char* subbuff= malloc(MAX_LENGTH);
//   //   memcpy(subbuff, &envp[11][9], strlen(envp[11])-9);
//   //   printf("Hello %s ! \n", subbuff);
//   // }

//   //P5 c) d)
//   if(argc==1)
//   {
//    // printf("Hello %s ! \n", getenv("USERNAME"));
//     printf("Hello %s ! \n", getenv("USER_NAME"));
//   }
  
//   //P4
//   //int n, k=0;
//   // while(k!= atoi(argv[1]))
//   // {
//   //   n=2;
//   //   printf("Hello ");
//   //   while(n!=argc)
//   //   {
//   //     printf( "%s " , argv[n]);
//   //     n++;
//   //   }
//   //   printf("! \n");
//   //   k++;
//   // }
//  return 0;
// } 

#include<errno.h>
#include <stdio.h>
#define BUF_LENGTH 256
#define MAX 512
int main(void)
{
  FILE *src, *dst;
  char buf[BUF_LENGTH];
  if ( ( src = fopen( "infile.txt", "r" ) ) == NULL )
  {
    perror("Error");
    exit(1);
  }
  if ( ( dst = fopen( "outfile.txt", "w" ) ) == NULL )
  {
    perror("Error");
    exit(2);
  } 
  while( ( fgets( buf, MAX, src ) ) != NULL )
  {
    fputs( buf, dst );
  }
  fclose( src );
  fclose( dst );
  exit(0); 
}