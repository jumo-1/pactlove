#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>   
#include <unistd.h>
#include <semaphore.h>
sem_t sem1,sem2,sem3; // 定义两个信号量，供线程函数使用
struct  ThreadArgs{
    void* arg; // 存放主程序中需要传递给线程的参数
    int thread_id;
    pthread_mutex_t* lock_ptr; // 存放主程序中锁的指针
};
int num=0; // 定义全局变量，供线程函数使用
int num1=0; // 定义全局变量，供线程函数使用
pthread_mutex_t mutex1; // 定义全局锁，供线程函数使用
pthread_mutex_t mutex2; // 定义全局锁，供线程函数使用
void* func1(void* arg){
    struct ThreadArgs* args=(struct ThreadArgs*)arg;
    int* res=(int*)args->arg;
    printf("Hello from thread 1\n");
    
    for(int i=0;i<10000000;){
        pthread_mutex_lock(args->lock_ptr); // 加锁，保护共享资源
        //sleep(1); // 模拟线程执行过程中可能发生的上下文切换，增加竞争条件的可能性
        if(pthread_mutex_trylock(&mutex1)==0){
            (*res)++;
            pthread_mutex_unlock(&mutex1); // 解锁全局锁，允许其他线程访问共享资源
            pthread_mutex_unlock(args->lock_ptr); // 解锁，允许其他线程访问共享资源
            i++;
        }else{
            pthread_mutex_unlock(args->lock_ptr); // 解锁，允许其他线程访问共享资源
           // sleep(1); // 如果无法获取锁，等待一段时间后重试，增加竞争条件的可能性
        }
        

    }
    
    printf("Thread 1 exiting\n");
    return NULL;
}
void* func2(void* arg){
    struct ThreadArgs* args=(struct ThreadArgs*)arg;
    int* res=(int*)args->arg;
    printf("Hello from thread 2\n");
    
    for(int i=0;i<10000000;){
        pthread_mutex_lock(&mutex1); // 加锁，保护共享资源
        //sleep(1); // 模拟线程执行过程中可能发生的上下文切换，增加竞争条件的可能性
        if(pthread_mutex_trylock(&mutex2)==0){
            (*res)++;
            i++;
            pthread_mutex_unlock(&mutex2); // 解锁全局锁，允许其他线程访问共享资源
            pthread_mutex_unlock(&mutex1); // 解锁全局锁，允许其他线程访问共享资源
        }else{
            pthread_mutex_unlock(&mutex1); // 解锁全局锁，允许其他线程访问共享资源
           // sleep(1); // 如果无法获取锁，等待一段时间后重试，增加竞争条件的可能性
        }
        
        
    }
    printf("Thread 2 exiting\n");
    return NULL;
}
void* func3(void* arg){
    struct ThreadArgs* args=(struct ThreadArgs*)arg;
    int* res=(int*)args->arg;
    printf("Hello from thread 3\n");
    
    for(int i=0;i<10000000;){
        pthread_mutex_lock(&mutex2); // 加锁，保护共享资源
        //sleep(1); // 模拟线程执行过程中可能发生的上下文切换，增加竞争条件的可能性
        if(pthread_mutex_trylock(args->lock_ptr)==0){
            (*res)++;
            i++;
            pthread_mutex_unlock(args->lock_ptr); // 解锁，允许其他线程访问共享资源
            pthread_mutex_unlock(&mutex2); // 解锁全局锁，允许其他线程访问共享资源
        }else{
            pthread_mutex_unlock(&mutex2); // 解锁全局锁，允许其他线程访问共享资源
            //sleep(1); // 如果无法获取锁，等待一段时间后重试，增加竞争条件的可能性
        }
       
        
    }
    
    printf("Thread 3 exiting\n");
    return NULL;
}
void *func4(void* arg){
    
    printf("Hello from thread 4\n");
    while(1){ // 等待所有线程完成任务
        sem_wait(&sem1); // 等待信号量，确保线程按照顺序执行
        if(num1>=9){ // 如果所有线程都完成了任务，退出循环
            sem_post(&sem2); // 释放信号量，允许下一个线程执行
            pthread_exit(NULL); // 退出线程
        }
        num1++;
        printf("Thread 1: %d\n", num1);
        
        
        sem_post(&sem2); // 释放信号量，允许下一个线程执行
        
    }
    
    printf("Thread 4 exiting\n");
    return NULL;
}
void *func5(void* arg){
    
    printf("Hello from thread 5\n");
    while(1){ // 等待所有线程完成任务
        sem_wait(&sem2); // 等待信号量，确保线程按照顺序执行
        if(num1>=9){ // 如果所有线程都完成了任务，退出循环
            sem_post(&sem3); // 释放信号量，允许下一个线程执行
            pthread_exit(NULL); // 退出线程
        }
        num1++;
        printf("Thread 2: %d\n", num1);
        
        
        sem_post(&sem3); // 释放信号量，允许下一个线程执行
    }
    
    printf("Thread 5 exiting\n");
    return NULL;
}
void *func6(void* arg){
    
    printf("Hello from thread 6\n");
    while(1){ // 等待所有线程完成任务
        sem_wait(&sem3); // 等待信号量，确保线程按照顺序执行
        if(num1>=9){ // 如果所有线程都完成了任务，退出循环
            sem_post(&sem1); // 释放信号量，允许下一个线程执行
            pthread_exit(NULL); // 退出线程
        }
        num1++;
        printf("Thread 3: %d\n", num1  );
        
        
        sem_post(&sem1); // 释放信号量，允许下一个线程执行
    }
    
    printf("Thread 6 exiting\n");
    return NULL;
}
int main(){
    pthread_t tid1,tid2,tid3,tid4,tid5,tid6;
    int res=0;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex,NULL);
    pthread_mutex_init(&mutex1,NULL); // 初始化全局锁
    pthread_mutex_init(&mutex2,NULL); // 初始化全局锁
    struct ThreadArgs args1={.thread_id=1,.lock_ptr=&mutex,.arg=&res};
    sem_init(&sem1,0,1); // 初始化信号量，初始值为1
    sem_init(&sem2,0,0); // 初始化信号量，初始值为1
    sem_init(&sem3,0,0); // 初始化信号量
    if(pthread_create(&tid1,NULL,func1,&args1)!=0){
        fprintf(stderr,"Failed to create thread 1\n");
        return 1;
    }
    struct ThreadArgs args2={.thread_id=2,.lock_ptr=&mutex,.arg=&res};
    if(pthread_create(&tid2,NULL,func2,&args2)!=0){
        fprintf(stderr,"Failed to create thread 2\n");
        return 1;
    }
    struct ThreadArgs args3={.thread_id=3,.lock_ptr=&mutex,.arg=&res};
    if(pthread_create(&tid3,NULL,func3,&args3)!=0){
        fprintf(stderr,"Failed to create thread 3\n");
        return 1;
    }

    if(pthread_create(&tid4,NULL,func4,NULL)!=0){
        fprintf(stderr,"Failed to create thread 4\n");
        return 1;
    }
    if(pthread_create(&tid5,NULL,func5,NULL)!=0){
        fprintf(stderr,"Failed to create thread 5\n");
        return 1;
    }
    if(pthread_create(&tid6,NULL,func6,NULL)!=0){
        fprintf(stderr,"Failed to create thread 6\n");
        return 1;
    }

    if(pthread_join(tid1,NULL)!=0){
        fprintf(stderr,"Failed to join thread 1\n");
        return 1;
    }
    if(pthread_join(tid2,NULL)!=0){
        fprintf(stderr,"Failed to join thread 2\n");
        return 1;
    }
    if(pthread_join(tid3,NULL)!=0){
        fprintf(stderr,"Failed to join thread 3\n");
        return 1;
    }
    if(pthread_join(tid4,NULL)!=0){
        fprintf(stderr,"Failed to join thread 4\n");
        return 1;
    }
    if(pthread_join(tid5,NULL)!=0){
        fprintf(stderr,"Failed to join thread 5\n");
        return 1;
    }
    if(pthread_join(tid6,NULL)!=0){
        fprintf(stderr,"Failed to join thread 6\n");
        return 1;
    }
    printf("Final result: %d\n",res);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    sem_destroy(&sem1);
    sem_destroy(&sem2);
    sem_destroy(&sem3);
    return 0;
}