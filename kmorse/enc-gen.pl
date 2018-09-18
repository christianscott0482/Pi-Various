#!/usr/bin/perl

use warnings;

my $input = shift;	#reads input argument
my $outputc = shift;	#reads output argument (encoding.c)
my $outputh = shift;	#reads output argument (encoding.h)

#opens files to read or be written to
open (my $in, "<", "$input") or die;
open (my $outc, ">", "$outputc") or die;
open (my $outh, ">", "$outputh") or die;

#variable defines
my %data = ();		#all encoded data (will likely become hash)
my $bmstring;		#encoded line before being pushed to morselist
my $counter = 0;	#counter for number of lines pushed to morselist
my $mchar;		#holds the character being converted

#create a list of the data keys, including numbers representing all the 
#characters not included in the morse alphabet
#The first nine ascii characters are typed out to avoid issues with 
#double values later in the code
#
#9 numbers were arbitrarily chosen to represent 0-9 on the ascii table
#(not numbers 0-9) to be 65-73
my @extrac = ("1", "2", "3", "4", "5", "6", "7", "8", "9", "0", 10..64, "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", 91..255);

#initialize all hash values with 0, hash is filled with 
#256 ASCII characters 
foreach $mchar (@extrac){
	$data{$mchar} = 0;
}


#sorts through input file and converts each line into
#the specified format, and puts the data in a hash sorted
#by the character it's associated with
while(my $myline = <$in>){	#go through until <eof>
	my @line = split(//,$myline);	
	$bmstring = "";		#initialize
	for(my $i = $#line; $i > 1; $i--){
		if ($line[$i] eq "-"){
			$bmstring = $bmstring . "1";
		}
		if ($line[$i] eq "*"){
			$bmstring = $bmstring . "1";
		}
		if ($line[$i] eq " "){
			$bmstring = $bmstring . "0";
		}
	}
	$mchar = $line[0];		#retrieve key/character
	$data{$mchar} = $bmstring;	#input data to hash
	$counter += 1; 
	next;

}
#hand coded binary counter
my @bincount = ("0");		#string of binary count
my $binsize = 0;	#current size of binary count
my $rev;		#reverse iterator
my $maxcheck = 0;	#a flag that says if the binary is at all
			#1's or not
my $bin = "";		#binary when converted to morse format
my $flag = 0;		#flag to say if a binary matches a character from
			#the morse alphabet
$mchar = 0;
$counter = 0;

#for ($i = 0; $i < $#extrac; $i++){
#	print "$data{$extrac[$i]}\n";
#}


#start "binary" string counter through the list of extra characters plus
#the 36 characters already standardized in morse.
for (my $j = 0; $j < ($#extrac + 1 + 36); $j++){
	
	$bin = "";	#initialize
	$maxcheck = 1;	#1 = not max, 0 = max
	$flag = 0;	#flag to check if a generated binary number is
			#equal to one that's already in the morse alphabet

	for(my $k = $#bincount; $k >= 0; $k--){
		#the first zero will always become 1
		if ($bincount[$k] eq "0"){
			@bincount[$k] = "1";
			#whenever the first zero is found, lower one's 
			#must become zero's, then loop is broken out of
			for ($rev = $k + 1; $rev <= $#bincount; $rev++){
                                $bincount[$rev] = "0";
                        }
			$k = -1;		#break out of loop
			$maxcheck = 0;
		}	
	}
	#if no zero's found in the string, add a digit and clear bits
	if ($maxcheck == 1){
		for($k = 1; $k <= $#bincount; $k++){
			@bincount[$k] = "0";
		}
		push @bincount, "0";
		$binsize += 1;
	}
	#assume 1 is a 'dit' -> 1; 0 is a 'dah' -> 111; '' is a 0
	#This code converts the binary number to that format
	for ($i = 0; $i <= $#bincount; $i++){
		if ($bincount[$i] eq "1"){
			if ($i == $#bincount){
				$bin = $bin . 1;
			}
			else{
				$bin = $bin .1 . 0;
			}
		}
		if ($bincount[$i] eq "0"){
			if ($i == $#bincount){
				$bin = $bin . 1 . 1 . 1;
			}
			else {
				$bin = $bin . 1 . 1 . 1 . 0;
			}
		}
	}

	#This code goes through each key and checks for doubles
	#of binary, and sets if it isn't a double
	for ($i = $#extrac; $i >= 0; $i--){
		if($bin eq $data{$extrac[$i]}){
			$flag = 1;
		}
	}
	if ($flag == 1){
		if ($data{$counter} == 0){ 
			$counter -= 1;
		}
	}
	else {
			$data{$counter} = $bin;
	}
	$counter += 1;
	
}
#prints some c code in maine file
print $outc "#include <stdio.h>\n";
print $outc "#include <stdlib.h>\n";
print $outc "struct morsechar{\n";
print $outc "        int morsekey;\n";
print $outc "        int morsecode;\n";
print $outc "};\n\n";



#prints each element of data
foreach $mchar (@extrac){
	print $outc "int main()\n";
	print $outc "{\n";
	print $outc "	struct morsechar list[256];\n\n";
	print $outc "	//Declaring struct of $mchar\n";
	print $outc "	struct morsechar ";
	print $outc "	m$mchar;\n";
	print $outc "	m$mchar.morsekey = $mchar;\n";
	print $outc "	m$mchar.morsecode = 0b$data{$mchar};\n\n";
}

#header file stuff
print $outh "#ifndef MORSE_CON\n";
print $outh "#define MORSE_CON\n";
print $outh "int morse_con(morsechar * mchar);\n";
print $outh "#endif";

close $in;
close $outc;
close $outh;
