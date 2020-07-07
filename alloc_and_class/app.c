#include<stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define FILE  "/dev/test"
int main()
{
	int  i;
	char ubuf[100];
	int fd;
	fd=open(FILE,O_RDWR);
	if (fd < 0)
	{
		printf("open %s error.\n", FILE);
		return -1;
	}
	printf("open %s success..\n", FILE);
	
	
	while(1)
	{
		printf("please enter one of 1,2and3.\n");
	    scanf("%d",&i);
		memset(ubuf,0,sizeof(ubuf));
		switch(i)
		{
		case 1:
			ubuf[0]='1';
			write(fd,ubuf,1);break;
		case 2:
			ubuf[0]='0';
			write(fd,ubuf,1);
			break;
		case 3:
			for(i=0;i<10;i++)
			{
				ubuf[0]='0';
			write(fd,ubuf,1);
			sleep(1);
			ubuf[0]='1';
			write(fd,ubuf,1);
			sleep(1);
			}
			break;
		case 4:
			return 0;
		default :
			printf("error,please enter again!");break;
		}	
	}
	close(fd);
	return 0;	
}




