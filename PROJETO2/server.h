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
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include "log.c"
#include "queue.c"

#define READ 0
#define WRITE 1

char* generate_salt();

bool id_in_use(uint32_t id);

void * bankOffice(void * arg);

void bankOfficeOpen(int id);

void bankOfficeClose(int id);

int create_account(uint32_t id, const char *password, uint32_t balance);

int create_admin_account(const char *password);

int create_user_account(uint32_t id, const char *password, uint32_t balance);

char *getSha256(char *filename);

int authenticate(uint32_t accountID, const char password[]);

int mutex_unlock(pthread_mutex_t* mutex, int threadID, sync_role_t role, int pid);

int mutex_lock(pthread_mutex_t* mutex, int threadID, sync_role_t role, int pid);

int mutex_unlock_account(int threadID, sync_role_t role, int pid);

int mutex_lock_account(int threadID, sync_role_t role, int pid);

int logSemMech(int id, sync_mech_op_t sync_op, sync_role_t role, int pid, int val);

int logDelaySync(int threadID, int id, int delay_ms);

int sendReply(int pid, tlv_reply_t reply);