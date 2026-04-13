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
#pragma pack(1)
struct Myprotocol{
    uint16_t magic; // 魔数，固定值0x1234
    uint8_t version; // 版本号，当前为1
    uint8_t type; // 消息类型，1表示文本消息,
    uint32_t len; // 后面数据的长度
    char payload[]; // 可变长度的消息内容
};
#pragma pack()
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
				unsigned char ch[1024];
				int m=recv(events[i].data.fd,ch,1023,0);
				if(m>0){
					ch[m]='\0';
					//struct Myprotocol *msg=(struct Myprotocol *)ch;
					struct Myprotocol tmp;
					size_t len_Myprotocol=sizeof(struct Myprotocol);
					if(m<len_Myprotocol){ //【重要修复】验证接收到的数据长度，确保至少包含协议头部
						printf("Incomplete protocol header from client fd %d\n", events[i].data.fd);
						continue;
					}
					memcpy(&tmp, ch, sizeof(struct Myprotocol)); //【重要修复】正确解析协议头部
					if(ntohs(tmp.magic)!=0x1234){ //【重要修复】验证魔数，确保协议正确
						printf("Invalid magic number from client fd %d\n", events[i].data.fd);
						continue;
					}
					
					size_t payload_len=ntohl(tmp.len); //【重要修复】正确解析消息内容长度——序列转换
					if(payload_len>0 && payload_len<=1023-len_Myprotocol){ //【重要修复】验证消息内容长度，防止缓冲区溢出
						//msg->payload=malloc(payload_len); //【重要修复】为消息内容分配内存
						struct Myprotocol *msg=malloc(len_Myprotocol+payload_len); //【重要修复】为整个协议结构分配内存
						memcpy(msg, ch , payload_len+len_Myprotocol); //【重要修复】正确解析协议头部和消息内容
						msg->payload[payload_len]='\0'; // 确保消息内容以'\0'结尾
						printf("127.0.0.1:8888\n\t%s\n", msg->payload);
						free(msg); //【重要修复】释放为协议结构分配的内存
						send(events[i].data.fd,"received\n",9,0);
					} else {
						printf("Invalid payload length from client fd %d\n", events[i].data.fd);
						continue;
					}
					
					
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
