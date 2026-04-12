#include<stdio.h>
#include<signal.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>  
volatile sig_atomic_t stop = 0;
void handle_sigint(int sig) {
    stop = 1; // 设置标志位
}
int main(){
	//【重要修复】忽略SIGPIPE信号，防止客户端断开时服务器崩溃
	signal(SIGPIPE, SIG_IGN); 
	// 处理SIGINT信号，优雅关闭服务器
	signal(SIGINT, handle_sigint);
	int lfd=socket(AF_INET,SOCK_STREAM,0);
	if(lfd==-1){
		perror("socket();");
		return 0;
	}
	struct sockaddr_in serv_addr;
	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);
	serv_addr.sin_port = htons(8888);
	serv_addr.sin_family=AF_INET;
	socklen_t len=sizeof(struct sockaddr_in);
	//【重要修复】设置SO_REUSEADDR选项，允许地址重用，避免绑定失败
	int opt=1;
	setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	//
	if(bind(lfd,(struct sockaddr *)&serv_addr,len)==-1){
		perror("bind();");
		return 0;
	}
	int epfd = epoll_create1(0);
	struct epoll_event epe,events[10];
	epe.events=EPOLLIN;
	epe.data.fd=lfd;
	epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&epe);
	if(listen(lfd,128)==-1){
			perror("listen();");
			return 0;
		}
	while(!stop) { // 循环直到收到SIGINT信号
		
		int num=epoll_wait(epfd,events,10,-1);
		if(num==-1){
			if (errno == EINTR) {
				// 【重要修复】处理被信号中断的情况，继续等待事件
				continue;
			}
			perror("epoll_wait();");
			return 0;
		}
		for(int i=0;i<num;i++){
			if(events[i].data.fd==lfd){
				struct sockaddr_in clie_addr;
				int cfd=accept(lfd,(struct sockaddr *)&clie_addr,&len);
				epe.data.fd=cfd; //【重要修复】正确设置事件数据为新连接的文件描述符
				epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&epe);
			}
			else{
				char ch[1024];
				int m=recv(events[i].data.fd,ch,1023,0);
				if(m>0){
					ch[m]='\0';
					printf("127.0.0.1:8888\n\t%s\n",ch);
					send(events[i].data.fd,"received\n",9,0);
				} else if (m == 0) {
                    // 【重要修复】客户端断开，清理资源
                    printf("Client fd %d closed connection.\n", events[i].data.fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    close(events[i].data.fd);
                }
			}

		}

	
	
	}
	
	
	printf("Shutting down server...\n");
	close(lfd);
	return 0;

}
