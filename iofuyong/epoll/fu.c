#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/select.h>
#include<sys/poll.h>
#include <sys/epoll.h>
int main(){
	int fd1=open("f1",O_RDWR|O_NONBLOCK);	
	int fd2=open("f2",O_RDWR|O_NONBLOCK);
	char ch[1024]={0};
		
	int epfd=epoll_create1(0);
	struct epoll_event epe[2],events[5];
	epe[0].data.fd=fd1;
	epe[0].events=EPOLLIN;
	epoll_ctl(epfd,EPOLL_CTL_ADD,fd1,&epe[0]);

	epe[1].data.fd=fd2;
	epe[1].events=EPOLLIN;
	epoll_ctl(epfd,EPOLL_CTL_ADD,fd2,&epe[1]);
	
	while(1){
		int n=epoll_wait(epfd,events,5,1000);
		if(n==0){
			printf("no in f\n");
			continue;
		}
		for(int i=0;i<n;i++){
			int m=read(events[i].data.fd,ch,1023);
			if(m<0){
				perror("read()");
			}
			if(m>0){
			ch[m]='\0';
			}
			printf("%s;%s\n",events[i].data.fd==fd1?"f1":"f2",ch);
		}
	}
	return 0;
}
