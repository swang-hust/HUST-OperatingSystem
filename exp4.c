#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

//节省栈空间
char path[1000][1000];
char filename[1000];
char temp[1000][1000];
char chmod_buf[11];
char chmod_default[11] = "----------";
char type;
struct tm *ftime;
unsigned long size;
int flag;

//获得文件类型
char getFileType(struct stat *st)
{
    char type = '-';
    switch (st->st_mode  & S_IFMT)
    {
        case S_IFSOCK:
        type = 's';
                break;
        case S_IFLNK:
        type = 'l';
                break;
        case S_IFREG:
        type = '-';
                break;
        case S_IFBLK:
        type = 'b';
                break;
        case S_IFDIR:
        type = 'd';
                break;
        case S_IFCHR:
        type = 'c';
                break;
        case S_IFIFO:
        type = 'p';
                break;
    }
    return type;
}

//获得文件访问权限
void getFilePerm(struct stat *st, char *perm)
{
    mode_t mode = st->st_mode;
    if (mode & S_IRUSR)
        perm[1] = 'r';
    if (mode & S_IWUSR)
        perm[2] = 'w';
    if (mode & S_IXUSR)
        perm[3] = 'x';
    if (mode & S_IRGRP)
        perm[4] = 'r';
    if (mode & S_IWGRP)
        perm[5] = 'w';
    if (mode & S_IXGRP)
        perm[6] = 'x';
    if (mode & S_IROTH)
        perm[7] = 'r';
    if (mode & S_IWOTH)
        perm[8] = 'w';
    if (mode & S_IXOTH)
        perm[9] = 'x';
}

void printdir(char const *dirname, int depth)
{
	struct stat statbuf;
	//if(!strcmp(dirname, "/dev/")) return;
	strcpy(path[depth], dirname); //path储存路径名
	DIR *dir;
	printf("\n%s:\n",dirname);
	/***************第一次遍历目录，获取文件大小*********/
	
	dir = opendir(path[depth]);//打开目录
	//如果打开目录失败，则退出
	if(dir == NULL)
    {
    	printf("opendir error!\n");
    	return;
    }
	struct dirent *entry;

	size = 0;
	while ((entry = readdir(dir))!=NULL)
	{
		char *fname;
		fname = entry->d_name;
		strcpy(temp[depth],path[depth]);
		strcat(temp[depth],fname);
		if((strcmp(fname,".")==0) || (strcmp(fname, ".."))==0)
		{
			continue;
		}
		if (lstat(temp[depth],&statbuf) == -1)
		{
			printf("STAT Error!\n");
			return;
		}
		size += statbuf.st_size/1024+((statbuf.st_size%1024>0)?1:0);
	}
	printf("total %lu\n", /*size/1024+((size%1024>0)?1:0)*/size);
	closedir(dir);


	/*************第二次遍历目录，输出文件夹内容**************/
	dir = opendir(path[depth]);
	if(dir == NULL)
    {
    	printf("opendir error!\n");
    	return;
    }
	while ((entry = readdir(dir))!=NULL)
	{
		char *fname;
		fname = entry->d_name;
		strcpy(temp[depth], path[depth]);
		strcat(temp[depth], fname);
		if((strcmp(fname,".")==0) || (strcmp(fname, ".."))==0)
		{
			continue;
		}
		if (lstat(temp[depth],&statbuf) == -1)
		{
			printf("STAT Error!\n");
			exit(0);
		}
		strcpy(chmod_buf, chmod_default);
		type = getFileType(&statbuf);
		chmod_buf[0] = type;
		getFilePerm(&statbuf, chmod_buf);
		
		ftime = localtime(&statbuf.st_mtime);
		if(getpwuid(statbuf.st_uid)==NULL) continue;
		printf("%s %lu %s %s %10lu %02d月 %02d %02d:%02d %s\n",
			chmod_buf, //类型及权限
			statbuf.st_nlink, //内部文件个数
			getpwuid(statbuf.st_uid)->pw_name,//拥有者
			getgrgid(statbuf.st_gid)->gr_name,//所属组
			statbuf.st_size,//文件大小
			ftime->tm_mon+1,//创建时间
			ftime->tm_mday,
			ftime->tm_hour,
			ftime->tm_min,
			fname//文件名
			);
	}
	closedir(dir);

	/*************第三次遍历目录，递归调用**************/
	dir = opendir(path[depth]);//打开目录
	//如果打开目录失败，则退出
	if(dir == NULL)
    {
    	printf("opendir error!\n");
    	return;
    }

	size = 0;
	while ((entry = readdir(dir))!=NULL)
	{
		char *fname;
		fname = entry->d_name;
		strcpy(temp[depth],path[depth]);
		strcat(temp[depth],fname);
		if((strcmp(fname,".")==0) || (strcmp(fname, ".."))==0)
		{
			continue;
		}
		if (lstat(temp[depth],&statbuf) == -1)
		{
			printf("STAT Error!\n");
			return;
		}
		if(statbuf.st_size !=0 && S_ISDIR(statbuf.st_mode))
		{
			strcat(temp[depth],"/");
			char const *p = temp[depth];
			printdir(p, depth+1);
		}
	}
	closedir(dir);
}

int main(int argc, char const *argv[])
{
	if (argc == 1)
	{
		printf("Error!No Pathname!\n");
		return 0;
	}
	printdir(argv[1], 0);
	return 0;
}