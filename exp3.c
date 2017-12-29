#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>

#define BUF_NUM 8
#define BUF_SIZE 1024

int P(int semid, int index)
{
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op = -1;
	sem.sem_flg = 0;
	return semop(semid, &sem, 1);
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

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

char *copy(char *s,char *res){
    for(int i = 0;i < 1024;i ++){
        res[i] = s[i];
    }
    return res;
}

int semid;
int cr = 0, cw = 0;
const int sizeByte = 1024 * 8;
const int size = 8;
int rcount = 0,wcount = 0;
char pname[1024];
char name[20];

int main(int argc, char const *argv[])
{
    union semun arg;
    //获取并初始化信号灯变量
	semid = semget(IPC_PRIVATE,2,IPC_CREAT|0600);
    int tmp = semctl(semid,0,GETVAL);
    tmp = semctl(semid,0,GETVAL);
    arg.val = BUF_NUM;
	tmp = semctl(semid,0,SETVAL,arg);
	arg.val = 0;
	semctl(semid,1,SETVAL,arg);
    tmp = semctl(semid,0,GETVAL);
    tmp = semctl(semid,1,GETVAL);

    char * memory = (char *) mmap(NULL,BUF_NUM * BUF_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    char *index = (char *) mmap(NULL,BUF_NUM * BUF_NUM,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    if ( *(int *) memory == -1 )
    {
        printf("get memory error") ;
        return -1 ;
    }

    pid_t pid = fork();
    //如果进程创建失败，则输出提示语句并结束
    if(pid < 0)
    {
    	printf("Fork() runtime error!\n");
    	return 0;
    }
    //父进程读文件并写入缓冲区
    else if ( pid > 0 )
    {
        FILE* file = fopen(argv[1],"rb");
        int flag = 1;
        while(flag > 0){
            P(semid,0);
            flag = fread(memory+(BUF_SIZE)*cw,1,BUF_SIZE,file);
            printf("write %d bytes\n",flag);
            *((int*)(index + cw*4)) = flag;
            V(semid,1);
            cw = (cw + 1) % size;
            if(flag != BUF_SIZE){
                break;
            }
        }
        printf("write over\n");
        P(semid,0);
        V(semid,1);
        fclose(file);
        return  0;
    }
    //子进程读缓冲区并写入文件
    else if ( pid == 0) {
    	strcpy(pname, argv[2]);
    	strcpy(name, argv[3]);
    	strcat(pname, name);
        FILE* file = fopen(pname,"ab");
        char res[BUF_SIZE];
        while(1){
           int t = P(semid,1);
           int len = *((int*)(index + cr*4));
           printf("length is %d\n",len);
           if(len == 0){
               V(semid,0);
               break;
           }
           printf("read %d bytes.\n",len);
           fwrite(copy(memory + BUF_SIZE * cr,res),1,len,file);
           cr = (cr + 1) % size;
           V(semid,0);
           if(len < BUF_SIZE){
               break;
           }
        }
        fclose(file);
        printf("finished!/n");
    }
    munmap(memory,BUF_SIZE * BUF_NUM);
    munmap(index,BUF_NUM * BUF_NUM);
	return 0;
}