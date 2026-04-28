#define _GNU_SOURCE // 定义宏以使用GNU扩展功能 // 放在所有 #include 之前
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>   
#include <sys/syscall.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>
#define BUFFER_SIZE 5
#define LOG(fmt,...) do{ \
    struct timeval _tv;\
    gettimeofday(&_tv,NULL);\
    struct tm _tm;\
    localtime_r(&_tv.tv_sec,&_tm);\
    fprintf(stderr,"[%02d:%02d:%02d.%03ld][TID:%ld]"fmt"\n",_tm.tm_hour,_tm.tm_min,_tm.tm_sec, (long)(_tv.tv_usec/1000),(long)syscall(SYS_gettid),##__VA_ARGS__);\
    fflush(stderr);\
}while(0)
int shutdown_flag=0; // 定义一个标志变量，表示生产者是否已经完成生产
struct  ThreadArgs{
    void* arg; // 存放主程序中需要传递给线程的参数
    int thread_id;
    int *in; // 存放主程序中输入位置的指针
    int *out; // 存放主程序中输出位置的指针
    int *producer_count; // 生产者生产的产品数量
    int *consumer_count; // 消费者消费的产品数量
    int count;
    pthread_mutex_t* lock_ptr; // 存放主程序中锁的指针
    pthread_cond_t* cond_full; // 存放主程序中条件变量的指针
    pthread_cond_t* cond_empty; // 存放主程序中条件变量的指针
};
void *producer(void* arg){
    struct ThreadArgs* args=(struct ThreadArgs*)arg;
    int *buffer=(int*)args->arg;
    
    while(1){
        pthread_mutex_lock(args->lock_ptr);
        if(*(args->producer_count)>=20){ // 生产者生产了20个产品后退出循环
            pthread_cond_broadcast(args->cond_empty); // 唤醒等待消费者的条件变量
            pthread_cond_broadcast(args->cond_full); // 唤醒等待生产者的条件变量
            pthread_mutex_unlock(args->lock_ptr);
            pthread_exit(NULL);
        }
        // 如果缓冲区满了，等待消费者消费
        while((*(args->in)+1)%BUFFER_SIZE==*(args->out)&&(*(args->producer_count)<20)){ 
            pthread_cond_wait(args->cond_full,args->lock_ptr);
        }
        if(*(args->producer_count)>=20){ // 生产者生产了20个产品后退出循环
            pthread_cond_broadcast(args->cond_empty); // 唤醒等待消费者的条件变量
            pthread_cond_broadcast(args->cond_full); // 唤醒等待生产者的条件变量
            pthread_mutex_unlock(args->lock_ptr);
            pthread_exit(NULL);
        }
        
        // 生产者生产一个产品
        buffer[*(args->in)]=rand()%100; // 生产一个随机数作为产品
        LOG("Produced %d: %d", args->thread_id, buffer[*(args->in)]);
        *(args->in)=(*(args->in)+1)%BUFFER_SIZE; // 更新输入位置
        (*args->producer_count)++; // 增加生产者生产的产品数量
        args->count++; // 增加线程生产的产品数量
        if(*(args->in)!=*(args->out)){ // 如果缓冲区不空了，唤醒等待消费者的条件变量
            pthread_cond_signal(args->cond_empty); // 唤醒等待消费者的条件变量
        }
        pthread_mutex_unlock(args->lock_ptr);
    }
    return NULL;
}

