/*****************************************************************************
			Davis Ward
			200626114
			ENEL 452
			Assignment 1
			452_as1_200426114.h

			This program takes a character input to indicate the
			operation to undergo, and 4 integer inputs for the numbers
			to undergo the operation
			ex. "a 1 2 3 4" is (1 + 2j) + (3 + 4j)
*****************************************************************************/
#pragma once
#include <stdio.h>
#include <string>
using namespace std;

struct Complex
{
	float real;
	float imag;
	char sign = '+';

	Complex(float Real, float Imag)
	{
		real = Real;
		imag = Imag;
	}
	void calculateSign()
	// calculate the sign for imag, then abs imag
	// this is used to simplify printing the value
	{
		if (imag < 0) sign = '-';
		else sign = '+';
		imag = abs(imag);
	}
};

void add(Complex, Complex, Complex*);
void sub(Complex, Complex, Complex*);
void mul(Complex, Complex, Complex*);
void div(Complex, Complex, Complex*);

bool checkDiv(Complex, Complex);
bool checkNumbers(Complex, Complex);
bool processRequest(char, Complex, Complex, Complex*);