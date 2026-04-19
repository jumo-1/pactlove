#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <string.h>
#define NUM_CHILDREN 10
struct My_shm{
    int data;
    char str[100];
    int competition;
};
// 初始化信号量
int init_sem(int semid,int value){ 
    union semun {
        int val;               // 用于 SETVAL
        struct semid_ds *buf;  // 用于 IPC_STAT 和 IPC_SET
        unsigned short *array; // 用于 GETALL 和 SETALL
    } sem_union;
    sem_union.val = value;
    return semctl(semid, 0, SETVAL, sem_union);
 
}
// P操作
int P(int semid){
    struct sembuf sem_b={0,-1,SEM_UNDO}; // 对信号量进行P操作，SEM_UNDO表示如果进程异常退出，系统会自动恢复信号量的值
    
    return semop(semid, &sem_b, 1);
}
// V操作
int V(int semid){
    struct sembuf sem_b={0,1,SEM_UNDO}; // 对信号量进行V操作，SEM_UNDO表示如果进程异常退出，系统会自动恢复信号量的值
    return semop(semid, &sem_b, 1);
}
// 删除信号量
int del_sem(int semid){
    return semctl(semid, 0, IPC_RMID);  
}
void handle_sigint(int sig){
    //printf("父进程%d收到信号%d，继续执行...\n",getpid(),sig);
    // -1 表示回收任意子进程，WNOHANG 表示非阻塞回收子进程
    printf("子进程%d结束，父进程%d继续执行...\n",sig,getpid());
    while(waitpid(-1,NULL,WNOHANG)>0);
    
}
void handle_100(int sig){
    printf("父进程%d收到信号%d，继续执行...\n",getpid(),sig);
}
int main(){
    signal(SIGUSR1,handle_100);
    //signal(SIGCHLD,handle_sigint);//注册信号处理函数，处理子进程结束的信号
    // ... 之后 fork 出来的子进程结束后会被 handle_sigchld 自动处理
    pid_t pid[NUM_CHILDREN];
    int fd[2];
    pipe(fd); // 创建管道
    key_t key = ftok("msgqueuefile", 65); // 生成消息队列的键值
    int msgid = msgget(key, 0666 | IPC_CREAT); // 创建消息队列
    struct msg_buffer {
        long msg_type; // 消息类型
        char msg_text[100]; // 消息内容
    } message;
    //共享内存的初始化
    key_t k1=ftok("sharedmemfile", 65);
    int shmid=shmget(k1, 1024, 0666 | IPC_CREAT);
    if(shmid==-1){
        perror("shmget error");
        exit(1);
    }
    void *shmaddr=shmat(shmid,NULL,0); // 将共享内存附加到进程地址空间
    if(shmaddr==(void*)-1){
        perror("shmat error");
        exit(1);
    }
    struct My_shm *const my_shm=(struct My_shm*)shmaddr; // 将共享内存地址转换为结构体指针
    my_shm->competition=0; // 初始化竞争变量
    // 信号量的初始化
    key_t k2=ftok("semfile", 65);
    int semid=semget(k2, 1, 0666 | IPC_CREAT);
    if(semid==-1){
        perror("semget error");
        exit(1);
    }
    if(init_sem(semid, 1)==-1){
        perror("init_sem error");
        exit(1);
    };
    for(int i=0;i<NUM_CHILDREN;i++){
        pid[i]=fork();
        if(pid[i]<0){
            perror("fork error");
            exit(1);
        }
        if(pid[i]==0){
            //子进程
            if(i==1){
                printf("子进程%d:%d开始执行...\n",getpid(),i);
                kill(getppid(),SIGUSR1); // 子进程向父进程发送信号
                
            }
            if(i==2){
                printf("子进程%d:%d开始执行...\n",getpid(),i);
                char buffer[100];
                sprintf(buffer,"%d: Hello from child process !\n",getpid());
                write(fd[1],buffer,strlen(buffer)); // 子进程向管道写入数据
                if(close(fd[1])==-1){
                    perror("close error");
                }
                
            }
            if(i==3){
                printf("子进程%d:%d开始执行...\n",getpid(),i);
                message.msg_type = 1; // 设置消息类型
                sprintf(message.msg_text, "%d: Hello from child process 3!\n", getpid()); // 设置消息内容
                msgsnd(msgid, &message, sizeof(message.msg_text), 0); // 子
                //strcpy(message.msg_text, "Hello from child process 3!"); // 设置消息内容
                //execlp("whoami","whoami",NULL); // 子进程执行 whoami 命令
                
            }
            if(i==4){
                printf("子进程%d:%d开始执行...\n",getpid(),i);
                message.msg_type = 2; // 设置消息类型
                sprintf(message.msg_text, "%d: Hello from child process 4!\n", getpid()); // 设置消息内容
                msgsnd(msgid, &message, sizeof(message.msg_text), 0); // 子进程向消息队列发送消息
                
                //execlp("uptime","uptime",NULL); // 子进程执行 uptime 命令
                
            }
            if(i==0){
                printf("子进程%d:%d开始执行...\n",getpid(),i);
                msgrcv(msgid,&message, sizeof(message.msg_text),3,0); // 子进程从消息队列接收消息
                printf("子进程%d从消息队列接收到消息：%s",getpid(),message.msg_text);
                
                //execlp("ps","ps","-ef",NULL); // 子进程执行 ps -ef 命令
            }
            if(i==5){
                //int *int_ptr=(int*)shmaddr; // 将共享内存地址转换为整数指针
                //*int_ptr=42; // 向共享内存写入数据
                //printf("子进程%d:%d向共享内存写入数据：%d\n",getpid(),i,*int_ptr);
                my_shm->data=41;
                printf("子进程%d:%d向共享内存写入数据：%d\n",getpid(),i,my_shm->data);
            }
            if(i==6){
                strcpy(my_shm->str,"Hello from  6!"); // 向共享内存写入字符串
                printf("子进程%d:%d向共享内存写入字符串：%s\n",getpid(),i,my_shm->str);
            }
            if(i==7){
                P(semid); // P操作，进入临界区
                for(int j=0;j<1000000;j++){
                    
                    my_shm->competition++; // 竞争变量自增
                    
                }
                V(semid); // V操作，离开临界区
            }
            if(i==8){
                
                P(semid); // P操作，进入临界区
                for(int j=0;j<1000000;j++){
                    
                    my_shm->competition++; // 竞争变量自增
                    
                }
                V(semid); // V操作，离开临界区
                
            }
            if(i==9){
                
                P(semid); // P操作，进入临界区
                for(int j=0;j<1000000;j++){
                    
                    my_shm->competition++; // 竞争变量自增
                    
                }
                V(semid); // V操作，离开临界区
                
            }
            exit(0); // 子进程退出
        }else{
            continue; // 父进程继续创建下一个子进程
        }
    
    }
    
    for(int i=0;i<2;i++){
        msgrcv(msgid,&message, sizeof(message.msg_text),0,0); // 父进程从消息队列接收消息
        printf("父进程%d从消息队列接收到消息：%s",getpid(),message.msg_text);
    }
    message.msg_type = 3; // 设置消息类型
    sprintf(message.msg_text, "%d: Hello from parent process!\n", getpid()); // 设置消息内容
    msgsnd(msgid, &message, sizeof(message.msg_text), 0); // 父进程向消息队列发送消息
    char buffer[100];
    printf("父进程%d等待子进程结束...\n",getpid());
    int m=read(fd[0],buffer,sizeof(buffer)-1); // 父进程从管道读取数据
    if(m==-1){
        perror("read error");
    }
    if(m>0){
        buffer[m]='\0'; // 添加字符串结束符
        printf("父进程%d从管道读取到数据：%s",getpid(),buffer);
    }
    if(m==0){
        printf("管道已关闭，父进程%d没有读取到数据...\n",getpid());
    }
    for(int i=0;i<NUM_CHILDREN;i++){
        wait(NULL); // 等待所有子进程结束
        printf("父进程%d收到子进程%d结束信号...\n",getpid(),pid[i]);
    }
    close(fd[1]);  // 关闭写端
    close(fd[0]);  // 关闭读端
    // 任务完成后，删除消息队列
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl RMID failed");
    } else {
        printf("消息队列已成功从内核中销毁。\n");
    }
    printf("父进程%d从共享内存读取到数据：%d\n",getpid(),my_shm->data);
    printf("父进程%d从共享内存读取到字符串：%s\n",getpid(),my_shm->str);
    printf("父进程%d从共享内存读取到竞争变量的值：%d\n",getpid(),my_shm->competition);
    int ok=shmdt(shmaddr); // 分离共享内存
    if(ok==-1){
        perror("shmdt error");
        exit(1);
    }
    shmctl(shmid, IPC_RMID, NULL); // 删除共享内存
    del_sem(semid); // 删除信号量
    return 0;
}