#include "server.h"

bank_account_t accounts[MAX_BANK_ACCOUNTS];

int main(int argc, char *argv[])
{
  mkfifo(SERVER_FIFO_PATH,O_RDONLY);
    if (argc < 2)
    {
      printf("Insufficient number of arguments\n");
      return 1;
    }

    // int fd1, fd2;
    // char fifoName[]="/tmp/secure_";
    // char pid[WIDTH_ID + 1];

    pthread_t threads[atoi(argv[1])];

    int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    logDelay(logfile, 0,0);
    close(logfile);

    if(*argv[1] < 1 || atoi(argv[1]) > MAX_BANK_OFFICES)
      return 1; 

    int id[atoi(argv[1])];
    for(int i = 1; i <= atoi(argv[1]); i++)
    {
      id[i-1]=i;
      pthread_create(&threads[i-1], NULL, bankOffice, &id[i-1]);
    }

    for(int i = 1; i <= atoi(argv[1]); i++)
      pthread_join(threads[i-1], NULL);

    create_admin_account(argv[2]);
    
    printf("id: %d\n", accounts[0].account_id);
    printf("balance: %d\n", accounts[0].balance);
    printf("salt: %s\n", accounts[0].salt);
    printf("hash: %s\n", accounts[0].hash);
        
      // read(fd1, pid, sizeof(pid));
      // close(fd1);
      // strcat(fifoName, pid);
      // printf("fifoName: %s\n", fifoName);
      
      // mkfifo(fifoName,0660);
      // fd2= open(fifoName, O_WRONLY);
    
      // close(fd2);
      // unlink(fifoName);

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
  return (&accounts[id] != NULL);
}

int authenticate(uint32_t accountID, const char password[])
{
  char* pass_salt = NULL;
  if(!id_in_use(accountID))
    return RC_ID_NOT_FOUND;
  strcpy(pass_salt, password);
  strcpy(pass_salt, accounts[accountID].salt);
  if(getSha256(pass_salt)==accounts[accountID].hash)
    return RC_OK;
  else
    return RC_LOGIN_FAIL;
}

int processRequest(int operation, const req_value_t* request)
{
  uint32_t accountID = request->header.account_id;
  if(operation == OP_CREATE_ACCOUNT)
  {
    if(accountID != 0)
      return RC_OP_NALLOW;
    if(authenticate(accountID, request->header.password)==RC_OK)
    {
      create_user_account(request->create.account_id, request->create.password, request->create.balance);
      int logfile = open(SERVER_LOGFILE, O_WRONLY, 0644);
      logAccountCreation(logfile, pthread_self(), &accounts[accountID]);
      close(logfile);
    }
    else
      return RC_LOGIN_FAIL;
  }
  if(operation == OP_BALANCE)
  {
    if(accountID == 0)
      return RC_OP_NALLOW;
    if(authenticate(accountID, request->header.password)==RC_OK)
    {
        int logfile = open(SERVER_LOGFILE, O_WRONLY, 0644);
        close(logfile);
    }
  }
  if(operation == OP_TRANSFER)
  {
    if(accountID == 0)
      return RC_OP_NALLOW;
  }
  if(operation == OP_SHUTDOWN)
  {
    if(accountID != 0)
      return RC_OP_NALLOW;
  }
  return RC_OK;
}
void* bankOffice(void * arg)
{
  sem_t* sem;

  int id = *(int *)arg;
  char *operation = NULL;
  uint32_t* length = 0;
  req_value_t* value = NULL;
  bankOfficeOpen(id);
  int fd;
  sem= sem_open(SEM_NAME,0,0600,0);
   if(sem == SEM_FAILED)
  {
    perror("Server failed in sem_open()");
    exit(RC_OTHER);
  } 
  sem_wait(sem);
  do{
    fd=open(SERVER_FIFO_PATH, O_RDONLY);
    if(fd==-1) sleep(1);
  } while(fd == -1);
  
  read(fd, operation, sizeof(op_type_t));
  read(fd, length, sizeof(uint32_t));
  read(fd, value, sizeof(*length));
  sem_close(sem);
  processRequest(atoi(operation), value);
  bankOfficeClose(id);
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

int create_account(uint32_t id, const char *password, uint32_t balance)
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

int create_admin_account(const char *password){
  if (strlen(password) > MAX_PASSWORD_LEN + 1 || strlen(password) < MIN_PASSWORD_LEN + 1)
    return 1;

  return create_account(0, password, 0);
}

int create_user_account(uint32_t id, const char *password, uint32_t balance){
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
    fprintf(stderr, "Failed to execute echo\n");
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
      close(fd1[READ]);
      execlp("sha256sum", "sha256sum", NULL);
      fprintf(stderr, "Failed to execute echo\n");
      exit(1);
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