#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
//gitkokkeilu
#define LOGOUT 15
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

int main(void)
{
	char * cmd, line[MAXLEN], * args[MAXNUM];
	int background, i;
	int pid;
	char cwd[MAXLEN];
	char user[MAXLEN];
	char host[MAXLEN];
	
	signal(SIGINT, sighandler);
	signal(SIGALRM, SIG_IGN);
	
	while (1) {
		
		background = 0;
		/* Set variables */
		getcwd(cwd, sizeof(cwd));
		getlogin_r(user, sizeof(user));
		gethostname(host, sizeof(host));
		/* print the prompt */
		printf("[SOHT] %s@%s:%s$ ", user, host, cwd);
		/* set the timeout for alarm signal (autologout) */
		alarm(LOGOUT);
		
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
		
		if (strcmp(args[0],"exit")==0) {
			exit(0);
		}
		
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
	return 0;
}
