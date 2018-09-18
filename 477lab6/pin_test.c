/* Matthew Blanchard & Christian Auspland
 * ECE 477
 * AVR Pin Manager user program
 * Calls upon a USB serial connection to the AVR to 
 * read and modify pin values
 */

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	FILE *fd;		//opens device
	FILE *fdwrite;	//opens file to write
	//struct termios usb;	
	char pronto[1024];
	int w_buf = 0;
	int freq = 0;		//frequency of IR signal
	int count = 0;		//counts received times
	int pairs = 0;		//number of pairs sent
	char end = 1;		//when this becomes zero, while loop will end
	int i = 0;

	// There should be a single argument (the file path for the device)
	if (argc != 3) {
		printf("Error: expected two arguments\n");
		return 1;
	}

	// Attempt to open file 
	fd = fopen(argv[1], "r");
	if (fd == NULL) {
		printf("Error: couldn't open file %s\n", argv[1]);
		return 2;
	}

	fdwrite = fopen(argv[2], "w");
	if(fdwrite == NULL) {
		printf("Error: couldn't open file %s\n", argv[2]);
		return 3;
	}

	/*/ Read serial port attributes
	memset(&usb, 0, sizeof(usb));
	if (tcgetattr(fd, &usb) != 0) {
		printf("Error: failed to get attributes for serial port\n");
		return 4;
	}

	// Set speeds
	cfsetospeed(&usb, B2400);
	cfsetispeed(&usb, B2400);

	// Set character size 
	usb.c_cflag = (usb.c_cflag & ~CSIZE) | CS8;
	
	usb.c_iflag &= ~IGNBRK;
	usb.c_lflag = 0;
	usb.c_oflag = 0;
	
	usb.c_iflag &= ~(IXON | IXOFF | IXANY);
	usb.c_cflag |= (CLOCAL | CREAD);

	usb.c_cflag &= ~(PARENB | PARODD);
	usb.c_cflag |= 1;
	usb.c_cflag &= ~CSTOPB;
	usb.c_cflag &= ~CRTSCTS;

	// Disable blocking
	usb.c_cc[VMIN] = 0;
	usb.c_cc[VTIME] = 5;

	if (tcsetattr(fd, TCSANOW, &usb)) {
		printf("Error: failed to set serial port attributes\n");
		return 5;
	}*/	
	fscanf(fd, "%d", &freq);	//frequency is sent first

 	while(end) {
		fscanf(fd, "%d", &w_buf);
		printf("%d\n", w_buf);
		if (w_buf == 111) break;
		pronto[count] = w_buf;
		count += 1;

	}
	pairs = count / 2;	//finds number of pairs
	
	//preamble
	fprintf(fdwrite, "0000\n");	//first four words, always zero
	fprintf(fdwrite, "%04x\n", freq);	//print frequency
	fprintf(fdwrite, "%04x\n", pairs);	//print number of pairs	
	fprintf(fdwrite, "0000\n");	//fourth set is all zeros
	
	for(i = 0; i < count; i++) {
		fprintf(fdwrite, "%04x\n", pronto[i]);
	}
	return 0;			
}
