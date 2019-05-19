#include "server.h"
  
sem_t full, empty;
queue_t *queue;
bank_account_t accounts[MAX_BANK_ACCOUNTS];
bool shutdown=false;
int numThreads;
pthread_mutex_t fifoMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t accountsMutex= PTHREAD_MUTEX_INITIALIZER;//[MAX_BANK_ACCOUNTS];

int main(int argc, char *argv[])
{
  queue = createQueue(MAX_QUEUE_SIZE);

  if (argc < 2)
  {
    printf("Insufficient number of arguments\n");
    return 1;
  }

  fclose(fopen(SERVER_LOGFILE, "w"));
  fclose(fopen(USER_LOGFILE, "w"));

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
  int fullValue;
  sem_getvalue(&full, &fullValue);
  logSemMech(MAIN_THREAD_ID, SYNC_OP_SEM_INIT , SYNC_ROLE_PRODUCER, 0 , fullValue);
  sem_init(&full, 0, 0);
  
  int emptyValue;
  sem_getvalue(&full, &emptyValue);
  logSemMech(MAIN_THREAD_ID, SYNC_OP_SEM_INIT , SYNC_ROLE_PRODUCER, 0 , emptyValue);
  sem_init(&empty, 0, MAX_QUEUE_SIZE);

  pthread_t threads[atoi(argv[1])];
  numThreads=atoi(argv[1]);
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
      int val;
      sem_getvalue(&empty, &val);
      logSemMech(MAIN_THREAD_ID, SYNC_OP_SEM_WAIT , SYNC_ROLE_PRODUCER, request.value.header.pid, val);  
      printf("id: %d, waiting before main sem wait \n", 0);
      sem_wait(&empty);
      printf("id: %d, waiting before main mutex lock \n", 0);
      mutex_lock(&fifoMutex, MAIN_THREAD_ID, SYNC_ROLE_PRODUCER, request.value.header.pid);
      push(queue, request);
      printf("id: %d, waiting before main mutex unlock \n", 0);
      mutex_unlock(&fifoMutex, MAIN_THREAD_ID, SYNC_ROLE_PRODUCER, request.value.header.pid);
      sem_post(&full);
      sem_getvalue(&full, &val);
      logSemMech(MAIN_THREAD_ID, SYNC_OP_SEM_POST , SYNC_ROLE_PRODUCER, request.value.header.pid, val );  

    }
    else
    {
      tlv_reply_t reply;
      reply.type=request.type;
      rep_value_t value;
      if(request.type==OP_BALANCE)
      {
        rep_balance_t balance;
        balance.balance=0;
        value.balance=balance;
      }
      if(request.type==OP_TRANSFER)
      {
        rep_transfer_t transfer;
        transfer.balance=0;
        value.transfer=transfer;
      }
      if(request.type==OP_SHUTDOWN)
      {
        rep_shutdown_t shutdownRep;
        shutdownRep.active_offices=atoi(argv[1]);
        value.shutdown=shutdownRep;
      }

      rep_header_t header;
      header.account_id = request.value.header.account_id;
      header.ret_code = ret;
      value.header = header;
      reply.value=value;
      reply.length=sizeof(header);
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
  
  sem_destroy(&full);
  sem_destroy(&empty);
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
    return true;
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
  rep_value_t value;
  rep_header_t header;
  header.account_id= accountID;

  switch(operation){
    case OP_CREATE_ACCOUNT:
    {  
      reply.type = OP_CREATE_ACCOUNT;
      if (accountID != 0)
        header.ret_code = RC_OP_NALLOW;
      else
      {
  //      mutex_lock_account(threadID, SYNC_ROLE_ACCOUNT, request->value.create.account_id); 
        logDelaySync(threadID ,request->value.header.account_id, request->value.header.op_delay_ms);
        usleep(request->value.header.op_delay_ms*1000);
         
        header.ret_code = create_user_account(request->value.create.account_id, request->value.create.password, request->value.create.balance);
        if(header.ret_code==RC_OK)
        {
          int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
          logAccountCreation(logfile, threadID, &accounts[request->value.create.account_id]);
          close(logfile);
        }
    //    mutex_unlock_account(threadID, SYNC_ROLE_ACCOUNT, request->value.header.account_id);
        printf("id: %d, mutex unlocked \n", threadID);

      }
      value.header = header;
      reply.value = value;
      reply.length=sizeof(header);
      break;
    }
    case OP_BALANCE:
    {
      reply.type=OP_BALANCE;
      if(accountID == 0)
        header.ret_code = RC_OP_NALLOW;
      else
        header.ret_code = RC_OK;
      value.header=header;
      rep_balance_t balance;
      mutex_lock_account(threadID, SYNC_ROLE_ACCOUNT, request->value.create.account_id);
      usleep(request->value.header.op_delay_ms*1000);
      logDelaySync(threadID ,request->value.header.account_id, request->value.header.op_delay_ms);
      balance.balance=accounts[accountID].balance;
      mutex_unlock_account(threadID, SYNC_ROLE_ACCOUNT, request->value.create.account_id);
      value.balance=balance;
      reply.value=value;
      reply.length=sizeof(header) + sizeof(balance);
      break;
    }
    case OP_TRANSFER:
    {
      int ret;
      rep_transfer_t transfer;
      transfer.balance = 0;

      mutex_lock_account(threadID, SYNC_ROLE_ACCOUNT, request->value.header.account_id);
      usleep(request->value.header.op_delay_ms * 1000);
      logDelaySync(threadID, request->value.header.account_id, request->value.header.op_delay_ms);

      mutex_lock_account(threadID, SYNC_ROLE_ACCOUNT, request->value.create.account_id);
      usleep(request->value.header.op_delay_ms * 1000);
      logDelaySync(threadID, request->value.create.account_id, request->value.header.op_delay_ms);

      if((accounts[request->value.header.account_id].balance - request->value.transfer.amount >= MIN_BALANCE) && 
      (accounts[request->value.transfer.account_id].balance + request->value.transfer.amount <= MAX_BALANCE))
        {
          accounts[request->value.header.account_id].balance -= request->value.transfer.amount;
          accounts[request->value.transfer.account_id].balance += request->value.transfer.amount;
          transfer.balance = request->value.transfer.amount;
          ret=RC_OK;
        }
        else
            ret=RC_NO_FUNDS;
      mutex_unlock_account(threadID, SYNC_ROLE_ACCOUNT, request->value.create.account_id);
      mutex_unlock_account(threadID, SYNC_ROLE_ACCOUNT, request->value.header.account_id);
      reply.type=OP_TRANSFER;
      if(accountID == 0 || request->value.transfer.account_id==0)
        {
          header.ret_code = RC_OP_NALLOW;
          transfer.balance = 0;
        }
      else
        header.ret_code = ret;
      value.header=header;
      value.transfer= transfer;
      reply.value=value;
      reply.length=sizeof(header) + sizeof(transfer);
      break;
    }
    case OP_SHUTDOWN:
    {
      int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
      logDelay(logfile, request->value.header.account_id, request->value.header.op_delay_ms);
      close(logfile);
      usleep(request->value.header.op_delay_ms*1000);

      int val;

      sem_getvalue(&full, &val);
      for(int i=0; i<numThreads-1; i++)
        sem_post(&full);

      reply.type=OP_SHUTDOWN;
      rep_shutdown_t shutdownRep;

      if(accountID != 0)
      {
        header.ret_code = RC_OP_NALLOW;
        shutdownRep.active_offices=numThreads;
      }
      else
      {
        header.ret_code = RC_OK;
        shutdown=true;
        int fullValue;
        sem_getvalue(&full, &fullValue);
        if(fullValue>= numThreads)
          shutdownRep.active_offices = numThreads;
        else
          shutdownRep.active_offices = fullValue;    
      }
      value.header=header;
         
      reply.value.shutdown= shutdownRep;
      reply.value=value;
      reply.length=sizeof(header) + sizeof(shutdownRep);
      break;
    }
    default:
      break;  
  }

  int logfile = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
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
    int value;

  do{
    int val;

    tlv_request_t request;
    sem_getvalue(&full, &val);

    logSemMech(id, SYNC_OP_SEM_WAIT , SYNC_ROLE_CONSUMER, 0 , val );  
    printf("id: %d, waiting before sem wait\n", id);
    sem_wait(&full);
    printf("id: %d, waiting after sem wait\n", id);

    if(shutdown && ((val-1)==0|| val==0))
      break;
    mutex_lock(&fifoMutex, id, SYNC_ROLE_CONSUMER, request.value.header.pid);
    request = pop(queue);
    mutex_unlock(&fifoMutex, id, SYNC_ROLE_CONSUMER, request.value.header.pid);;
        printf("id: %d, waiting before process request\n", id);

    processRequest(&request, id);
    sem_post(&empty);
    sem_getvalue(&empty, &val);
    logSemMech(id, SYNC_OP_SEM_POST , SYNC_ROLE_CONSUMER, request.value.header.pid , val );  
    sem_getvalue(&full,&value);
  }while(!shutdown || value!=0);

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
 // pthread_mutex_init(&accountsMutex[id], NULL);
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
  if (strlen(password) > MAX_PASSWORD_LEN + 1 || strlen(password) < MIN_PASSWORD_LEN)
    return 1;

  logDelaySync(MAIN_THREAD_ID, 0, 0);

  int ret = create_account(MAIN_THREAD_ID, password, 0);

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

int mutex_lock(pthread_mutex_t *mutex, int threadID, sync_role_t role, int id)
{
  pthread_mutex_lock(mutex);
  int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logSyncMech(logfile, threadID, SYNC_OP_MUTEX_LOCK, role, id);
  close(logfile);
}

int mutex_unlock(pthread_mutex_t* mutex, int threadID, sync_role_t role, int pid)
{
  pthread_mutex_unlock(mutex);
  int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logSyncMech(logfile,threadID, SYNC_OP_MUTEX_UNLOCK ,role, pid);
  close(logfile);
}

int mutex_lock_account(int threadID, sync_role_t role, int id)
{
  printf("inicio lock mutex\n");
  pthread_mutex_lock(&accountsMutex);
    printf("depois lock mutex\n");

  int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logSyncMech(logfile, threadID, SYNC_OP_MUTEX_LOCK, role, id);
  close(logfile);
}

int mutex_unlock_account(int threadID, sync_role_t role, int id)
{
  pthread_mutex_unlock(&accountsMutex);
  int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logSyncMech(logfile,threadID, SYNC_OP_MUTEX_UNLOCK ,role, id);
  close(logfile);
}


int logDelaySync(int threadID, int id, int delay_ms)
{
  int logfile= open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logSyncDelay(logfile, threadID,id, delay_ms);
  close(logfile);  
}

int logSemMech(int id, sync_mech_op_t sync_op, sync_role_t role, int pid, int val)
{
    int logfile = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    logSyncMechSem(logfile,id, sync_op , role, pid , val );  
    close(logfile);
}