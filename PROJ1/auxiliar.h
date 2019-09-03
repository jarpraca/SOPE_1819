#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>

#define INPUT_END 1
#define OUTPUT_END 0

char* getFileInfo(char* filename);

bool isDirectory(char* filename);

void printAllDirectoryInfo(char *root, char* outfile, int fdLog);

