#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <string.h>
struct My_shm{
    int data[5];
};
int init_sem(int semid,int *value,int num){ 
    union semun {
        int val;               // 用于 SETVAL
        struct semid_ds *buf;  // 用于 IPC_STAT 和 IPC_SET
        unsigned short *array; // 用于 GETALL 和 SETALL
    } sem_union;
    unsigned short values[5];
    for(int i=0;i<num;i++){
        values[i]=value[i];
    }

    sem_union.array = values; 
    return semctl(semid, 0, SETALL, sem_union);
 
}
int P(int semid,int sem_num){
    struct sembuf sem_b={sem_num,-1,SEM_UNDO}; // 对信号量进行P操作，SEM_UNDO表示如果进程异常退出，系统会自动恢复信号量的值
    
    return semop(semid, &sem_b, 1);
}
int V(int semid,int sem_num){
    struct sembuf sem_b={sem_num,1,SEM_UNDO}; // 对信号量进行V操作，SEM_UNDO表示如果进程异常退出，系统会自动恢复信号量的值
    return semop(semid, &sem_b, 1);
}
int del_sem(int semid){
    return semctl(semid, 0, IPC_RMID);  
}
int main(){
    int shmid=shmget(IPC_PRIVATE, 1024, 0666 | IPC_CREAT);
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
    for(int i=0;i<5;i++){
        my_shm->data[i]=0; // 初始化共享变量
    }

    // 互斥信号量的初始化
    int semid=semget(IPC_PRIVATE, 5, 0666 | IPC_CREAT);
    if(semid==-1){
        perror("semget error");
        exit(1);   
    }   
    int sem_values[5]={1,1,1,1,1}; // 初始化五个信号量的值
    if(init_sem(semid, sem_values, 5)==-1){
        perror("init_sem error");
        exit(1);
    };
    printf("semid:%d\n",semid);
    for(int i=0;i<3;i++){
        pid_t pid=fork();
        if(pid==0){
            if(i==0){
                printf("子进程%d开始执行...\n",getpid());
                P(semid,0); // P操作，进入临界区
                sleep(1); // 模拟处理数据的时间
                P(semid,1); // P操作，进入临界区
                 sleep(1); // 模拟处理数据的时间
                my_shm->data[0]=1; // 向共享内存写入数据
                printf("子进程%d向共享内存写入数据：%d\n",getpid(),my_shm->data[0]);
                V(semid,0); // V操作，离开临界区
                V(semid,1); // V操作，离开临界区
            }
            if(i==1){
                printf("子进程%d开始执行...\n",getpid());
                P(semid,1); // P操作，进入临界区
                sleep(1); // 模拟处理数据的时间
                P(semid,2); // P操作，进入临界区
                 sleep(1); // 模拟处理数据的时间
                my_shm->data[1]=2; // 向共享内存写入数据
                printf("子进程%d向共享内存写入数据：%d\n",getpid(),my_shm->data[1]);
                V(semid,1); // V操作，离开临界区
                V(semid,2); // V操作，离开临界区
            }
            if(i==2){
                printf("子进程%d开始执行...\n",getpid());
                P(semid,2); // P操作，进入临界区
                sleep(1); // 模拟处理数据的时间
                P(semid,0); // P操作，进入临界区
                 sleep(1); // 模拟处理数据的时间
                my_shm->data[2]=3; // 向共享内存写入数据
                printf("子进程%d向共享内存写入数据：%d\n",getpid(),my_shm->data[2]);
                V(semid,2); // V操作，离开临界区
                V(semid,0); // V操作，离开临界区
            }
            
            exit(0); // 子进程退出
        }else if(pid>0){
            // 父进程继续创建下一个子进程
            continue;
        }else{
            perror("fork error");
            exit(1);    

        }
    }
    for(int i=0;i<3;i++){
        wait(NULL); // 等待所有子进程结束
        printf("父进程%d收到子进程%d结束信号...\n",getpid(),i);
    }
    del_sem(semid); // 删除信号量
    shmdt(shmaddr); // 分离共享内存
    shmctl(shmid, IPC_RMID, NULL); // 删除共享内存
    return 0;
}