void *consumer(void* arg){
    struct ThreadArgs* args=(struct ThreadArgs*)arg;
    int *buffer=(int*)args->arg;
    while(1){
        pthread_mutex_lock(args->lock_ptr);
        if(shutdown_flag==1&&*(args->in)==*(args->out)){ // 消费者消费了20个产品后退出循环
                
                pthread_mutex_unlock(args->lock_ptr);
                pthread_exit(NULL);
            }
        // 如果缓冲区空了，等待生产者生产
        while(*(args->in)==*(args->out)&&shutdown_flag!=1){
            
            pthread_cond_wait(args->cond_empty,args->lock_ptr);
        }
        if(shutdown_flag==1&&*(args->in)==*(args->out)){ // 消费者消费了20个产品后退出循环
                
                pthread_mutex_unlock(args->lock_ptr);
                pthread_exit(NULL);
            }
        
        LOG("Consumed %d: %d", args->thread_id, buffer[*(args->out)]); // 消费一个产品
        *(args->out)=(*(args->out)+1)%BUFFER_SIZE; // 更新输出位置
        (*args->consumer_count)++; // 增加消费者消费的产品数量
        args->count++; // 增加线程消费的产品数量
        if((*(args->in)+1)%BUFFER_SIZE!=*(args->out)){ // 如果缓冲区不满了，唤醒等待生产者的条件变量}
            pthread_cond_signal(args->cond_full); // 唤醒等待生产者的条件变量
        }
        pthread_mutex_unlock(args->lock_ptr);
        usleep(100000); // 模拟消费者消费产品的时间，增加程序的可观察性
    }
    return NULL;
}
int main(){
    pthread_t prod_tid1,prod_tid2,cons_tid1,cons_tid2,cons_tid3;
    int buffer[BUFFER_SIZE]; // 定义一个缓冲区，供生产者和消费者使用
    int in=0,out=0,pc=0,cc=0; // 定义两个变量，分别表示缓冲区的输入和输出位置
    pthread_mutex_t mutex; // 定义一个互斥锁，保护缓冲区的访问
    if(pthread_mutex_init(&mutex,NULL)!=0){ // 初始化互斥锁
        fprintf(stderr,"Failed to initialize mutex\n");
        return 1;
    }
    pthread_cond_t cond_full,cond_empty; // 定义一个条件变量，供生产者和消费者使用
    if(pthread_cond_init(&cond_full,NULL)!=0){ // 初始化条件变量
        fprintf(stderr,"Failed to initialize condition variable cond_full\n");
        return 1;
    }
    if(pthread_cond_init(&cond_empty,NULL)!=0){
        fprintf(stderr,"Failed to initialize condition variable cond_empty\n");
        return 1;
    }
    srand((unsigned)time(NULL)); // 设置随机数种子，确保每次运行程序时生成的随机数不同
    struct ThreadArgs prod_args={.thread_id=1,.count=0,.lock_ptr=&mutex,.arg=buffer,.in=&in,.out=&out,.cond_full=&cond_full,.cond_empty=&cond_empty,.producer_count=&pc,.consumer_count=&cc}; // 定义生产者线程的参数
    if(pthread_create(&prod_tid1,NULL,producer,&prod_args)!=0){    
        fprintf(stderr,"Failed to create producer thread\n");
        return 1;
    }
    struct ThreadArgs prod_args1={.thread_id=2,.count=0,.lock_ptr=&mutex,.arg=buffer,.in=&in,.out=&out,.cond_full=&cond_full,.cond_empty=&cond_empty,.producer_count=&pc,.consumer_count=&cc}; // 定义生产者线程的参数
    if(pthread_create(&prod_tid2,NULL,producer,&prod_args1)!=0){    
        fprintf(stderr,"Failed to create producer thread\n");
        return 1;
    }
    struct ThreadArgs cons_args={.thread_id=3,.count=0,.lock_ptr=&mutex,.arg=buffer,.in=&in,.out=&out,.cond_full=&cond_full,.cond_empty=&cond_empty,.producer_count=&pc,.consumer_count=&cc}; // 定义消费者线程的参数
    if(pthread_create(&cons_tid1,NULL,consumer,&cons_args)!=0){
        fprintf(stderr,"Failed to create consumer thread\n");
        return 1;
    }
    struct ThreadArgs cons_args1={.thread_id=4,.count=0,.lock_ptr=&mutex,.arg=buffer,.in=&in,.out=&out,.cond_full=&cond_full,.cond_empty=&cond_empty,.producer_count=&pc,.consumer_count=&cc}; // 定义消费者线程的参数
    if(pthread_create(&cons_tid2,NULL,consumer,&cons_args1)!=0){
        fprintf(stderr,"Failed to create consumer thread\n");
        return 1;
    }
    struct ThreadArgs cons_args2={.thread_id=5,.count=0,.lock_ptr=&mutex,.arg=buffer,.in=&in,.out=&out,.cond_full=&cond_full,.cond_empty=&cond_empty,.producer_count=&pc,.consumer_count=&cc}; // 定义消费者线程的参数
    if(pthread_create(&cons_tid3,NULL,consumer,&cons_args2)!=0){
        fprintf(stderr,"Failed to create consumer thread\n");
        return 1;
    }

    printf("Producer and Consumer threads created successfully\n");
    if(pthread_join(prod_tid1,NULL)!=0){
        fprintf(stderr,"Failed to join producer thread\n");
        return 1;
    }
    printf("Producer thread 1 produced %d items\n", prod_args.count);
    if(pthread_join(prod_tid2,NULL)!=0){
        fprintf(stderr,"Failed to join producer thread\n");
        return 1;
    }
    printf("Producer thread 2 produced %d items\n", prod_args1.count);
    printf("Producer threads joined successfully\n");
    pthread_mutex_lock(&mutex);
    shutdown_flag=1; // 设置标志变量，表示生产者已经完成生产
    pthread_cond_broadcast(&cond_empty); // 唤醒所有等待消费者的条件变量
    pthread_mutex_unlock(&mutex);
    if(pthread_join(cons_tid1,NULL)!=0){
        fprintf(stderr,"Failed to join consumer thread\n");
        return 1;
    }
    printf("Consumer thread 1 consumed %d items\n", cons_args.count);
    if(pthread_join(cons_tid2,NULL)!=0){
        fprintf(stderr,"Failed to join consumer thread\n");
        return 1;
    }
    printf("Consumer thread 2 consumed %d items\n", cons_args1.count);
    if(pthread_join(cons_tid3,NULL)!=0){
        fprintf(stderr,"Failed to join consumer thread\n");
        return 1;
    }
    printf("Consumer thread 3 consumed %d items\n", cons_args2.count);
    printf("Consumer threads joined successfully\n");
    pthread_mutex_destroy(&mutex); // 销毁互斥锁
    pthread_cond_destroy(&cond_full); // 销毁条件变量
    pthread_cond_destroy(&cond_empty); // 销毁条件变量
    return 0;
}