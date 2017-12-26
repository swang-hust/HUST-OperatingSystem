#include <unistd.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>  
#include <sys/sem.h> 
#include <pthread.h>

struct ARGU
{
	int sem1;
	int sem2;
	int result;
};

void P(int semid, int index)
{
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op = -1;
	sem.sem_flg = 0;
	semop(semid, &sem, 1);
	return;
}

void V(int semid, int index)
{	
	struct sembuf sem;	
	sem.sem_num = index;
    sem.sem_op =  1;
    sem.sem_flg = 0;	
    semop(semid,&sem,1);	
    return;
}

union semun{
	int val;
	struct sem_ds *buf;
	unsigned short *array;
};

//线程执行函数定义
void calculate(struct ARGU *a);
void print(struct ARGU *a);

int times;

int main(int argc, char const *argv[])
{
	if(argc == 1)
	{
		times = 100;
	}
	else
	{
		times = atoi(argv[1]);
	}
	//int s_write = semget((key_t)1234, 1, 0666|IPC_CREAT);
	//int s_read = semget((key_t)4321, 0, 0666|IPC_CREAT);
	//创建信号灯,并赋初值
	struct ARGU arg;
	arg.sem1 = semget(IPC_PRIVATE, 1, 0666|IPC_CREAT);
	arg.sem2 = semget(IPC_PRIVATE, 1, 0666|IPC_CREAT);
	arg.result = 0;

	union semun sem_args1, sem_args2;
	unsigned short array1[1]={1}, array2[1]={0};
	sem_args1.array = array1;
	sem_args2.array = array2;
	semctl(arg.sem1, 0, SETALL, sem_args1);
	semctl(arg.sem2, 0, SETALL, sem_args2);
	/*****************创建两个线程*************/
	pthread_t id_w, id_r;	//线程句柄定义
	int ret = pthread_create(&id_w, NULL, (void *)calculate, &arg);
	if(ret!=0)
	{
		printf("Thread create error!\n");
	}
	ret = pthread_create(&id_r, NULL, (void *)print, &arg);
	if(ret!=0)
	{
		printf("Thread create error!\n");
	}

	//等待两个线程运行结束
	pthread_join(id_w, NULL);
	pthread_join(id_r, NULL);
	//删除信号灯
	return 0;
}


void calculate(struct ARGU *a)
{
	static int i=1;
	while(i<=times)
	{
		P(a->sem1,0);
		a->result += i;
		V(a->sem2,0);
		i++;
		
	// }
	}
}

void print(struct ARGU *a)
{
	static int j=1;
	while(j<=times)
	{
		P(a->sem2,0);
		printf("Round %d:\n\tThe result is %d.\n\n",j,a->result);
		V(a->sem1,0);
		j++;
	}
}