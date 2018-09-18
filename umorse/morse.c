#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morse.h"

int main()
{
	char mstring[50];		//holds string from stdin
	int i = 0;			//counter
	int k = 0;			//counter
	int checksum = 0;		//total of checksum
	int input_length = 0;		//length of stdin string
	int morse_length = 0;		//length of converted morse string
	char constring[17];		//holds morse string from char
	while(1) {
		fgets(mstring, 50,  stdin);	//get string from stdin
		input_length = strlen(mstring);		//get input length
		checksum = 1;		//checksum starts at 1 for preamble
		printf("__*_");
		for(i = 0; i < input_length - 1; i++){
			strcpy(constring, convert(mstring[i]));
			morse_length = strlen(constring);
			for(k = 0; k < morse_length; k++){
				if (constring[k] == '*') checksum += 1;
				printf("%c", constring[k]);
			}
			if (i == (input_length - 2)){
				goto skip;
			}
			printf("__");	
			skip:;
			printf("_"); 
		}
		checksumfunc(checksum);
		printf("\n");
	}
	return 0;
}


void checksumfunc(int sum){
	unsigned int j = 0;	//used to check each bit
	char sumarray[8];	//holds checksum string
	unsigned int count = 7;	//array iterator, counts down from 8(8bits)
	for (j = 1 << 7; j > 0; j = j / 2){
		if (sum & j){
			sumarray[count] = '_';
		}
		else {
			sumarray[count] = '*';
		}
		//printf("%c\n", sumarray[count]);
		count -= 1;
	}
	for (j = 0; j < 8; j++){
	       printf("%c", sumarray[j]);
	}
	return;	
}

//An extremely lazy function made up of if statements
const char * convert(char pass){
	
	if (pass == 'A') return "*_***";
	if (pass == 'B') return "***_*_*_*";
	if (pass == 'C') return "***_*_***_*";
	if (pass == 'D') return "***_*_*";
	if (pass == 'E') return "*";
	if (pass == 'F') return "*_*_***_*";
	if (pass == 'G') return "***_***_*";
	if (pass == 'H') return "*_*_*_*";
	if (pass == 'I') return "*_*";
	if (pass == 'J') return "*_***_***_***";
	if (pass == 'K') return "***_*_***";
	if (pass == 'L') return "*_***_*_*";
	if (pass == 'M') return "***_***";
	if (pass == 'N') return "***_*";
	if (pass == 'O') return "***_***_***";
	if (pass == 'P') return "*_***_***_*";
	if (pass == 'Q') return "***_***_*_***";
	if (pass == 'R') return "*_***_*";
	if (pass == 'S') return "*_*_*";
	if (pass == 'T') return "***";
	if (pass == 'U') return "*_*_***";
	if (pass == 'V') return "*_*_*_***";
	if (pass == 'W') return "*_***_***";
	if (pass == 'X') return "***_*_*_***";
	if (pass == 'Y') return "***_*_***_***";
	if (pass == 'Z') return "***_***_*_*";
	if (pass == '1') return "*_***_***_***_***";
	if (pass == '2') return "*_*_***_***_***";
	if (pass == '3') return "*_*_*_***_***";
	if (pass == '4') return "*_*_*_*_***";
	if (pass == '5') return "*_*_*_*_*";
	if (pass == '6') return "***_*_*_*_*";
	if (pass == '7') return "***_***_*_*_*";
	if (pass == '8') return "***_***_***_*_*";
	if (pass == '9') return "***_***_***_***_*";
	if (pass == '0') return "***_***_***_***_***";
	if (pass == ' ') return "_";
	return 0;
}

