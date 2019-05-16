#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <semaphore.h>
#include <stdlib.h>
#include <errno.h>
#include "sope.h"

#define READ 0
#define WRITE 1

void getAccountArgs(char *args, char *acc_args[]);

void getTransferArgs(char *args, char *acc_args[]);