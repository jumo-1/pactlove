#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int main(){
	int fd1,fd2;
	fd1=open("/proc/loadavg",O_RDONLY);
	fd2=open("/proc/meminfo",O_RDONLY);
	char ch1[5]={0};
	if(read(fd1,ch1,4)!=4){
		perror("message");
	}
	double n1=strtod(ch1,NULL);
	printf("%.2f\n",n1);
	
	char ch2[2048]={0};
	read(fd2,ch2,2048);
	char *pr1=strstr(ch2,"MemTotal");
	if(!pr1){perror("message");}
	unsigned long n2=strtoul(pr1+9,NULL,10);
	char *pr2=strstr(ch2,"MemAvailable");
	if(!pr2){perror("message");}
	unsigned long n3=strtoul(pr2+13,NULL,10);
	double n4=(double)n3/n2*100.0;
	printf("%.2f%%\n",n4);

	return 0;

	




}
