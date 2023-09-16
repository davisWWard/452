/*****************************************************************************
			Davis Ward
			200626114
			ENEL 452
			Assignment 1
			452_as1_200426114.cpp

			This program takes a character input to indicate the
			operation to undergo, and 4 integer inputs for the numbers
			to undergo the operation
			ex. "a 1 2 3 4" is (1 + 2j) + (3 + 4j)
*****************************************************************************/

#include "452_as1_200426114.h"


void add(Complex num1, Complex num2, Complex* result)
/*	Add two complex numbers */
{
	result->real = num1.real + num2.real;
	result->imag = num1.imag + num2.imag;

}

void sub(Complex num1, Complex num2, Complex* result)
/*	Subtract two complex numbers */
{
	result->real = num1.real - num2.real;
	result->imag = num1.imag - num2.imag;

}

void mul(Complex num1, Complex num2, Complex* result)
/*	Multiply two complex numbers */
{
	result->real = (num1.real * num2.real) - (num1.imag * num2.imag);
	result->imag = (num1.real * num2.imag) + (num1.imag * num2.real);

}

void div(Complex num1, Complex num2, Complex* result)
/*	Divide two complex numbers*/
{
	result->real = ((num1.real * num2.real) + (num1.imag * num2.imag))
		/ ((num2.real * num2.real) + (num2.imag * num2.imag));
	result->imag = ((num1.imag * num2.real) - (num1.real * num2.imag))
		/ ((num2.real * num2.real) + (num2.imag * num2.imag));

}

bool checkDiv(Complex num1, Complex num2)
/*	Check if the attempted division is legal
*	throw an error if x / 0 is attempted 
*	where x is not 0	*/
{
	if (((num2.real == 0) && (num1.real != 0))
		|| ((num2.imag == 0) && (num1.imag != 0)))
	{
		fprintf(stderr, "error code: 4: divide by zero\n");
		return 0;
	}
	return 1;
}

bool checkNumbers(Complex num1, Complex num2)
/*	Check the entered numbers to ensure there are no null values
*	If there is a null, return an error	*/
{
	if ((num1.real == 0xFFFFFFFF) || (num1.imag == 0xFFFFFFFF)
		|| (num2.real == 0xFFFFFFFF) || (num2.imag == 0xFFFFFFFF))
	{
		fprintf(stderr, "error code: 2: missing arguments\n");
		return 0;
	}
	return 1;
}

bool processRequest(char input, Complex num1, Complex num2, Complex* result)
/*	Proccess the request. If the operation character "input" is an
*	invalid operation, return an error
*/
{
	if (input == 'a' || input == 'A')
		// input is a, meaning add
	{
		add(num1, num2, result);
	}
	else if (input == 's' || input == 'S')
		// input is s, meaning subtract
	{
		sub(num1, num2, result);
	}
	else if (input == 'm' || input == 'M')
		// input is m, meaning multiply
	{
		mul(num1, num2, result);
	}
	else if (input == 'd' || input == 'D')
		// input is d, meaning divide
	{
		if (checkDiv(num1, num2))
		{
			div(num1, num2, result);
		}
		else 
			return 0;
	}
	else
	{
		// improper operater argument
		fprintf(stderr, "error code: 1: illegal command\n");
		return 0;
	}
	result->calculateSign();
	return 1;
}
