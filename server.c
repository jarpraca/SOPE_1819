#include "server.h"

queue_t *queue;
bank_account_t accounts[MAX_BANK_ACCOUNTS];
bool shutdown=false;

// int readline(int fd, tlv_request_t* request)
// {
//  int n;
//  do {
//  n = read(fd,request,1);
//  } while (n>0 && request++ != '\0');
//  return (n>0);
// } 
int main(int argc, char *argv[])
{
  queue = createQueue(MAX_QUEUE_SIZE);

  if (argc < 2)
  {
    printf("Insufficient number of arguments\n");
    return 1;
  }

  int fdFIFO;
  if(mkfifo(SERVER_FIFO_PATH,0660)<0){
    if (errno==EEXIST) 
      printf("FIFO '/tmp/secure_srv' already exists\n");
    else
      printf("Can't create FIFO\n");
  }

  do {
    fdFIFO=open(SERVER_FIFO_PATH, O_RDONLY);
        if (fdFIFO == -1) sleep(1);
  } while (fdFIFO == -1);

  

  sem_t* sem;
  sem= sem_open(SEM_NAME,O_CREAT,0600,0);
  if(sem == SEM_FAILED)
  {
    perror("Server failure in sem_open()");
    exit(3);
  } 
  pthread_t threads[atoi(argv[1])];

  if(*argv[1] < 1 || atoi(argv[1]) > MAX_BANK_OFFICES)
    return 1;

  int id[atoi(argv[1])];
  for(int i = 1; i <= atoi(argv[1]); i++)
  {
    id[i-1]=i;
    pthread_create(&threads[i-1], NULL, bankOffice, &id[i-1]);
  }

  create_admin_account(argv[2]);

tlv_request_t request;
while(!shutdown)
{
  int numRead = read(fdFIFO,&request, sizeof(tlv_request_t));
  if(numRead!=sizeof(tlv_request_t))
    continue;
    int ret;
    if((ret=authenticate(request.value.header.account_id, request.value.header.password))==RC_OK)
    {
        if(!isFull(queue))
        {
          // int val;
          // sem_getvalue(sem, &val);
          // int logfile = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
          // logSyncMechSem(logfile,MAIN_THREAD_ID, SYNC_OP_SEM_WAIT , SYNC_ROLE_PRODUCER, request.value.header.pid, val );  
          // close(logfile);
          // sem_wait(sem);
          push(queue, request);
          int val;
          sem_getvalue(sem, &val);
          int logfile = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
          logSyncMechSem(logfile,MAIN_THREAD_ID, SYNC_OP_SEM_POST , SYNC_ROLE_PRODUCER, request.value.header.pid, val );  
          close(logfile);
          sem_post(sem);
        }
        else
          continue;
    }
    else
    {
      tlv_reply_t reply;
      reply.type=request.type;
      reply.length=sizeof(rep_value_t);
      rep_value_t value;
      value.header.account_id=request.value.header.account_id;
      value.header.ret_code=ret;
      reply.value=value;
      int logfile = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
      logReply(logfile, MAIN_THREAD_ID, &reply);
      close(logfile);
      char fifoName[USER_FIFO_PATH_LEN];
      char pid[10];

      sprintf(pid, "%d", request.value.header.pid);
      strcpy(fifoName, USER_FIFO_PATH_PREFIX);
      strcat(fifoName, pid);

      mkfifo(fifoName, 0660);
      int fd= open(fifoName,O_WRONLY);
      write(fd, &reply, sizeof(tlv_reply_t));
      close(fd);
      unlink(fifoName);
    }
    
  }


  for(int i = 1; i <= atoi(argv[1]); i++)
    pthread_join(threads[i-1], NULL);     

  close(fdFIFO);
  unlink(SERVER_FIFO_PATH);
  
  sem_close(sem);
  sem_unlink(SEM_ACCOUNTS);
  sem_unlink(SEM_NAME);
 // free(queue->array);
  free(queue);
  return 0; 
}

char * generate_salt(){
  char characters[] = "0123456789abcdef";
  char *salt;
  salt = malloc(SALT_LEN + 1);

  srand(time(0));

  for(int i = 0; i < SALT_LEN; i++){
    char character[1];
    salt[i] = characters[rand() % 16];
  }

  return salt;
}


