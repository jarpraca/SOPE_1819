#include "server.h"

bank_account_t accounts[MAX_BANK_ACCOUNTS];

int slog;

int main(int argc, char *argv[])
{
  int shmfd, fdFIFO;
  char *shm, *s;

  if(mkfifo(SERVER_FIFO_PATH,0660)<0)
    if (errno==EEXIST) 
      printf("FIFO '/tmp/secure_srv' already exists\n");
    else 
      printf("Can't create FIFO\n"); 


    do {
    fdFIFO=open(SERVER_FIFO_PATH, O_RDONLY);
        if (fdFIFO == -1) sleep(1);
    } while (fdFIFO == -1);

  tlv_request_t request;
  read(fdFIFO,&request, sizeof(tlv_request_t));

  shmfd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);

  if (shmfd < 0)
  {
    perror("Server failure in shm_open()");
    exit(1);
  }

  if (ftruncate(shmfd, SHM_SIZE) < 0)
  {
    perror("Server failure in ftruncate()");
    exit(2);
  }

  shm = (char *)mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
  if (shm == MAP_FAILED)
  {
    perror("Server failure in mmap()");
    exit(3);
  }

  sem_t* sem;
  sem= sem_open(SEM_NAME, O_CREAT, 0660);

  if (argc < 2)
  {
    printf("Insufficient number of arguments\n");
    return 1;
  }

  pthread_t threads[atoi(argv[1])];

  int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logDelay(logfile, 0,0);
  close(logfile);

  if(*argv[1] < 1 || atoi(argv[1]) > MAX_BANK_OFFICES)
    return 1;

  s = shm;
  for (int i = 0; i < SHM_SIZE - 1; i++){

    *s++ = 1;
  }
  *s = (char)0;

  int id[atoi(argv[1])];
  for(int i = 1; i <= atoi(argv[1]); i++)
  {
    id[i-1]=i;
    pthread_create(&threads[i-1], NULL, bankOffice, &id[i-1]);
  }

  create_admin_account(argv[2]);

  printf("id: %d\n", accounts[0].account_id);
  printf("balance: %d\n", accounts[0].balance);
  printf("salt: %s\n", accounts[0].salt);
  printf("hash: %s\n", accounts[0].hash);

  for(int i = 1; i <= atoi(argv[1]); i++)
    pthread_join(threads[i-1], NULL);     

  close(fdFIFO);
  unlink(SERVER_FIFO_PATH);
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
  char pass_salt[MAX_PASSWORD_LEN+64];
  if(!id_in_use(accountID))
    return RC_ID_NOT_FOUND;
  strcpy(pass_salt, password);
  strcpy(pass_salt, accounts[accountID].salt);
  if(getSha256(pass_salt)==accounts[accountID].hash)
    return RC_OK;
  else
    return RC_LOGIN_FAIL;
}

int processRequest(tlv_request_t* request)
{
  char fifoName[USER_FIFO_PATH_LEN];
  char *pid;
  sprintf(pid, "%d", request->value.header.pid);
  strcpy(fifoName, USER_FIFO_PATH_PREFIX);
  strcat(fifoName, pid);
  int operation= request->type;
  uint32_t accountID = request->value.header.account_id;
  tlv_reply_t reply;
  reply.length=sizeof(rep_value_t);
  switch(operation){
    case OP_CREATE_ACCOUNT:
    {
        reply.type=OP_CREATE_ACCOUNT;
        rep_value_t value;
        rep_header_t header;
        header.account_id= accountID;
        if(accountID != 0)
          header.ret_code = RC_OP_NALLOW;
        else
          header.ret_code = RC_OK;
        value.header=header;
        reply.value=value;
        create_user_account(request->value.create.account_id, request->value.create.password, request->value.create.balance);
        break;
    }
    case OP_BALANCE:
    {
      reply.type=OP_BALANCE;
      rep_value_t value;
      rep_header_t header;
      header.account_id= accountID;
      if(accountID == 0)
        header.ret_code = RC_OP_NALLOW;
      else
        header.ret_code = RC_OK;
      value.header=header;
        rep_balance_t balance;
      balance.balance=accounts[accountID].balance;
      value.balance=balance;
      reply.value=value;
      break;
    }
    case OP_TRANSFER:
    {
      reply.type=OP_TRANSFER;
      rep_value_t value;
      rep_header_t header;
      header.account_id= accountID;
      if(accountID == 0)
        header.ret_code = RC_OP_NALLOW;
      else
        header.ret_code = RC_OK;
      value.header=header;
        rep_transfer_t transfer;
      transfer.balance= request->value.transfer.amount;
      reply.value.transfer= transfer;
      reply.value=value;
      break;
    }
    case OP_SHUTDOWN:
    {
      reply.type=OP_SHUTDOWN;
      rep_value_t value;
      rep_header_t header;
      header.account_id= accountID;
      if(accountID != 0)
        header.ret_code = RC_OP_NALLOW;
      else
        header.ret_code = RC_OK;
      value.header=header;
      rep_shutdown_t shutdown;
      shutdown.active_offices = 100; /////////////////////////////just a number, have to figure out how to get the info
      reply.value.shutdown= shutdown;
      reply.value=value;
      break;
    }
    default:
      break;
  }
    
  int logfile = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logReply(logfile, pthread_self(), &reply);
  close(logfile);

  mkfifo(fifoName, 0660);
  int fd= open(fifoName,O_WRONLY);
  write(fd, &reply, sizeof(tlv_reply_t));
  close(fd);
  unlink(fifoName);
  return RC_OK;
}
void* bankOffice(void * arg)
{
  //sem_t* sem;

  int id = *(int *)arg;
  //tlv_request_t request;
  bankOfficeOpen(id);
  //int fd;
  // sem= sem_open(SEM_NAME,0,0600,0);
  //  if(sem == SEM_FAILED)
  // {
  //   perror("Server failed in sem_open()");
  //   exit(RC_OTHER);
  // } 
  // sem_wait(sem);
    
  //sem_close(sem);
  //processRequest(&request);
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

  int ret = create_account(0, password, 0);

  if (ret == 0)
    return logAccountCreation(slog, 0, &accounts[0]);
  else
    return ret;
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