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
#define BUF_SIZE 4096
#pragma pack(1)
struct Myprotocol{
    uint16_t magic; // 魔数，固定值0x1234
    uint8_t version; // 版本号，当前为1
    uint8_t type; // 消息类型，1表示文本消息,
    uint32_t len; // 后面数据的长度
    char payload[]; // 可变长度的消息内容
};
#pragma pack()
struct connection_item {
	int fd; // 连接的文件描述符
	unsigned char buffer[BUF_SIZE]; // 连接的缓冲区
	size_t buffer_len; // 缓冲区中数据的长度
	
};
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
	struct connection_item *item0=malloc(sizeof(struct connection_item)); //【重要修复】定义连接项结构体变量
	item0->fd = lfd; //【重要修复】设置监听文件描述符
	epe.events=EPOLLIN;
	epe.data.ptr = item0; //【重要修复】将事件数据指针设置为连接项
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
			struct connection_item *item = (struct connection_item *)events[i].data.ptr; //void *指针需要强制转换为struct connection_item *类型
			// 编译器无法自动推断类型，必须显式转换
			if(item->fd==lfd){
				struct sockaddr_in clie_addr;
				int cfd=accept(lfd,(struct sockaddr *)&clie_addr,&len);
				//epe.data.fd=cfd; //【重要修复】正确设置事件数据为新连接的文件描述符
				struct connection_item *item = malloc(sizeof(struct connection_item)); //【重要修复】为连接项分配内存
				item->fd = cfd; //【重要修复】设置连接项的文件描述符
				item->buffer_len = 0; //【重要修复】初始化连接项的缓冲区长度
				memset(item->buffer, 0, BUF_SIZE); //【重要修复】清空缓冲区
				struct epoll_event epe1;
				epe1.data.ptr = item; //【重要修复】将事件数据指针设置为连接项
				epe1.events = EPOLLIN;
				//epe1.events = EPOLLIN | EPOLLET; //【重要修复】使用边缘触发模式，提高性能
				epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&epe1); //【重要修复】将新连接添加到epoll监视列表
			}
			else {
				
				int m = recv(item->fd, item->buffer + item->buffer_len,
					BUF_SIZE - item->buffer_len, 0);
				if (m > 0) {  //确保接收到的数据长度大于0，避免处理空数据
					item->buffer_len += m;
					size_t len_Myprotocol = sizeof(struct Myprotocol);
					size_t processed = 0;
					while (item->buffer_len - processed >= len_Myprotocol) {
						struct Myprotocol tmp;
						memcpy(&tmp, item->buffer + processed, len_Myprotocol);
						if (ntohs(tmp.magic) != 0x1234) {
							printf("Invalid magic number from client fd %d\n", item->fd);
							break;
						}
						size_t payload_len = ntohl(tmp.len);
						size_t msg_len = len_Myprotocol + payload_len;
						if (payload_len == 0 || payload_len > BUF_SIZE - len_Myprotocol) {
							printf("Invalid payload length from client fd %d\n", item->fd);
							break;
						}
						if (item->buffer_len - processed < msg_len) {
							break; // 等待更多数据到达
						}
						struct Myprotocol *msg = malloc(msg_len + 1);
						memcpy(msg, item->buffer + processed, msg_len);
						msg->payload[payload_len] = '\0';
						printf("127.0.0.1:8888\n\t%s\n", msg->payload);
						free(msg);
						send(item->fd, "received\n", 9, 0);
						processed += msg_len;
					}
					if (processed > 0) {
						memmove(item->buffer, item->buffer + processed,
							item->buffer_len - processed);
						item->buffer_len -= processed;
					}
				} else if (m == 0) {
					// 【重要修复】客户端断开，清理资源
					printf("Client fd %d closed connection.\n", item->fd);
					epoll_ctl(epfd, EPOLL_CTL_DEL, item->fd, NULL);
					
					close(item->fd);
					free(item); //【重要修复】释放连接项的内存
				}
			}

		}

	
	
	}
	
	free(item0); //【重要修复】释放监听项的内存
	printf("Shutting down server...\n");
	close(lfd);
	return 0;

}
