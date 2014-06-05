#include <stdio.h>      
#include <stdlib.h>    
#include <string.h>   
#include <fcntl.h>   
#include <errno.h>  
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc,char *argv[])
{
	int from_fd,to_fd;
	int bytes_read,bytes_write;
	char buffer[100];
	char *ptr;
	if(argc!=3)
	{
		fprintf(stderr,"Usage:%s from file to file\n\a",argv[0]);
		exit(1);
	}
	if((from_fd=open(argv[1],O_RDONLY))==-1)  
	{
		return -1;
	}
	if((to_fd=open(argv[2],O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR))==-1) 
	{
		return -2;
	}
	while(bytes_read=read(from_fd,buffer,100))
	{
		if((bytes_read==-1)&&(errno!=EINTR)) break;
		else if(bytes_read>0)
		{
			ptr=buffer;
			while(bytes_write=write(to_fd,ptr,bytes_read))
			{
				if((bytes_write==-1)&&(errno!=EINTR)) break;
				else if(bytes_write==bytes_read) break;
				else if(bytes_write>0)
				{
					ptr+=bytes_write;
					bytes_read-=bytes_write;
				}
			}
			if(bytes_write==-1) break;
		}
	}
	close(from_fd);
	close(to_fd);
	return 0;
} 
