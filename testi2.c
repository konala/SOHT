#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAXNUM 40
#define MAXLEN 160

void sighandler(int sig)
{
    switch (sig) {
        default:
            break;
    }
    return;
}

int isPipe(char **args) {

    int i =1;
    while(args[i]!= NULL) {
        if(strcmp(args[i],">")==0){
            return 1;
        } else if(strcmp(args[i],"<")==0){
            return 2;
        } else if(strcmp(args[i],"|")==0){
            return 3;
        }
        i++;
    }
    return 0;
}

int main(void)
{
    char * cmd, line[MAXLEN], * args[MAXNUM];
    int background, i;
    int pid;
    char cwd[MAXLEN];
    char user[MAXLEN];
    char host[MAXLEN];
    char home[MAXLEN] = "/home/";
    int event; // 1 = >, 2 = <, 3 = pipe, 0 = default

    char leftSide[MAXLEN];
    char rightSide[MAXLEN];
    char *rightArgs[MAXNUM]; // Right side of |
    char *leftArgs[MAXNUM]; // Left side of |
    
    int p;
    int k;
    int stdinDupl; 
    int stdoutDupl;

    int fd[2];
    
    signal(SIGINT, sighandler);
    signal(SIGALRM, SIG_IGN);
    
    /* Default home directory */
    strcat(home, getenv("USER")); //Append username to the home path
    setenv("HOME", home, 1); //Set the home environmental variable
    chdir(getenv("HOME")); // Change starting directory according to home
    
    /* Main loop */
    while (1) {
        k = 0;
        background = 0;
        stdinDupl = dup(0);
        stdoutDupl = dup(1);
        
        /* print the prompt */
        getcwd(cwd, sizeof(cwd)); 
        getlogin_r(user, sizeof(user));
        gethostname(host, sizeof(host));
        printf("[SOHT] %s@%s:%s$ ", user, host, cwd);
        /* set the timeout for alarm signal (autologout) */
        //alarm(LOGOUT);
        
        /* read the users command */
        if (fgets(line,MAXLEN, stdin) == NULL) {
            printf("\nlogout\n");
            exit(0);
        }
        line[strlen(line) - 1] = '\0';
        
        if (strlen(line) == 0)
            continue;
        
        /* start to background? */
        if (line[strlen(line)-1] == '&') {
            line[strlen(line)-1]=0;
            background = 1;
        }
        
        /* split the command line */
        i = 0;
        cmd = line;
        while ( (args[i] = strtok(cmd, " ")) != NULL) {
            printf("arg %d: %s\n", i, args[i]);
            i++;
            cmd = NULL;
        }
        args[i] = NULL;

        if (strcmp(args[0],"exit")==0) {
            exit(0);


        /*if first argument is "cd"*/
        } else if (strcmp(args[0],"cd")==0) {

            /*if no second argument for cd, goes to home directory*/
            if(args[1]== NULL) {       
                chdir(getenv("HOME"));

            /*error*/
            }else if(chdir(args[1])!=0) {  
                perror("cd");
            }
            continue;
        }
        /* Pipes and redirection */
        event = isPipe(args); // Check if arguments require pipes or redirection
        /* Redirection > */
        if (event == 1) {
            printf(">\n");
            leftSide[0] = '\0';
            rightSide[0] = '\0';
            k = 0;

            /* Initialize rightArgs */
            while (strcmp(args[k], ">") != 0) {
                strcat(leftSide, args[k]);
                strcat(leftSide, " ");
                leftArgs[k] = args[k];
                k++;
            }
            leftArgs[k] = NULL;
            k++;
            p = k;
            printf("leftSide: %s\n", leftSide);

            /* Initialize leftArgs */
            while (args[k] != NULL) {
                strcat(rightSide, args[k]);
                //strcat(rightSide, " ");
                rightArgs[k-p] = args[k];
                k++;
            }
            rightArgs[k-p] = NULL;
            printf("rightSide: %s\n", rightSide);

            /* Store original stdin and stdout */
            stdinDupl = dup(0);
            stdoutDupl = dup(1);
            
            //create file named after argument after ">"
            //parameters for write only mode, create if file dont exist etc.

            int outputFd = open(rightSide, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IWGRP               | S_IRGRP | S_IRUSR);           
            dup2(outputFd,1);
            close(outputFd);
            pid_t pid11;
            pid11 = fork();
            int stat;
            if(pid11<0){

                perror("fork");
                continue;
            } else if(pid11== 0) {
                
                int ex = execvp(leftArgs[0],leftArgs);
            
                if(ex == -1) {
                    dup2(stdoutDupl,1);
                    close(stdoutDupl);
                    perror("exec");

                
                }

            } else {

                waitpid(pid11, &stat, 0);

            }
            dup2(stdoutDupl,1);
            close(stdoutDupl);
        

            


        /* Redirection < */
        } else if (event == 2) {

            k = 0;
            leftSide[0] = '\0';
            rightSide[0] = '\0';
            

            /* Initialize rightArgs */
            while (strcmp(args[k], "<") != 0) {
                strcat(leftSide, args[k]);
                strcat(leftSide, " ");
                leftArgs[k] = args[k];
                k++;
            }
            leftArgs[k] = NULL;
            k++;
            p = k;
            printf("leftSide: %s\n", leftSide);

            /* Initialize leftArgs */
            while (args[k] != NULL) {
                strcat(rightSide, args[k]);
                //strcat(rightSide, " ");
                rightArgs[k-p] = args[k];
                k++;
            }
            rightArgs[k-p] = NULL;
            printf("rightSide: %s\n", rightSide);

            /* Store original stdin and stdout */
            stdinDupl = dup(0);
            stdoutDupl = dup(1);

            int inputFd = open(rightSide,O_RDONLY);
            dup2(inputFd,0);
            close(inputFd);

            pid_t pid22;
            int stat22;
            pid22 = fork();

            if(pid22<0) {
                perror("fork");
                continue;

            }else if(pid22 == 0){           
                int ex = execvp(leftArgs[0],leftArgs);
                if(ex == -1) {
                    dup2(stdinDupl,0);
                    close(stdinDupl);

                    perror("exec");
                }
            
            }else {

                waitpid(pid22,&stat22,0);
            }


            dup2(stdinDupl, 0);
            close(stdinDupl);



        /* Pipes */
        } else if (event == 3) {
            
            pid_t pid1, pid2, waitid1, waitid2;
            int status1, status2;

            k = 0;
            leftSide[0] = '\0';
            rightSide[0] = '\0';

            /* Initialize rightArgs */
            while (strcmp(args[k], "|") != 0) {
                strcat(leftSide, args[k]);
                strcat(leftSide, " ");
                leftArgs[k] = args[k];
                k++;
            }
            leftArgs[k] = NULL;
            k++;
            p = k;
            printf("leftSide: %s\n", leftSide);

            /* Initialize leftArgs */
            while (args[k] != NULL) {
                strcat(rightSide, args[k]);
                strcat(rightSide, " ");
                rightArgs[k-p] = args[k];
                k++;
            }
            rightArgs[k-p] = NULL;
            printf("rightSide: %s\n", rightSide);

            /* Store original stdin and stdout */
            stdinDupl = dup(0);
            stdoutDupl = dup(1);

            pid1 = fork(); //Fork once
            if (pid1==0) { // Child
                pipe(fd); //Create pipe
                pid2 = fork(); //Second fork

                if (pid2 == 0) { //Grandchild
                    dup2(fd[1],1); //Make STDOUT be the copy of fd[1], which is write end of the pipe we created
                    close(fd[0]); //Close the read end of the pipe
                    execvp(leftArgs[0], leftArgs);

                } /*else { this else not needed?*/
                    waitid2 = waitpid(pid2, &status2, 0);
                    dup2(fd[0], 0);
                    close(fd[1]);
                    execvp(rightArgs[0], rightArgs);
                //}

                dup2(stdinDupl, 0);
                close(stdinDupl);
                dup2(stdoutDupl, 1);
                close(stdoutDupl);

            } else {
                waitid1 = waitpid(pid1, &status1, 0);
            }
            

        } else {

        
            /* fork to run the command */
            switch (pid = fork()) {
                case -1:
                    /* error */
                    perror("fork");
                    continue;
                case 0:
                    /* child process */
                    execvp(args[0], args);
                    perror("execvp");
                    exit(1);
                default:
                    /* parent (shell) */
                    if (!background) {
                        alarm(0);
                        //waitpid(pid, NULL, 0);
                        while (wait(NULL)!=pid)
                            printf("some other child process exited\n");
                    }
                    break;
            }
        }
        
    }
    return 0;
}
