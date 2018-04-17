//
// Created by vyam on 22/3/18.
//

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <time.h>

#define MAX_CMD_LEN 100
#define NUM_TOKEN 10
#define DELIMITERS " \t\r\n\a"

#define CYAN    "\x1b[36m"
#define RED    "\x1b[95m"
#define RESET   "\x1b[0m"


struct history {
    char cmd[MAX_CMD_LEN];
    struct tm* timeinfo;
    int bleh;
} hist[25];

int count = 0;


void put_history(char *cmd) {
    hist[count].bleh = 1;
    strcpy(&hist[count].cmd, cmd);
    hist[count].cmd[strlen(cmd)+1] = '\n';

    time_t rawtime;
    time(&rawtime);
    hist[count].timeinfo = localtime(&rawtime);

    count = (count+1)%25;
}

void get_history() {
    for(int i = 0; i < 25; i++) {
        if (hist[i].bleh) {
            char curr_time[32];
            //strftime(curr_time, 32, "_%Y_%m_%d_%H_%M", );
            printf("%s" RED "%s\n" RESET, asctime(hist[i].timeinfo), hist[i].cmd);
        }
    }
}

char *read_alias(char *cmd) {

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("/home/vyam/.usp_rc", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    char * ret = NULL;
    char * dub = NULL;
    while ((read = getline(&line, &len, fp)) != -1) {
        //char **args = malloc(NUM_TOKEN * sizeof(char*));
        //printf("jhk %s\n", line);
        dub = strdup(line);
        char *token = strtok(line, "=");
        token = strtok(line, "=");

        if(strcmp(token, cmd) == 0) {
            ret = strdup(dub+strlen(token)+1);
            break;
        }
    }

    fclose(fp);

    return ret;
}

int exec_process(char **args) {
    pid_t pid;

    int status;

    if((pid=fork())==-1)
       perror("Shell: can't fork: %s\n");

    else if(pid==0)
        if (execvp(args[0], args) == -1) {
            perror("Shell: can't exec: %s\n");
            exit(1);
        }

    if((pid=waitpid(pid,&status,0))<0)
        perror("Shell: waitpid error: %s\n");
}


int main(int argc, char **argv)
{

    while (1)
    {
        char cwd[1024];
        char host[64];
        struct passwd *pass;

        getcwd(cwd, sizeof(cwd));
        gethostname(host, sizeof(host));
        pass = getpwuid(getuid());

        printf("%s@%s:" CYAN "%s" RESET "$ ", pass->pw_name, host, cwd);

        char *cmd = NULL;
        long bufsize = 0;
        getline(&cmd, &bufsize, stdin); // no need to allocate memory getline will alllcate on its owns

        put_history(cmd);

        char *dub = strdup(cmd);
        char **args = malloc(NUM_TOKEN * sizeof(char*));
        char *token = strtok(cmd, DELIMITERS);

        int i = 0;

        char *alias = NULL;
        if((alias = read_alias(token)) != NULL) {
            token = strtok(alias, DELIMITERS);
        }
        else
            token = strtok(dub, DELIMITERS);


        while(token)
        {
            args[i] = token;
            token = strtok(NULL, DELIMITERS);
            i++;
        }

        args[i] = NULL;

        if(args) {
            if(*args[i - 1] == '\\') {
                while(*args[i - 1] == '\\') {
                    i--;
                    args[i] == NULL;
                    printf("> ");
                    cmd = NULL;
                    bufsize = 0;
                    getline(&cmd, &bufsize, stdin);

                    token = NULL;
                    token = strtok(cmd, DELIMITERS);

                    while(token)
                    {
                        args[i] = token;
                        token = strtok(NULL, DELIMITERS);
                        i++;
                    }
                    args[i] = NULL;
                }
            }

            if (strcmp(args[0], "cd") == 0) {
                if (chdir(args[1]) != 0) {
                    perror("cd: ");
                }
            }
            else if(strcmp(args[0], "hist") == 0)
                get_history();
            else if(strcmp(args[0], "exit") == 0)
                exit(0);
            else
                exec_process(args);
        }

        free(cmd);
        free(args);
    }
}
