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

int read_IR(int file, char *session);
void print_pronto();

int main(int argc, char *argv[])
{
	int fd = 0;		// Device file descriptor
	FILE *out;		// Output file pointer
	struct termios usb;     // Termios struct	
	int pairsx = 0;		// Number of pairs sent
	int pairsy = 0;
	char prontox[300];	// stores transmitted data
	char prontoy[300];	
	char prontoavg[300];	//avg pronto data
	int x = 0;		//stores a value of prontox
	int y = 0;		//stores a value of prontoy
	int k = 0;		//interator
	int avg = 0;		//stores a value of prontoavg
	int error = 0;		//holds standard deviation
	int temp = 0;		//holds a value

	// Expecting two arguments: a device file and an output file
	if (argc != 3) {
		printf("Error: expected two arguments\n");
		return 1;
	}

	// Open the device file
	printf("Opening device file %s ...\n", argv[1]);
	fd = open(argv[1], O_RDWR);
	if (fd <= 0) {
		printf("Error: failed to open file %s\n", argv[1]);
		return 2;
	}
	printf("Device file opened ... \n");

	// Verify that a TTY device was opened
	if(!isatty(fd)) {
		printf("Error: device file %s is not a TTY device\n", argv[1]);
		return 3;
	}

	// Open output file
	printf("Opening output file %s ...\n", argv[2]);
	out = fopen(argv[2], "w");
	if(out == NULL) {
		printf("Error: failed to open file %s\n", argv[2]);
		return 4;
	}
	printf("Opened output file ... \n");

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

	// Begin transmission
	printf("Reading device output ... \n");


	printf("Test the IR code until you get the same\n");
	printf("number of pairs twice in a row.\n");

	//start reading IR codes, and asks for more until two 
	//sets of IR codes that are similar have been read in a row
	//by checking the total number of pairs.
	while (1) {
		pairsx = read_IR(fd, prontox);
		if(pairsx == pairsy) {
			printf("Matching pairs found!\n");
			break;
		}
		printf("Pair not found, try again.\n");
		pairsy = read_IR(fd, prontoy);
		if(pairsx == pairsy) {
			printf("Matching pairs found!\n");
			break;
		}
		printf("Pair not found, try again.\n");
	}

	//Goes through each value of both sets of pronto codes, then
	//averages the two values, and also calculates a form of error
	//throughout the logic.
	for (k = 0; k < ((pairsx * 2) + 1); k++) {
		x = prontox[k];
		y = prontoy[k];
		avg = ((x + y) / 2);
		temp = (x - y);
		error += abs(temp);
		prontoavg[k] = avg;
	}
	error = (error / ((pairsx * 2) + 1));	
	print_pronto(pairsx, prontoavg, out);
	fprintf(out, "Error = %d\n", error);
	printf("Error = %d\n", error);

	fclose(out);
	close(fd);
	return 0;			
}


//Reads the buffer from the virtual serial port, and organizes it into pronto
//code format. It's also printed to the terminal for error checking.
int read_IR(int file, char *session)
{
	char c = 0;
	int sespair = 0;
	char charbuf[6];
	int chunk = 0;
	int count = 0;
	int j = 0;

	while (1) {
		j = 0;
                while(1) {
			read(file, &c, 1); 
		        if ((c == '\r') || (c == '-')) break;
		        charbuf[j] = c;
		        j++;
		}
                chunk = atoi(charbuf);
                if (c == '-') break;
                count += 1;
                session[count] = chunk;
                printf("%04x\n", chunk);
    
        }
        sespair = (count / 2); 
	printf("pairs: %d\n", sespair);
	return sespair;
}

//A simple function that prints the specified array(or set of pronto codes in
//this case) with a preample to the specified output file. 
void print_pronto(int pair, char *session, FILE *output)
{
	int i = 0;
	//preample
	fprintf(output, "0000\n"); //first four words, always zero
	fprintf(output, "006d\n"); //prints frequency of the signal, decimal 38
	fprintf(output, "%04x\n", pair); //print number of pairs in this test
	fprintf(output, "0000\n"); //fourth set is all zeros

	//pronto code
	for(i = 0; i < ((pair * 2) + 1); i++) {
		fprintf(output, "%04x\n", session[i]); 
	}
	return;
}


