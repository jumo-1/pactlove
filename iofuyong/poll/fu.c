#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/select.h>
#include<sys/poll.h>
int main(){
	int fd1=open("f1",O_RDWR|O_NONBLOCK);	
	int fd2=open("f2",O_RDWR|O_NONBLOCK);
	struct pollfd pf[2];
	pf[0].fd=fd1;
	pf[0].events=POLLIN;

	pf[1].fd=fd2;
	pf[1].events=POLLIN;
	char ch[1024]={0};
	while(1){
		int ready=poll(pf,2,2000);
		if(ready==0){
			printf("no in f\n");
		}
		if(ready==-1){
			perror("poll()");
		}

		for(int i=0;i<2;i++){
			if(pf[i].revents & POLLIN){
				int n=read(pf[i].fd,ch,1023);
				if(n<0){
					perror("message");
				}else{
				ch[n]='\0';
				}
				printf("f%d:%s\n",i+1,ch);
			}
		}
	}
	



	return 0;
}
