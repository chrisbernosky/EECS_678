#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>

#define BSIZE 256

#define BASH_EXEC  "/bin/bash"
#define FIND_EXEC  "/bin/find"
#define XARGS_EXEC "/usr/bin/xargs"
#define GREP_EXEC  "/bin/grep"
#define SORT_EXEC  "/bin/sort"
#define HEAD_EXEC  "/usr/bin/head"

int main(int argc, char *argv[])
{
	int status;
	pid_t pid_1, pid_2, pid_3, pid_4;

	//creates a unidirectional data channel that can be used for interprocess communication
	//pipefd[0] is the read end of the pipe 
	//pipefd[1] is the write end of the pipe
	int pipefd_1[2];
	int pipefd_2[2];
	int pipefd_3[2];


	//establish the pipes
	pipe(pipefd_1);
	pipe(pipefd_2);
	pipe(pipefd_3);


	if (argc != 4) {
		printf("usage: finder DIR STR NUM_FILES\n");
		exit(0);
	}

	//find $1 -name '*'.[ch]
	pid_1 = fork();
	if (pid_1 == 0) {
		/* First Child */
		//based on slide 17 on lab 3
		char cmdbuf[BSIZE];
		bzero(cmdbuf, BSIZE);

		/*Set Up Pipes*/
		//allows to write to backend of file
		dup2(pipefd_1[1], STDOUT_FILENO); 

		//follow cammand for the 1st command prompt on the main IPC page
		//FIND_EXEC is used as it is defined above
		sprintf(cmdbuf, "%s %s -name \'*\'.[ch]", FIND_EXEC, argv[1]);

		//close open pipe ends
		close(pipefd_1[0]);
		close(pipefd_1[1]);
		close(pipefd_2[0]);
		close(pipefd_2[1]);
		close(pipefd_3[0]);
		close(pipefd_3[1]);

		//error reporting during execution
		if((execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char *) 0)) < 0)
		{
			fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
			return EXIT_FAILURE;
		}

		exit(0);
	}

	//xargs grep -c $2
	pid_2 = fork();
	if (pid_2 == 0) {
		/* Second Child */
		char cmdbuf[BSIZE];
		bzero(cmdbuf, BSIZE);

		/*Set Up Pipes*/
		dup2(pipefd_1[0], STDIN_FILENO);
		dup2(pipefd_2[1], STDOUT_FILENO);

		//follow cammand for the 2nd command prompt on the main IPC page
		//XARGS_EXEC and GREP_EXEC are used as they are defined above
		sprintf(cmdbuf, "%s %s -c %s", XARGS_EXEC, GREP_EXEC, argv[2]);

		//close open pipe ends
		close(pipefd_1[0]);
		close(pipefd_1[1]);
		close(pipefd_2[0]);
		close(pipefd_2[1]);
		close(pipefd_3[0]);
		close(pipefd_3[1]);

		//error reporting during execution
		if((execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char *) 0)) < 0)
		{
			fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
			return EXIT_FAILURE;
		}
		exit(0);
	}

	//sort -t : +1.0 -2.0 --numeric --reverse 
	//reverse sort 
	pid_3 = fork();
	if (pid_3 == 0) {
		/* Third Child */
		char cmdbuf[BSIZE];
		bzero(cmdbuf, BSIZE);

		/*Set Up Pipes*/
		dup2(pipefd_2[0], STDIN_FILENO);
		dup2(pipefd_3[1], STDOUT_FILENO);

		//follow cammand for the 3rd command prompt on the main IPC page
		//SORT_EXEC is used as it is defined above
		sprintf(cmdbuf, "%s -t : +1.0 -2.0 --numeric --reverse", SORT_EXEC);

		//Close all open pipe ends
		close(pipefd_1[0]);
		close(pipefd_1[1]);
		close(pipefd_2[0]);
		close(pipefd_2[1]);
		close(pipefd_3[0]);
		close(pipefd_3[1]);

		//error reporting during execution
		if((execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char *) 0)) < 0)
		{
			fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
			return EXIT_FAILURE;
		}
		exit(0);
	}

	//head --lines=$3
	pid_4 = fork();
	if (pid_4 == 0) {
		/* Fourth Child */
		char cmdbuf[BSIZE];
		bzero(cmdbuf, BSIZE);

		/*Set Up Pipes*/
		dup2(pipefd_3[0], STDIN_FILENO);

		//follow cammand for the 3rd command prompt on the main IPC page
		//HEAD_EXEC is used as it is defined above
		sprintf(cmdbuf, "%s --lines=%s", HEAD_EXEC, argv[3]);

		//close all open pipe ends
		close(pipefd_1[0]);
		close(pipefd_1[1]);
		close(pipefd_2[0]);
		close(pipefd_2[1]);
		close(pipefd_3[0]);
		close(pipefd_3[1]);

		//error reporting during execution
		if((execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char *) 0)) < 0)
		{
			fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
			return EXIT_FAILURE;
		}
		exit(0);
	}

	//make sure one last time that all pipe ends are closed
	close(pipefd_1[0]);
	close(pipefd_1[1]);
	close(pipefd_2[0]);
	close(pipefd_2[1]);
	close(pipefd_3[0]);
	close(pipefd_3[1]);

	if ((waitpid(pid_1, &status, 0)) == -1) {
		fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
		return EXIT_FAILURE;
	}
	if ((waitpid(pid_2, &status, 0)) == -1) {
		fprintf(stderr, "Process 2 encountered an error. ERROR%d", errno);
		return EXIT_FAILURE;
	}
	if ((waitpid(pid_3, &status, 0)) == -1) {
		fprintf(stderr, "Process 3 encountered an error. ERROR%d", errno);
		return EXIT_FAILURE;
	}
	if ((waitpid(pid_4, &status, 0)) == -1) {
		fprintf(stderr, "Process 4 encountered an error. ERROR%d", errno);
		return EXIT_FAILURE;
	}

	return 0;
}
