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

void read_and_print_pronto(FILE *input, int output);

int main(int argc, char *argv[])
{
	int fout = 0;		// Device file descriptor
	FILE *fin;		// Output file pointer
	struct termios usb;     // Termios struct	

	// Expecting two arguments: a device file and an output file
	if (argc != 3) {
		printf("Error: expected two arguments\n");
		return 1;
	}

	// Open the device file
	printf("Opening device file %s ...\n", argv[1]);
	fout = open(argv[1], O_RDWR);
	if (fout <= 0) {
		printf("Error: failed to open file %s\n", argv[1]);
		return 2;
	}
	printf("Device file opened ... \n");

	// Verify that a TTY device was opened
	if(!isatty(fout)) {
		printf("Error: device file %s is not a TTY device\n", argv[1]);
		return 3;
	}

	// Open input file
	printf("Opening output file %s ...\n", argv[2]);
	fin = fopen(argv[2], "r");
	if(fin == NULL) {
		printf("Error: failed to open file %s\n", argv[2]);
		return 4;
	}
	printf("Opened output file ... \n");

	// Clear termios struct
	memset(&usb, 0, sizeof(usb));

	// Read existing serial parameters
	if (tcgetattr(fout, &usb) != 0) {
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
	if (tcsetattr(fout, TCSANOW, &usb) != 0) {
		printf("Error: failed to set serial port attributes\n");
		return 6;
	}

	// Clear any preexisting data on the serial interface (input and output)
	tcflush(fout, TCIOFLUSH);



	// Begin transmission
	printf("Reading device output ... \n");

	
	// Loops through all of the data in the input file, and then immediately
	// writes the data to the output file (device file). 
	read_and_print_pronto(fin, fout);

	fclose(fin);
	close(fout);
	return 0;			
}

// Read first four unique codes, then loop through the rest of the file
// a number of times equal to the number of pairs sent * 2.
void read_and_print_pronto(FILE *input, int output) 
{
	char charbuf[6];
	char pairbuf[6];
	char hex[8] = {'0', 'x'};
	int pairs = 0;
	int i = 0;
	
	// Read initial set of zeros
	fscanf(input, "%s", charbuf);	
	write(output, charbuf, strlen(charbuf)); 

	// Read frequency and sends data
	fscanf(input, "%s", charbuf);	
	write(output, charbuf, strlen(charbuf)); 

	// Read Number of pairs and send data
	fscanf(input, "%s", pairbuf);	
	strcat(hex, pairbuf);
	pairs = strtol(pairbuf, NULL, 16);
	write(output, charbuf, strlen(charbuf));

	// Read final set of zeros and send data
	fscanf(input, "%s", charbuf);	
	write(output, "%s", strlen(charbuf));

	for (i = 0; i < (pairs *2) + 1; i++) {
		fscanf(input, "%s", charbuf);
		write(output, "%s", strlen(charbuf));
		usleep(50000);
	}

	return;
}

