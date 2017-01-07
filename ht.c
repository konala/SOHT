/* Systeemiohjelmoinnin harjoitustyö, Plan B
* 
* Tekijät:
* Konsta Ala-Ilomäki, 0418514
* Johannes Kohvakka, #######
* 7.1.2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAXNUM 40
#define MAXLEN 160

/* checks if case is redirection, pipe or just regular command, return values: 1 = >, 2 = <, 3 = |, 4 = < >, 0 = default */
int isPipe(char **args) { 

	int i =1;
	while(args[i]!= NULL) {
		if(strcmp(args[i],">")==0){
			return 1;
		} else if(strcmp(args[i],"<")==0){
			int ctr = i+1;
            while (args[ctr] != NULL) {
                if (strcmp(args[ctr], ">") == 0) {
                    return 4;
                }
                ctr++;
            }
			return 2;
		} else if(strcmp(args[i],"|")==0){
			return 3;
		}
		i++;
	}
	return 0;
}

int main(void) {
	char * cmd, line[MAXLEN], * args[MAXNUM];
	int background, i;
	int pid;
	char cwd[MAXLEN];
	char user[MAXLEN];
	char host[MAXLEN];
	char home[MAXLEN] = "/home/";	
	
	int event;
	int fd[2];
	char leftSide[MAXLEN];
	char rightSide[MAXLEN];
	char *rightArgs[MAXNUM];
	char *leftArgs[MAXNUM];
	char *middleArgs[MAXNUM];
	
	/* Default home directory */
	strcat(home, getenv("USER"));
	setenv("HOME", home, 1);
	chdir(getenv("HOME"));
	
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
			//printf("arg %d: %s\n", i, args[i]);
			i++;
			cmd = NULL;
		}
		args[i] = NULL;

		if (strcmp(args[0],"exit") == 0) {
			exit(0);


		/* if first argument is "cd" */
		} else if (strcmp(args[0],"cd") == 0) {

			/* if no second argument for cd, goes to home directory */
			if(args[1] == NULL) {	   
				chdir(getenv("HOME"));

			/*error*/
			}else if(chdir(args[1]) != 0) {  
				perror("cd");
			}
			continue;
		}

		/* Pipes and redirection */

		event = isPipe(args); // Check if arguments require pipes or redirection

		/* Redirection > */
		if (event == 1) {
			leftSide[0] = '\0';
			rightSide[0] = '\0';
			int k = 0;
			int stdinDupl; 
			int stdoutDupl;

			/* Initialize leftArgs */
			while (strcmp(args[k], ">") != 0) {
				strcat(leftSide, args[k]);
				strcat(leftSide, " ");
				leftArgs[k] = args[k];
				k++;
			}
			leftArgs[k] = NULL;
			k++;
			int p = k;
			//printf("leftSide: %s\n", leftSide);

			/* Initialize rightArgs */
			while (args[k] != NULL) {
				strcat(rightSide, args[k]);
				rightArgs[k-p] = args[k];
				k++;
			}
			rightArgs[k-p] = NULL;
			//printf("rightSide: %s\n", rightSide);

			/* Store original stdin and stdout */
			stdinDupl = dup(0);
			stdoutDupl = dup(1);

			/* file descriptor for output file */
			int outputFd = open(rightSide, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IWGRP | S_IRGRP | S_IRUSR); 			
			dup2(outputFd, 1); // makes stdout be a copy of output file descriptor "outputFd"
			close(outputFd);

			/* creating a fork */
			pid_t pid11;
			pid11 = fork();
			int stat;
			if (pid11 < 0) {
				perror("fork");
				continue;
			} else if (pid11 == 0) { // child
				/* Program execution. The output goes to stdout which is a copy of outputFd */ 
				int ex = execvp(leftArgs[0], leftArgs); 
				if (ex == -1) {
					dup2(stdoutDupl, 1);
					close(stdoutDupl);
					perror("exec");
				}
			} else { // Parent

				waitpid(pid11, &stat, 0);

			}
			/* Reset stdout back to normal and close the duplicate file descriptor which is no longer needed */ 
			dup2(stdoutDupl, 1);
			close(stdoutDupl);

		/* Redirection < */
		} else if (event == 2) {
			leftSide[0] = '\0';
			rightSide[0] = '\0';
			int k = 0;
			int stdinDupl; 
			int stdoutDupl;

			/* Initialize leftArgs */
			while (strcmp(args[k], "<") != 0) {
				strcat(leftSide, args[k]);
				strcat(leftSide, " ");
				leftArgs[k] = args[k];
				k++;
			}
			leftArgs[k] = NULL;
			k++;
			int p = k;
			//printf("leftSide: %s\n", leftSide);

			/* Initialize rightArgs */
			while (args[k] != NULL) {
				strcat(rightSide, args[k]);
				rightArgs[k-p] = args[k];
				k++;
			}
			rightArgs[k-p] = NULL;
			//printf("rightSide: %s\n", rightSide);

			/* Store original stdin and stdout */
			stdinDupl = dup(0);
			stdoutDupl = dup(1);

			/* File descriptor for program input */ 
			int inputFd = open(rightSide, O_RDONLY);
			dup2(inputFd, 0); // makes stdin be a copy of file descriptor "outputFd"
			close(inputFd);

			pid_t pid22;
			int stat22;
			pid22 = fork();

			if (pid22 < 0) {
				perror("fork");
				continue;
			} else if (pid22 == 0){	//child
				/* Program execution. Program gets its input from stdin which now is a copy of inputFd */
				int ex;
				ex = execvp(leftArgs[0], leftArgs);
				if (ex == -1) {
					dup2(stdinDupl, 0);
					close(stdinDupl);
					perror("exec");
				}
			} else {
				waitpid(pid22, &stat22, 0);
			}
			/* Reset stdin back to normal and close the duplicate file descriptor which is no longer needed */ 
			dup2(stdinDupl, 0);
			close(stdinDupl);



		/* Pipes */
		} else if (event == 3) {
			int k = 0;
			int stdinDupl; 
			int stdoutDupl;
	
			leftSide[0] = '\0';
			rightSide[0] = '\0';

			/* Initialize leftArgs */
			while (strcmp(args[k], "|") != 0) {
				strcat(leftSide, args[k]);
				strcat(leftSide, " ");
				leftArgs[k] = args[k];
				k++;
			}
			leftArgs[k] = NULL;
			k++;
			int p = k;
			//printf("leftSide: %s\n", leftSide);

			/* Initialize rightArgs */
			while (args[k] != NULL) {
				strcat(rightSide, args[k]);
				strcat(rightSide, " ");
				rightArgs[k-p] = args[k];
				k++;
			}
			rightArgs[k-p] = NULL;
			//printf("rightSide: %s\n", rightSide);

			/* Store original stdin and stdout */
			stdinDupl = dup(0);
			stdoutDupl = dup(1);

			pid_t pid1, pid2, waitid1, waitid2;
			int status1, status2;

			pid1 = fork(); // Fork once
			if (pid1 < 0) {
				perror("fork");
			}
			if (pid1 == 0) { // Child
				pipe(fd); // Create pipe with array fd[2] which is used to return two I/O file descriptors
				pid2 = fork(); // Second fork
				if (pid2 < 0) {
					perror("fork");
				}
				if (pid2 == 0) { // Grandchild
					dup2(fd[1], 1); // Make stdout be the copy of fd[1], which is the write end of the pipe we created earlier
					close(fd[0]); // Close the read end as it's not wanted
					int ex = execvp(leftArgs[0], leftArgs); // Program execution. Output goes to stdout which is a copy of our pipe's read end
					if (ex == -1) {
						dup2(stdinDupl, 0);
						close(stdinDupl);
						dup2(stdoutDupl, 1);
						close(stdoutDupl);
						perror("exec");
					}
				} 
				waitid2 = waitpid(pid2, &status2, 0); // Suspend execution until grandchild finishes
				dup2(fd[0], 0); // Child's stdin is now a copy of pipe's read end
				close(fd[1]); // Close the write end
				int ex2 = execvp(rightArgs[0], rightArgs); // Program execution. Input comes from stdin which is a copy of pipe's read end
				if (ex2 == -1) {
						dup2(stdinDupl, 0);
						close(stdinDupl);
						dup2(stdoutDupl, 1);
						close(stdoutDupl);
						perror("exec");
					}
				/* Reset stdin and stdout back to normal and close the duplicate file descriptors which are no longer needed */ 
				dup2(stdinDupl, 0);
				close(stdinDupl);
				dup2(stdoutDupl, 1);
				close(stdoutDupl);
			} else {
				waitid1 = waitpid(pid1, &status1, 0); // Suspend parent's execution until child finishes
			}

		/* Redirection < > */
		} else if (event == 4) {
			char middle[MAXLEN];
			middle[0] = '\0';
			leftSide[0] = '\0';
			rightSide[0] = '\0';
			int k = 0;
			int stdinDupl; 
			int stdoutDupl;

			/* Initialize leftArgs */
			while (strcmp(args[k], "<") != 0) {
				strcat(leftSide, args[k]);
				strcat(leftSide, " ");
				leftArgs[k] = args[k];
				k++;
			}
			leftArgs[k] = NULL;
			k++;
			int p = k;
			//printf("leftSide: %s\n", leftSide);

			/* Initialize middle arguments */
			while (strcmp(args[k], ">") != 0) {
				strcat(middle, args[k]);
				middleArgs[k-p] = args[k];
				k++;
			}
			//printf("middle: %s\n", middle);
			middleArgs[k] = NULL;
			k++;
			p = k;

			/* Initialize rightArgs */
			while (args[k] != NULL) {
				strcat(rightSide, args[k]);
				//strcat(rightSide, " ");
				rightArgs[k-p] = args[k];
				k++;
			}
			rightArgs[k-p] = NULL;
			//printf("rightSide: %s\n", rightSide);

			/* Store original stdin and stdout */
			stdinDupl = dup(0);
			stdoutDupl = dup(1);

			/* Create two file descriptors: One for program's input and one for output */
			int inputFd = open(middle,O_RDONLY);
			int outputFd = open(rightSide, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IWGRP | S_IRGRP | S_IRUSR);
			/* Make stdin and stdout copies of file descriptors */
			dup2(inputFd, 0);
			close(inputFd);
			dup2(outputFd, 1);
			close(outputFd);

			pid_t pid4;
			int stat4;
			pid4 = fork();

			if (pid4 < 0) {
				perror("fork");
				continue;

			} else if (pid4 == 0) {			
				int ex;
				/* Program execution. The file descriptors we created earlier determine program's input and output channels */
				ex = execvp(leftArgs[0], leftArgs);
				if (ex == -1) {
					dup2(stdinDupl,0);
					close(stdinDupl);
					perror("exec");
				}
			} else {
				waitpid(pid4,&stat4,0); // Suspend parent's execution until child finishes
			}
			/* Reset stdin and stdout back to normal and close the duplicate file descriptors which are no longer needed */ 
			dup2(stdinDupl, 0);
			close(stdinDupl);
			dup2(stdoutDupl, 1);
			close(stdoutDupl);	
		/* Built-in execution */ 			 			
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
						//alarm(0);
						waitpid(pid, NULL, 0);
					}
					break;
			}
		}
		
	}
	return 0;
}
