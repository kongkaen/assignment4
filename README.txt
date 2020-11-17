/*	Natthaphong Kongkaew
		ASSIGNMENT 4 : Multi-threaded Producer Consumer Pipeline
		CS 344
		Oregon State University
*/

------------------------------------------
Files required to compile the Program
	1.	line_processor.c
	2.	Makefile
------------------------------------------

------------------------------------------
COMPILING
	Open terminal commands
	Go to directory of the Files
	Run "make"
------------------------------------------

------------------------------------------
TESTING
(1) Testing with keyboard input	from the following step
		run "./line_processor"
		type input after the new line
		press enter when finish the line
		when the input have >= 80 character, it will print the line cumulatively

(2) Testing with input File
		example, run "./line_processor < inputfile.txt"

(3) Testing with input File and print the result to output file
		example, run "./line_processor < input.txt > output.txt"
------------------------------------------
