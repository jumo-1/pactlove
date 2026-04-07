#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
int main(){
	int file,file1,file2;
	struct stat st;
	if((file=open("afile.txt",O_RDWR|O_CREAT,0644))==-1){
		printf("文件创建失/打开败");
		return 0;
	}
	fstat(file,&st);

	file1=open("afile.txt",O_RDONLY,0644);
	file2=open("afile.txt",O_RDONLY,0644);

	fstat(file1,&st);
	fstat(file2,&st);
	char *wr="Hello world~";

	if(write(file,wr,strlen(wr))!=strlen(wr)){
		perror("message");
	}
	fsync(file);
	char rd1[20];
	lseek(file,0,SEEK_SET);
	ssize_t r1=read(file,rd1,20);
	char rd[3];
	lseek(file,0,SEEK_SET);
	ssize_t r;

	while((r=read(file,rd,3))!=0&&r!=-1){
		for(int i=0;i<r;i++){
			if(rd[i]==' '){
				printf("\n");
				lseek(file,-r+i+1,SEEK_CUR);
				break;
			}
			printf("%c",rd[i]);
		}
	}

	fstat(file,&st);
	if(r==0){
		printf("\nEnd\n");
	}
	if(r==-1){
		perror("\nmessage\n");
	}
	if(close(file)==-1){
		perror("message");
	}
	close(file);
	close(file1);
	close(file2);
	return 0;


}
