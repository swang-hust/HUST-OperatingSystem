#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int wait_state = 1;

void change_state(int signum)
{
	wait_state = 0;
}

void signal_kill_1(int signum)
{
	printf("Chile Process 1 is Killed by Parent!\n");
	exit(0);
}

void signal_kill_2(int signum)
{
	printf("Chile Process 2 is Killed by Parent!\n");
	exit(0);
}


int main(int argc, char const *argv[])
{
	pid_t child[2];

	char readBuf[100];
	char writeBuf[100];
	int fd[2]; //Createpipe
	int *read_fd = &fd[0];
	int *write_fd = &fd[1];
	if (pipe(fd))
	{
		printf("Pipe Create Error!\n");
		return -1;
	}

	int i; //Counter;
	for (i=0; i<2; i++) //Create to child processes.
	{
		child[i] = fork();
		if (child[i]== 0)//For child, break;
		{
			if(i == 0)
			{
				child[1] = 0;
			}
			break;
		}
	}

	if(child[0]<0 || child[1]<0)
	{
		printf("fork() excute wrong!\n");
		return -1;
	}
	else if(child[0]+child[1] == 0)     //child processes one;
	{
		signal(SIGINT, SIG_IGN);
		signal(SIGUSR1, signal_kill_1);
		close(*read_fd);
		i = 1;
		while(i)
		{
			sprintf(writeBuf, "I send you %d times.", i++);
			write(*write_fd,writeBuf,100);
			sleep(1);
		}
		//printf("I am child: %d, my father: %d\n", getpid(), getppid());
	}
	else if(child[0]!=0 && child[1]==0) //child processes two;
	{
		signal(SIGINT, SIG_IGN);
		signal(SIGUSR1, signal_kill_2);
		close(*write_fd);
		while(1)
		{
			read(*read_fd,readBuf,100);
			printf("%s\n",readBuf);
		}
		//printf("I am child: %d, my father: %d\n", getpid(), getppid());
	}
	else if(child[0]!=0 && child[1]!=0)
	{
		signal(SIGINT, change_state);
		while(wait_state);
		kill(child[0],SIGUSR1);
		wait();
		close(fd[1]);
		kill(child[1],SIGUSR1);
		wait();
		close(fd[0]);
		printf("Parent Process is Killed!\n");
		//printf("I am father: %d\n", getpid());
	}
	return 0;
}
