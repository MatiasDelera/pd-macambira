#!/usr/bin/perl -w

use Switch;

#========================================================================
# FUNCTIONS
#========================================================================

#------------------------------------------------------------------------
# parse out the types and codes from a line from the header 
# usage:  @dataArray getDataFromHeaderLine($lineFromHeader);
sub getDataFromHeaderLine 
{
	 $_ = shift;

	 my @returnArray;

	 if ( m/#define ([A-Z0-9_]*)\s+(0x)?([0-9a-f]+)/) 
	 { 
		  if ($2) { $index = hex($3); } else { $index = $3; }
		  if ($index >=0) 
		  {
				$returnArray[0] = $index;
				$returnArray[1] = lc("$1");
				return @returnArray;
		  }
#		  print "$1 \t\t\t $index   $#returnArray\n "; 
#		  if ($index == 0) { print("index: $index   3: $3  2: $2   1: $1   value: $returnArray[1]\n"); }
	 } 
}

#------------------------------------------------------------------------
# declare each array in the header
#
sub printCArrayDeclarations
{
	 my @arrayToPrint = @_;
	 print(HEADER "char *${arrayToPrint[0]}[$#arrayToPrint];\n");
}

#------------------------------------------------------------------------
# print an array out in C format
#
sub printCArray
{
	 my @arrayToPrint = @_;

#	 print("$arrayToPrint[0] $#arrayToPrint \n");

	 print(ARRAYS "int ${arrayToPrint[0]}_total = $#arrayToPrint;  /* # of elements in array */\n");
	 print(ARRAYS "char *${arrayToPrint[0]}[$#arrayToPrint] = {");

	 for($i = 1; $i < $#arrayToPrint; $i++)
	 {
		  # format nicely in sets of 6
		  if ( ($i+4)%6 == 5 ) { print(ARRAYS "\n       "); }
		  # if the array element's data is null, print NULL
		  if ($arrayToPrint[$i]) { print(ARRAYS "\"$arrayToPrint[$i]\","); }
		  else { print(ARRAYS "NULL,"); }
	 }

	 print(ARRAYS "\"$arrayToPrint[$#arrayToPrint]\"\n };\n\n\n");
}

#------------------------------------------------------------------------
# print an array out in a comment table in Pd
#
sub printPdFile
{
	 my @arrayToPrint = @_;
	 my $x;
	 my $y;
	 my $lineNum = 1;

	 my $PDFILENAME = "$arrayToPrint[0]-list.pd";
	 open(PDFILE, ">$PDFILENAME");

	 print(PDFILE "#N canvas 282 80 210 570 10;\n");
	 if ($arrayToPrint[0] eq "ev") { print(PDFILE "#X text 5 5 Event Types;\n"); }
	 else { print(PDFILE "#X text 5 5 Codes for Type: $arrayToPrint[0];\n"); }
	 print(PDFILE "#X text 5 20 ----------------------------;\n");

	 for($i = 1; $i <= $#arrayToPrint; $i++)
	 {
		  # if the array element's data is null, print NULL
		  if ($arrayToPrint[$i]) 
		  { 
				$x = 5;
				$y = $lineNum * 20 + 20;
				print(PDFILE "#X text $x $y $arrayToPrint[$i];\n"); 
				$lineNum++;
		  }
	 }

	 close(PDFILE);
}

#------------------------------------------------------------------------
# 
#
sub printArray
{
	 printPdFile(@_);
	 printCArray(@_);
	 printCArrayDeclarations(@_);
}

#========================================================================
# MAIN
#========================================================================

# source file
$SOURCEFILENAME = "linux/input.h";
open(INPUT_H, "<$SOURCEFILENAME");

# output files
$HEADERFILENAME = "input_arrays.h";
open(HEADER, ">$HEADERFILENAME");
$ARRAYSFILENAME = "input_arrays.c";
open(ARRAYS, ">$ARRAYSFILENAME");


#----------------------------------------
# create the arrays from INPUT_H
while (<INPUT_H>)
{
	 if (m/\#define (FF_STATUS|[A-Z_]*?)_/)
	 {
# filter EV_VERSION and *_MAX
		  m/\#define\s+(EV_VERSION|[A-Z_]+_MAX)\s+/;
#		  print "$1 \n";
		  switch ($1) 
		  {
				# types
				case "EV"        { ($index, $value) = getDataFromHeaderLine($_); $EV[$index] = $value; }
				# codes
				case "SYN"       { ($index, $value) = getDataFromHeaderLine($_); $SYN[$index] = $value; }
				case "KEY"       { ($index, $value) = getDataFromHeaderLine($_); $KEY[$index] = $value; }
# BTN codes are actually part of the KEY type
				case "BTN"       { ($index, $value) = getDataFromHeaderLine($_); $KEY[$index] = $value; }
				case "REL"       { ($index, $value) = getDataFromHeaderLine($_); $REL[$index] = $value; }
				case "ABS"       { ($index, $value) = getDataFromHeaderLine($_); $ABS[$index] = $value; }
				case "MSC"       { ($index, $value) = getDataFromHeaderLine($_); $MSC[$index] = $value; }
				case "LED"       { ($index, $value) = getDataFromHeaderLine($_); $LED[$index] = $value; }
				case "SND"       { ($index, $value) = getDataFromHeaderLine($_); $SND[$index] = $value; }
				case "REP"       { ($index, $value) = getDataFromHeaderLine($_); $REP[$index] = $value; }
				case "FF"        { ($index, $value) = getDataFromHeaderLine($_); $FF[$index] = $value; }
# there doesn't seem to be any PWR events yet...
#				case "PWR"       { ($index, $value) = getDataFromHeaderLine($_); $PWR[$index] = $value; }
				case "FF_STATUS" { ($index, $value) = getDataFromHeaderLine($_); $FF_STATUS[$index] = $value; }
#				else { print " none $_"; } 
		  }
	 }
}

#----------------------------------------
# create the files from the arrays

# print file headers
print(ARRAYS "#include \"hid.h\"\n\n");

print(HEADER "\#ifndef _INPUT_ARRAYS_H\n");
print(HEADER "\#define _INPUT_ARRAYS_H\n\n\n");

# generate a C array for each array and stick them all in the same file
printArray("ev",@EV);
printArray("ev_syn",@SYN);
printArray("ev_key",@KEY);
printArray("ev_rel",@REL);
printArray("ev_abs",@ABS);
printArray("ev_msc",@MSC);
printArray("ev_led",@LED);
printArray("ev_snd",@SND);
printArray("ev_rep",@REP);
printArray("ev_ff",@FF);
# there doesn't seem to be any PWR events yet...
#printArray("pwr",@PWR);
print(ARRAYS "char *ev_pwr[1] = { NULL };\n\n");
print(HEADER "char *ev_pwr[1];\n");
#
printArray("ev_ff_status",@FF_STATUS);

# print array of arrays
print(ARRAYS "char **event_names[",$#EV+1,"] = {");
for($i = 0; $i < $#EV; $i++)
{
	 # format nicely in sets of 6
	 if ( ($i+4)%6 == 5 ) { print(ARRAYS "\n       "); }

	 # if the array element's data is null, print NULL
	 if ($EV[$i]) 
	 { 
		  $_ = $EV[$i];
		  m/(ev_[a-z_]+)/;
		  print(ARRAYS "$1,");  
	 }
	 else { print(ARRAYS "NULL,"); }
}
$_ = $EV[$#EV];
m/(ev_[a-z_]+)/;
print(ARRAYS "$1\n };\n");

# print file footers
print(HEADER "\n\n\#endif  /* #ifndef _INPUT_ARRAYS_H */\n");

close(ARRAYS);
close(HEADER);
close(INPUT_H);

