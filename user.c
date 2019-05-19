#include "user.h"

int main(int argc, char *argv[])
{
    int fd1, fd2;
    pid_t pidN = getpid();
    char fifoName[USER_FIFO_PATH_LEN];
    char pid[WIDTH_ID + 1];

    if (argc < 6)
    {
        printf("Insufficient number of arguments\n");
        return 1;
    }

    int id = atoi(argv[1]);
    char *password = argv[2];
    int delay = atoi(argv[3]);

    if (id < 0 || id > 4095)
    {
        printf("Id must be a number between 1 and %d\n", MAX_BANK_ACCOUNTS);
        return 1;
    }

    if (strlen(password) < MIN_PASSWORD_LEN || strlen(password) > MAX_PASSWORD_LEN + 1)
    {
        printf("Password must have between %d and %d characters\n", MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
        return 1;
    }

    if (delay < 0)
    {
        printf("Delay cannot be a negative number\n");
        return 1;
    }

    if (atoi(argv[4]) < 0 || atoi(argv[4]) > 4)
    {
        printf("Operation must correspond to a number between 0 and 4\n");
        return 1;
    }

    int operation = atoi(argv[4]);

    char *args = argv[5];

    sprintf(pid, "%d", pidN);
    strcpy(fifoName, USER_FIFO_PATH_PREFIX);
    strcat(fifoName, pid);

    do
    {
        fd1 = open(SERVER_FIFO_PATH, O_WRONLY | O_NONBLOCK);
        if (fd1 == -1)
            sleep(1);
    } while (fd1 == -1);

    tlv_request_t request;

    req_header_t req_header;

    req_header.pid = pidN;
    req_header.account_id = id;
    req_header.op_delay_ms = delay;
    strcpy(req_header.password, password);
    request.length = sizeof(req_header);

    req_value_t req_value;

    req_value.header = req_header;

    if (operation == OP_CREATE_ACCOUNT)
    {
        req_create_account_t account;
        getAccountArgs(args, &account);
        if(account.balance>MAX_BALANCE || account.balance<MIN_BALANCE)
        {
            printf("Balance of the account to be created not valid. Insert a value between %ld and %ld \n", MIN_BALANCE, MAX_BALANCE);
            return 1;
        }
        if(account.account_id>MAX_BANK_ACCOUNTS || account.account_id<1)
        {
            printf("Id of the account to be created not valid. Insert a value between %d and %d \n", 1, MAX_BANK_ACCOUNTS-1);
            return 1;
        }
        
        req_value.create = account;
        request.length += sizeof(account);
    }
    else if (operation == OP_TRANSFER)
    {
        req_transfer_t transfer;
        getTransferArgs(argv[5], &transfer);
        if(transfer.amount>MAX_BALANCE || transfer.amount<MIN_BALANCE)
        {
            printf("Amount of the transfer to be created not valid. Insert a value between 0 and %ld \n", MAX_BALANCE);
            return 1;
        }
        req_value.transfer = transfer;
        request.length += sizeof(transfer);
    }

    request.type = operation;
    request.value = req_value;

    int logfile = open(USER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    logRequest(logfile, pidN, &request);
    close(logfile);
    time_t starttime;
    starttime=time(NULL);

    write(fd1, &request, sizeof(request));
    close(fd1);

    // receive answer
    do
    {
        fd2 = open(fifoName, O_RDONLY | O_NONBLOCK);
        if (fd2 == -1)
            sleep(1);
    } while (fd2 == -1);

    tlv_reply_t reply;
    time_t currtime;
    currtime=time(NULL);
    bool timeOut=true;
    while(currtime-starttime<FIFO_TIMEOUT_SECS)
    {
        if(read(fd2, &reply, sizeof(tlv_reply_t))>0)
        {
            timeOut=false;
            break;
        }
        currtime=time(NULL);
    }

    if(timeOut)
    {
        printf("No reply received in %d seconds \n", FIFO_TIMEOUT_SECS);
        reply.value.header.ret_code=RC_SRV_TIMEOUT;
        reply.type=request.type;
        reply.value.header.account_id=request.value.header.account_id;
    }
    if(operation == OP_BALANCE)
    {
        printf("Balance: %d€\n", reply.value.balance.balance);
    }

    logfile = open(USER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    logReply(logfile, pidN, &reply);
    close(logfile);
    close(fd2);
    return 0;
}

void getAccountArgs(char *args, req_create_account_t *account)
{
    char id[WIDTH_ID + 1]="";
    char balance[WIDTH_BALANCE + 1]="";
    char password[MAX_PASSWORD_LEN+1]="";

    for(unsigned int i=0; i < strlen(args); i++){
        if(args[i] == ' ')
            break;
        id[i]=args[i];
    }

    for(unsigned int i=(strlen(id)+1); i < strlen(args); i++){
         if(args[i] == ' ')
             break;
         balance[i-(strlen(id)+1)]=args[i];
    }
    
    for(unsigned int i=(strlen(id)+strlen(balance)+2); i < strlen(args); i++){
        password[i-(strlen(id)+strlen(balance)+2)]=args[i];
    }
    account->account_id = atoi(id);
    account->balance = atoi(balance);
    strncpy(account->password, password, MAX_PASSWORD_LEN+1);
}

void getTransferArgs(char* args, req_transfer_t *transfer)
{
    char id[WIDTH_ID + 1]="";
    char amount[WIDTH_BALANCE + 1]="";
    for(unsigned int i=0; i < strlen(args); i++){

        if(args[i] == ' ')
            break;
        id[i]=args[i];
    }

    for(unsigned int i=strlen(id)+1; i < strlen(args); i++){

        if(args[i] == ' ')
            break;
        amount[i-strlen(id)-1]=args[i];
    }
    transfer->account_id = atoi(id);
    transfer->amount = atoi(amount);
}


