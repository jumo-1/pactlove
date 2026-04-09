#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
//#include<sys/select.h>
//#include<sys/poll.h>
//#include <sys/epoll.h>
#include<dirent.h>
#include<errno.h>
int dir(const char*);
int main(){
	
	dir("example_dir");
	printf("完毕\n");
	return 0;
}
int dir(const char *dirname){
	DIR *dirp=opendir(dirname);
	if(dirp==NULL){
		if(errno==EACCES){
			printf("权限不足\n");
		}else if (errno==ENOENT){
			printf("路径不存在\n");
		}else if(errno==ENOTDIR){
			printf("是文件不是目录\n");
		}
	}
	while(1){
		struct dirent *dt=readdir(dirp);

		if(dt==NULL){
			return 0;
		}
		if(strcmp(dt->d_name,".")==0||strcmp(dt->d_name,"..")==0){
			continue;
		}
		char full[256];
		int n=snprintf(full,sizeof(full),"%s/%s",dirname,dt->d_name);
		full[n]='\0';
		struct stat st;
		stat(full,&st);
		printf("%s\n",full);
		if(S_ISREG(st.st_mode)){

			int fd=open(full,O_RDONLY);
			if(fd==-1){
				printf("文件%s无权限打开",full);
			}
			char ch[100];
			int n=read(fd,ch,100);
			if(n>=0){
				ch[n]='\0';
			}
			printf("\t%s\n",ch);
			close(fd);
				
		}
		if(S_ISDIR(st.st_mode)){
				
			dir(full);
		}
	}	

	closedir(dirp);
	return 0;
}
