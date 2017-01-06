#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

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
	
	int fd[2];
	
	signal(SIGINT, sighandler);
	signal(SIGALRM, SIG_IGN);
	
	/* Default home directory */
	strcat(home, getenv("USER")); //Append username to the home path
	setenv("HOME", home, 1); //Set the home environmental variable
	chdir(getenv("HOME")); // Change starting directory according to home
	
	/* Main loop */
	while (1) {
		
		background = 0;
		
		/* print the prompt */
		getcwd(cwd, sizeof(cwd)); 
		getlogin_r(user, sizeof(user));
		gethostname(host, sizeof(host));
		printf("[SOHT] %s@%s:%s$ ", user, host, cwd);
		/* set the timeout for alarm signal (autologout) */
		//alarm(LOGOUT);
		
		/* read the users command */
		if (fgets(line,MAXLEN,stdin) == NULL) {
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
		/* Redirection < */
		} else if (event == 2) {
			printf("<\n");
		/* Pipes */
		} else if (event == 3) {
			int k = 0;
			int stdin; 
			int stdout;
	
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
			int p = k;
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
			stdin = dup(0);
			stdout = dup(1);

			pid_t pid1, pid2, pid3, waitid1, waitid2, waitid3;
			int status1, status2, status3;


			pid1 = fork();
			if (pid1==0) {
				pipe(fd);
				pid2 = fork();

				if (pid2 == 0) {
					dup2(fd[1],1);
					close(fd[0]);
					execvp(leftArgs[0], leftArgs);

				} else {
					waitid2 = waitpid(pid2, &status2, WIFSTOPPED(status1));
					dup2(fd[0], 0);
					close(fd[1]);
					execvp(rightArgs[0], rightArgs);
				}

				dup2(stdin, 0);
				close(stdin);
				dup2(stdout, 1);
				close(stdout);

			} else {
				waitid1 = waitpid(pid1, &status1, WIFSTOPPED(status1));
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