bool id_in_use(uint32_t id){

  if (accounts[id].account_id != 0 || id==0)
  {
    printf("accounts id: %d \n", accounts[id].account_id);
    return true;
  }
  return false;
}

int authenticate(uint32_t accountID, const char password[])
{
  char pass_salt[MAX_PASSWORD_LEN+SALT_LEN+1];
  if(!id_in_use(accountID))
    return RC_ID_NOT_FOUND;
  strcpy(pass_salt, password);
  strcat(pass_salt, accounts[accountID].salt);
  char* hash;
  hash=getSha256(pass_salt);

  if(strcmp(hash,accounts[accountID].hash)==0)
  {
    printf("Aqui\n");
    free(hash);
    return RC_OK;
  }
  free(hash);
  return RC_LOGIN_FAIL;
}

int processRequest(tlv_request_t* request, int threadID)
{
  char fifoName[USER_FIFO_PATH_LEN];
  char pid[10];

  sprintf(pid, "%d", request->value.header.pid);
  strcpy(fifoName, USER_FIFO_PATH_PREFIX);
  strcat(fifoName, pid);

  int operation= request->type;
  uint32_t accountID = request->value.header.account_id;

  tlv_reply_t reply;

  reply.length=sizeof(rep_value_t);

  sem_t *sem_accounts;
  sem_accounts = sem_open(SEM_ACCOUNTS, 0, 0600,0);
  int val;
  sem_getvalue(sem_accounts, &val);
  int logfile = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logSyncMechSem(logfile,threadID, SYNC_OP_SEM_WAIT , SYNC_ROLE_ACCOUNT, request->value.header.account_id, val );  
  close(logfile);
  sem_wait(sem_accounts);

  switch(operation){
    case OP_CREATE_ACCOUNT:
    {  
      int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
      logSyncDelay(logfile, threadID ,request->value.header.account_id, request->value.header.op_delay_ms);
      close(logfile);
      usleep(request->value.header.op_delay_ms*1000);
      reply.type = OP_CREATE_ACCOUNT;
      rep_value_t value;
      rep_header_t header;
      header.account_id = accountID;
      printf("id: %d \n", request->value.create.account_id);
      if (accountID != 0)
        header.ret_code = RC_OP_NALLOW;
      else
        header.ret_code = create_user_account(request->value.create.account_id, request->value.create.password, request->value.create.balance);
      value.header = header;
      reply.value = value;
      break;
    }
    case OP_BALANCE:
    {
      int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
      logSyncDelay(logfile, threadID ,request->value.header.account_id, request->value.header.op_delay_ms);
      close(logfile);
      usleep(request->value.header.op_delay_ms*1000);
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
      int ret;
      int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
      logSyncDelay(logfile, threadID ,request->value.header.account_id, request->value.header.op_delay_ms);
      close(logfile);
      usleep(request->value.header.op_delay_ms*1000);
      if(accounts[request->value.header.account_id].balance-request->value.transfer.amount>=MIN_BALANCE)
      {
        if(accounts[request->value.transfer.account_id].balance+request->value.transfer.amount<=MAX_BALANCE)
        {
          accounts[request->value.header.account_id].balance-=request->value.transfer.amount;
          accounts[request->value.transfer.account_id].balance+=request->value.transfer.amount;
          ret=RC_OK;
        }
        else
          ret=RC_NO_FUNDS;
      }
      reply.type=OP_TRANSFER;
      rep_value_t value;
      rep_header_t header;
      header.account_id= accountID;
      if(accountID == 0)
        header.ret_code = RC_OP_NALLOW;
      else
        header.ret_code = ret;
      value.header=header;
        rep_transfer_t transfer;
      transfer.balance= request->value.transfer.amount;
      printf("transfer value: %d \n", request->value.transfer.amount);
      printf("transfer balance: %d \n", transfer.balance);

      value.transfer= transfer;
      reply.value=value;
      break;
    }
    case OP_SHUTDOWN:
    {
      int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
      logDelay(logfile, request->value.header.account_id, request->value.header.op_delay_ms);
      close(logfile);
      usleep(request->value.header.op_delay_ms*1000);
      shutdown=true;
      reply.type=OP_SHUTDOWN;
      rep_value_t value;
      rep_header_t header;
      header.account_id= accountID;
      if(accountID != 0)
        header.ret_code = RC_OP_NALLOW;
      else
        header.ret_code = RC_OK;
      value.header=header;
      rep_shutdown_t shutdownRep;
      shutdownRep.active_offices = 100; /////////////////////////////just a number, have to figure out how to get the info
      reply.value.shutdown= shutdownRep;
      reply.value=value;
      break;
    }
    default:
      break;  
  }
  sem_getvalue(sem_accounts, &val);
  logfile = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logSyncMechSem(logfile,threadID, SYNC_OP_SEM_POST , SYNC_ROLE_ACCOUNT, request->value.header.account_id, val );  
  close(logfile);
  sem_post(sem_accounts);
  sem_close(sem_accounts);

  logfile = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logReply(logfile, threadID, &reply);
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
  int id = *(int*)arg;
  bankOfficeOpen(id);
  sem_t *sem;
  sem = sem_open(SEM_NAME,0,0600,0);
  if(sem == SEM_FAILED)
  {
    perror("READER failure in sem_open()");
    exit(3);
  } 
  do{
    int val;
    sem_getvalue(sem, &val);
    int logfile = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    logSyncMechSem(logfile,id, SYNC_OP_SEM_WAIT , SYNC_ROLE_CONSUMER, 0 , val );  
    close(logfile);
    sem_wait(sem);
    tlv_request_t request;
    if(!isEmpty(queue))
    {
      request = pop(queue);
      processRequest(&request, id);
    }
    sem_getvalue(sem, &val);
    logfile = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    logSyncMechSem(logfile,id, SYNC_OP_SEM_POST , SYNC_ROLE_CONSUMER, request.value.header.pid , val );  
    close(logfile);
    sem_post(sem);
  }while(!shutdown);

  sem_close(sem);  
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
  bank_account_t new;

  new.account_id = id;
  new.balance = balance;
  char * salt;
  salt = generate_salt();
  strcpy(new.salt, salt);
  
  char pass_salt[SALT_LEN + MAX_PASSWORD_LEN + 1];
  strcpy(pass_salt, password);
  strcat(pass_salt, new.salt);

  char *hash;
  hash = getSha256(pass_salt);
  strcpy(new.hash, hash);

  accounts[id] = new;
  free(salt);
  free(hash);

  return RC_OK;
}

