#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

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

int isPipe(char **args) 
{

	int i =1;
	while(args[i]!= NULL) {
		if(strcmp(args[i],">")==0){
			return 1;
		} else if(strcmp(args[i],"<")==0){
			return 2;
		} else if(strcmp(args[i],"|")==0){
			return 3;
		} else {printf("1\n");return 0;}
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
<<<<<<< HEAD


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
=======
		int ispipe = isPipe(args); //checks if the case is pipe or redirection
		
		if(ispipe == 0) {//no pipe or redirection
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
		} else if(ispipe ==1) {

		} else if(ispipe == 2) {

		} else if(ispipe == 3){
>>>>>>> 8bc7a7376f2cf4e15f063bd64bc1e31a3c77946b
		}

	}
	return 0;
}






















