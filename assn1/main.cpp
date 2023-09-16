/*****************************************************************************
			Davis Ward
			200626114
			ENEL 452
			Assignment 1
			main.cpp

			This program takes a character input to indicate the 
			operation to undergo, and 4 integer inputs for the numbers
			to undergo the operation
			ex. "a 1 2 3 4" is (1 + 2j) + (3 + 4j)
*****************************************************************************/

#include "452_as1_200426114.h"


int main()
{
	char input = ' ';

	Complex complex3(0, 0);
	float real1, real2, imag1, imag2, extra;
	char str[60];
	
	fprintf(stdout, "Complex Calucator \n \n"
		" Type a letter to specify the arithmetic operator (a, s, m, d)\n"
		" followed by two complex numbers expressed as pairs of doubled\n"
		" ex. a 1 2 3 4 equals (1 + j 2) + (3 + j 4)\n"
		" ype Q or q to quit.\n\n");

	while ((input != 'Q') && (input != 'q'))
	// runing loop of the program
	// loop will end if q or Q is entererd, thus ending the program,
	{
		// reset all input values. 
		//NULL is not used due to my implementation of error checking
		input = ' ';
		real1 = 0xFFFFFFFF;
		real2 = 0xFFFFFFFF;
		imag1 = 0xFFFFFFFF;
		imag2 = 0xFFFFFFFF;
		extra = 0xFFFFFFFF;

		fprintf(stdout, "Enter expression: ");
		// take the input from the command line and enter it into str
		fgets(str, 60, stdin);
		// remove white space from str by shifting all characters left
		// while the front character is a whitespace
		char temp = str[0];
		while (temp == ' ')
		{
			for (int i = 0; i < 59; i++)
			{
				str[i] = str[i + 1];
			}
			temp = str[0];
		}
		// scan str and divide its contents into the nessessary variables
		sscanf(str, "%c %f %f %f %f %f", &input, &real1,
			&imag1, &real2, &imag2, &extra);

		Complex complex1(real1, imag1);
		Complex complex2(real2, imag2);
		if ((input != 'Q') && (input != 'q'))
		{

			// extra will be 0xFFFFFFFF if there are no extra arguments given
			if (extra != 0xFFFFFFFF)
			{
				fprintf(stderr, "error code: 3: extra arguments\n");
			}
			// check the numbers to ensure there are the proper number of entries
			else if (checkNumbers(complex1, complex2))
			{
				// process the numbers if there are proper arguments
				if (processRequest(input, complex1, complex2, &complex3))
				{
					// print the result
					fprintf(stdout, "%g %c j %g\n\n", complex3.real, complex3.sign, complex3.imag);
				}
			}
		}
	}
}
