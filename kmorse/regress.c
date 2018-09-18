#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main()
{
	int fd;
	int size;
	char buf[50];
	
	//open device file
	if ((fd = open("/dev/morse", O_WRONLY)) < 0) {
		printf("file not opened correctly\n");
		return -1;
	}

	//write to device file
	strcpy(buf, "test string\n");
	size = strlen(buf);
	if(write(fd, buf, size)){
		printf("write fail");
		return -1;
	}	

	//close device file
	close(fd);
	return 0;
}
