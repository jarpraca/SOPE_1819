#include "server.h"

struct bank_account accounts[MAX_BANK_ACCOUNTS];

int main(int argc, char *argv[])
{
    int fd1, fd2;
    char fifoName[]="/tmp/secure_";
    char pid[WIDTH_ID + 1];
    pthread_t threads[atoi(argv[1])];

    if (argc < 2)
      {
        printf("Insufficient number of arguments\n");
        return 1;
      }
    else{
      int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
      logDelay(logfile, 0,0);

      if(*argv[1] < 1 || atoi(argv[1]) > MAX_BANK_OFFICES)
        return 1; 

      int id[atoi(argv[1])];
      for(int i = 1; i <= atoi(argv[1]); i++){
        id[i-1]=i;
        pthread_create(&threads[i-1], NULL, bankOffice, &id[i-1]);
      }

      create_admin_account(argv[2]);
    }
    
    printf("id: %d\n", accounts[0].account_id);
    printf("balance: %d\n", accounts[0].balance);
    printf("salt: %s\n", accounts[0].salt);
    printf("hash: %s\n", accounts[0].hash);

    do{
      do {
          fd1=open("/tmp/secure_srv", O_RDONLY);
          if (fd1 == -1) sleep(1);
      } while (fd1 == -1);
        
      read(fd1, pid, sizeof(pid));
      close(fd1);
      strcat(fifoName, pid);
      printf("fifoName: %s\n", fifoName);
      
      mkfifo(fifoName,0660);
      fd2= open(fifoName, O_WRONLY);
      printf("SON: Calculating...\n");
    
      close(fd2);

    } while(true);
      
      // for(int i = 1; i <= *argv[1]; i++){
      //   pthread_join(threads[i-1], NULL);
      // }
    return 0; 
}

char * generate_salt(){
  char characters[] = "0123456789abcdef";
  char *salt;
  salt = malloc(SALT_LEN + 1);

  srand(time(0));

  for(int i = 0; i < 64; i++){
    salt[i] = characters[rand() % 16];
  }

  return salt;
}

bool id_in_use(uint32_t id){
  return &accounts[id] != NULL;
}

void* bankOffice(void * arg)
{
  bankOfficeOpen(*(int*)arg);
  int id = *(int *)arg;
  printf("id: %d, tid: %ld \n", id, pthread_self());
  bankOfficeClose(*(int*)arg);
  return NULL;
}

void bankOfficeOpen(int id)
{
  int logfile;
  logfile = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logBankOfficeOpen(logfile, id, pthread_self());
  close(logfile);
}

void bankOfficeClose(int id)
{
  int logfile;
  logfile = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logBankOfficeClose(logfile, id, pthread_self());
  close(logfile);
}

int create_account(uint32_t id, char *password, uint32_t balance)
{
  struct bank_account new;

  new.account_id = id;
  new.balance = balance;
  char * salt;
  salt = generate_salt();
  strcpy(new.salt, salt);
  free(salt);

  char pass_salt[SALT_LEN + MAX_PASSWORD_LEN + 1];
  strcpy(pass_salt, password);
  strcat(pass_salt, new.salt);

  char *hash = getSha256(pass_salt);
  strcpy(new.hash, hash);
  free(hash);

  accounts[id] = new;

  return 0;
}

int create_admin_account(char *password){
  if (strlen(password) > MAX_PASSWORD_LEN + 1 || strlen(password) < MIN_PASSWORD_LEN + 1)
    return 1;

  return create_account(0, password, 0);
}

int create_user_account(uint32_t id, char *password, uint32_t balance){
  if(!id_in_use(id))
    return RC_ID_IN_USE;
  
  if (id < 1 || id >= MAX_BANK_ACCOUNTS)
    return 1;

  if (strlen(password) > MAX_PASSWORD_LEN + 1 || strlen(password) < MIN_PASSWORD_LEN + 1)
    return 1;

  if (balance < MIN_BALANCE || balance > MAX_BALANCE)
    return 1;

  return create_account(id, password, balance);
}

char *getSha256(char *password)
{
  pid_t pid1, pid2;
  int fd1[2], fd2[2];

  pipe(fd1);
  pipe(fd2);
  pid1 = fork();
  pid2 = fork();

  char *sha256;
  sha256 = malloc(sizeof(HASH_LEN + 1));

  if (pid1 == 0)
  {
    dup2(fd1[WRITE], STDOUT_FILENO);
    close(fd1[READ]);
    execlp("echo", "echo", "-n", password, NULL);
    fprintf(stderr, "Failed to execute sha256sum\n");
    exit(1);
  }
  else if (pid1 > 0)
  {
    int status;
    dup2(fd1[READ], STDIN_FILENO);
    close(fd1[WRITE]);
    
    if (pid2 == 0)
    {
      dup2(fd2[WRITE], STDOUT_FILENO);
      waitpid(pid1, &status, 0);
      execlp("sha256sum", "sha256sum", NULL);
      close(fd1[READ]);
    }
    else if (pid2 > 0)
    {
      int status2;
      close(fd2[WRITE]);
      read(fd2[READ], sha256, 64);
      waitpid(pid2, &status2, 0);
    }
  }

  close(fd1[WRITE]);
  close(fd1[READ]);
  close(fd2[WRITE]);
  close(fd2[READ]);
  return sha256;
}