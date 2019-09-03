#include "auxiliar.h"

// Global variables
bool md5 = false;
bool sha1 = false;
bool sha256 = false;
bool out = false;
bool var_r = false;
bool var_v = false;
clock_t initial_time;
int logfile;
char *logfilename = NULL;
int cout;

int main(int argc, char *argv[], char *envp[])
{
    initial_time = clock();
    char *outfile = NULL;
    cout = dup(STDOUT_FILENO);
    if (argc < 2)            if (strstr(argv[i], "md5") != NULL)

    {
        printf("Insufficient number of arguments");
    }

    for (int i = 1; i < argc ; i++)
    {
        if (strcmp(argv[i], "-r") == 0)
        {
            var_r = true;
        }

        if (strcmp(argv[i], "-h") == 0) // done for now
        {
            i++;

            if (strstr(argv[i], "md5") != NULL)
            {
                md5 = true;
            }

            if (strstr(argv[i], "sha1") != NULL)
            {
                sha1 = true;
            }

            if (strstr(argv[i], "sha256") != NULL)
            {
                sha256 = true;
            }
        }

        if (strcmp(argv[i], "-o") == 0) //done for now
        {
            i++;
            outfile = argv[i];
            out = true;
            int f= open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            close(f);
        }

        if (strcmp(argv[i], "-v") == 0)
        {
            int n = 0;
            var_v = true;
            bool found = false;
            char *logfilename;
            while (envp[n] != NULL)
            {
                logfilename = strstr(envp[n], "LOGFILENAME");
                if (logfilename != NULL)
                {
                    logfilename += 12;
                    found = true;
                    break;
                }
                n++;
            }
            if (!found)
            {
                printf("LOGFILENAME variable not found in environmental variables\n");
                return 1;
            }

            logfile = open(logfilename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
            char *arg;
            char *buffer;
            char *aux;
            buffer = malloc(sizeof(1024));
            arg = malloc(sizeof(1024));
            aux= malloc(sizeof(1024));
            clock_t sec = clock() - initial_time;

            sprintf(aux, "%0.2f", (((float)sec) / CLOCKS_PER_SEC) * 1000);
            strcat(buffer, aux);
            sprintf(aux , " - %d", getpid());
            strcat(buffer, aux);
            for(int i = 0; i < argc; i++)
            {
                strcat(arg, argv[i]);
                strcat(arg, " ");
            }
            strcat(buffer, " - COMMAND ");
            strcat(buffer, arg);
            strcat(buffer, "\n");
            int j=0;
            while(buffer[j]!= '\n')
            {
                char aux2[2];
                aux2[0] = buffer[j];
                aux2[1]= '\0';
                write(logfile, aux2, 1);
                j++;
            }
            write(logfile, "\n", 1);
        }
    }
    printAllDirectoryInfo(argv[argc - 1], outfile, logfile);
           
    if(logfilename != NULL)
        close(logfile);

}
