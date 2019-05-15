#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <pthread.h>
#include <semaphore.h>

#include "log.c"

#define READ 0
#define WRITE 1

char *generate_salt();

bool id_in_use(uint32_t id);

void * bankOffice(void * arg);

void bankOfficeOpen(int id);

void bankOfficeClose(int id);

int create_account(uint32_t id, const char *password, uint32_t balance);

int create_admin_account(const char *password);

int create_user_account(uint32_t id, const char *password, uint32_t balance);

char *getSha256(char *filename);