int create_admin_account(const char *password){
  sem_t *sem_accounts;

  sem_accounts = sem_open(SEM_ACCOUNTS, O_CREAT, 0600, 0);
  if (sem_accounts == SEM_FAILED)
  {
    perror("WRITER failure in sem_open()");
    exit(4);
  }

  sem_post(sem_accounts);

  if (strlen(password) > MAX_PASSWORD_LEN + 1 || strlen(password) < MIN_PASSWORD_LEN)
    return 1;

  int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logSyncDelay(logfile, 0,0,0);
  close(logfile);  
  int ret = create_account(0, password, 0);

  sem_close(sem_accounts);
  if (ret == RC_OK)
  {
    int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    logAccountCreation(logfile, 0, &accounts[0]);
    close(logfile);
  }
  return ret;
}

int create_user_account(uint32_t id, const char *password, uint32_t balance){
  if(id_in_use(id))
    return RC_ID_IN_USE;
  
  if (id < 1 || id >= MAX_BANK_ACCOUNTS)
    return RC_OTHER;

  if (strlen(password) > MAX_PASSWORD_LEN + 1 || strlen(password) < MIN_PASSWORD_LEN + 1)
    return RC_OTHER;

  if (balance < MIN_BALANCE || balance > MAX_BALANCE)
    return RC_OTHER;

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
  sha256 = malloc(HASH_LEN + 1);

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
      read(fd2[READ], sha256, HASH_LEN);
      waitpid(pid2, &status2, 0);
    }
  }

  close(fd1[WRITE]);
  close(fd1[READ]);
  close(fd2[WRITE]);
  close(fd2[READ]);
  return sha256;
}