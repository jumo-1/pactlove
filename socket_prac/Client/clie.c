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
    uint32_t len; // 后面数据的长度
    uint8_t type; // 消息类型，1表示文本消息,
    char payload[]; // 可变长度的消息内容
};
#pragma pack()
int main(){
    int cfd=socket(AF_INET,SOCK_STREAM,0);
    if(cfd==-1){
        perror("socket();");
        return 0;
    }
    struct sockaddr_in clie_addr;
    inet_pton(AF_INET, "127.0.0.1", &clie_addr.sin_addr.s_addr);
    clie_addr.sin_port = htons(8888);
    clie_addr.sin_family=AF_INET;
    socklen_t len=sizeof(struct sockaddr_in);

    if(connect(cfd,(struct sockaddr *)&clie_addr,len)==-1){
        perror("connect();");
        return 0;
    }
    char *text="fuck you\n";
    size_t len_text=strlen(text);
    struct Myprotocol *msg=malloc(sizeof(struct Myprotocol)+len_text);
    msg->magic=htons(0x1234);
    msg->version=0;
    msg->len=htonl(len_text);
    msg->type=1;
    memcpy(msg->payload,text,len_text);
    send(cfd,msg,sizeof(struct Myprotocol)+len_text,0);
    free(msg);
    char ch[100];
    int m=recv(cfd,ch,100,0);
    if(m>0){
        ch[m]='\0';
        printf("\n%s\n", ch);

    }
    return 0;
}