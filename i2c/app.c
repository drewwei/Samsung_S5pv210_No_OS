#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void usage(char *file)
{
	printf("usage:%s argv[1]\n",file);
	printf("usage:%s argv[1] argv[2]\n",file);
}
int main(int argc, char **argv)
{
	if((argc != 3) &&(argc != 2))
	{
		usage(argv[0]);
		return -1;
	}
	int fd, ret;
	char buf[2];
	fd = open("/dev/my_i2c", O_RDWR);
	if(argc == 3)
	{
	buf[0] = strtoul(argv[1], NULL, 0);
	buf[1] = strtoul(argv[2], NULL, 0);
	write(fd, buf, 2);
	
	read(fd, &buf[0], 1);
	printf("OUT:%d\n",buf[0]);
	}
	else{
		buf[0] = strtoul(argv[1], NULL, 0);
		read(fd, &buf[0], 1);
		printf("OUT:%d\n",(buf[0]&0xfc)>>2);
	}
	close(fd);
	return 0;
}