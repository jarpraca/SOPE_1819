#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <semaphore.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "log.c"

#define READ 0
#define WRITE 1

void getAccountArgs(char *args, req_create_account_t *account);

void getTransferArgs(char *args, req_transfer_t *transfer);