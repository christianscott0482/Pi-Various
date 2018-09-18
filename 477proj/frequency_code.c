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


#define BAUDRATE B9600

int convert(int note);

int main(int argc, char *argv[])
{
	int fd = 0;		// File descriptor for serial port
	FILE *fb = 0;		// File descriptor for song
	struct termios usb;     // Termios struct	
	char sound[7];		// holds stdout string
	char end[] = "\r\n";	// end characters to send to AVR
	char quartlen[4];	//length of quarter note in a song
	int beat = 0;
	int inote = 0;
	int cnote = 0;
	int i = 0;

	// Expecting two arguments: a device file and an output file
	if (argc != 3) {
		printf("Error: expected two arguments\n");
		return 1;
	}

	// Open the device file
	printf("Opening device file %s ...\n", argv[1]);
	fd = open(argv[1], O_RDWR);
	if (fd <= 0) {
		printf("Error: failed to open device file %s\n", argv[1]);
		return 2;
	}
	printf("Device file opened ... \n");

	// Open song file
	printf("Opening song file %s ...\n", argv[2]);
	fb = fopen(argv[2], "r");
	if (fb == NULL) {
		printf("Error: failed to open song file %s\n", argv[2]);
		return 3;
	}

	// Verify that a TTY device was opened
	if(!isatty(fd)) {
		printf("Error: device file %s is not a TTY device\n", argv[1]);
		return 4;
	}

	// Clear termios struct
	memset(&usb, 0, sizeof(usb));

	// Read existing serial parameters
	if (tcgetattr(fd, &usb) != 0) {
	 	printf("Error: failed to retrieve serial port attributes\n");
	 	return 5;
	}

	// Configure serial port //
	///////////////////////////

	// Transmission speeds
	cfsetospeed(&usb, B9600);	// Output baud 9600 bps
	cfsetispeed(&usb, B9600);	// Input baud 9600 bps

	// Line flags
	usb.c_lflag |= ICANON;	// Canonical mode

	// Character flags
	usb.c_cflag |= CREAD;	// Enable receiver

	usb.c_cflag &= ~PARENB; // Disable parity
	usb.c_cflag &= ~CSTOPB; // One stop bit

	usb.c_cflag &= ~CSIZE;  // Character size is 8	
	usb.c_cflag |= CS8;	

	// Update serial device parameters
	if (tcsetattr(fd, TCSANOW, &usb) != 0) {
		printf("Error: failed to set serial port attributes\n");
		return 6;
	}

	// Clear any preexisting data on the serial interface (input and output)
	tcflush(fd, TCIOFLUSH);

	
	// Get first line: the quarternote length for this song
	fscanf(fb, "%s", quartlen);
	write(fd, quartlen, strlen(quartlen));
	beat = atoi(quartlen);
	printf("Beat sent to AVR: %d\n", beat);


	// Begin transmission

	while(1) {
		printf("start of scan\n");
		fscanf(fb, "%s", sound);
		printf("after scan");
		inote = atoi(sound);
		printf("note read: %d\n", inote);
		if ((inote >= 0) && (inote < 65535)) {
			cnote = convert(inote);
			printf("after function note value: %d\n", cnote);
			sprintf(sound, "%d", cnote);
			strcat(sound, end);
			printf("your number is: ");
			for (i = 0; i < 7; i++) {
				printf("%c", sound[i]);
			}
			printf("\n");
			printf("before write\n");
			write(fd, sound, strlen(sound));
			printf("after write\n");
			memset(sound, 0, strlen(sound));
			printf("after memset\n");
			usleep(beat * 1000);
			printf("after sleep\n\n");
		}
		else {
			printf("Value out of range\n");
		}
		if (feof(fb)) {
			printf("End of file reached\n");
			break;
		}		
	}
	return 0;

}

int convert(int note)
{
	double math = 0;
	double math_note = 0;

	if (note <= 0) {
		return 0;
	}
	math_note = note;
	math = 1 / math_note;
	printf("note frequency in time: %f\n", math);
	math = (math * 1000000) - 1;
	printf("converted value: %f\n", math);
	return math;
}
