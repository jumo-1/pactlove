#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int main(){
	int fd1=open("f1",O_RDWR|O_NONBLOCK);
	int fd2=open("f2",O_RDWR|O_NONBLOCK);
	fd_set read_fds;
	char ch[1024]; 
		
	while(1){
	FD_ZERO(&read_fds);
	FD_SET(fd1,&read_fds);
	FD_SET(fd2,&read_fds);
	
	int max_fd=fd1>fd2?fd1:fd2;

	int ready=select(max_fd+1,&read_fds,NULL,NULL,NULL);
	if(ready==0){
		printf("no in fifo");
	}
	if(ready==-1){
		perror("select()");
	}
	if(FD_ISSET(fd1,&read_fds)){
		int n=read(fd1,ch,1024);
		ch[n]='\0';
		printf("f1:%s\n",ch);
	}
	
	if(FD_ISSET(fd2,&read_fds)){
		int n=read(fd2,ch,1024);
		ch[n]='\0';
		printf("f2:%s\n",ch);
	}
	}


	return 0;
}
