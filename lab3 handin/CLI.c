#include "CLI.h"
#include "usart.h"

int pos = 0;
uint8_t rec[5];

void CLI_Transmit(uint8_t *pData, uint16_t Size)
{
		for (int i = 0; i < Size; i++)
		{
			char send = pData[i];
			sendByte(send);
			while ((USART2->SR & USART_SR_TXE) == 0);		// wait for transmission to be complete
		}
}


void CLI_Receive(uint8_t *pData, uint16_t Size)
{
	/*
	uint8_t byte;
	for (int i = 0; i <= Size; i ++)
	{
		while(dataIn == 0){}
		dataIn = 0;
		if (i < 0)				// dont let i go below 0
			i = 0;
		if (byte != '\r')	// if enter is pressed, stop taking input
			byte = getByte();
		pData[i] = byte;
		sendByte(pData[i]);
			
		if (byte == 8)		// backspace character
			i = i - 2;
	}
	*/
	while (pos < Size) {}
	pos = 0;
	for (int i = 0; i < Size; i++)
	{
		pData[i] = rec[i];	
	}
}


void USART2_IRQHandler()
{
	if (pos < 0)				// dont let i go below 0
		pos = 0;

	uint8_t byte = getByte();
	rec[pos] = byte;
	sendByte(rec[pos]);
	if (rec[pos] == 8)		// backspace character
	{
		pos = pos - 2;
		
	}
	if (rec[pos] == '\r')	// if enter is pressed, stop taking input
		pos = 10;
	pos++;
}


void CLI_Process(uint8_t *pData)
{
	// help state
	if (pData[0] == 'h' && pData[1] == 'e' && pData[2] == 'l' && pData[3] == 'p' && pData[4] == '\r')
	{
		uint8_t message[] = "\n\rHelp Screen\n\r"
										"Type \"on\" to turn the LED on\n\r"
										"Type \"off\" to turn the LED off\n\r"
										"Type \"state\" to query the state of the LED\n\r";
		CLI_Transmit(message, sizeof(message));
	}
	// turn led on
	else if (pData[0] == 'o' && pData[1] == 'n')
	{
		GPIOA->BSRR |= GPIO_BSRR_BS5;
		uint8_t message[] = "\nTurning on LED\n\r";
		CLI_Transmit(message, sizeof(message));		
	}
	// turn led off
	else if (pData[0] == 'o' && pData[1] == 'f' && pData[2] == 'f')
	{
		GPIOA->BSRR |= GPIO_BSRR_BR5;
		uint8_t message[] = "\nTurning off LED\n\r";
		CLI_Transmit(message, sizeof(message));
	}
	// state query
	else if (pData[0] == 's' && pData[1] == 't' && pData[2] == 'a' && pData[3] == 't' && pData[4] == 'e')
	{
		// led on
		if ((GPIOA->ODR & GPIO_ODR_ODR5) != 0)
		{
			uint8_t message[] = "\n\rThe LED is on\n\r";;
			CLI_Transmit(message, sizeof(message));
		}
		// led off
		else
		{
			uint8_t message[] = "\n\rThe LED is off\n\r";
			CLI_Transmit(message, sizeof(message));
		}
	}
	// empty line
	else if (pData[0] == '\r')
	{
		uint8_t message[] = "\n\rWelcome to the STM32F103RB CLI!\n\r"
										"Type \"help\" for help\n\r";
		CLI_Transmit(message, sizeof(message));
	}
	// not a valid command
	else 
	{
		uint8_t message[] = "\n\rThat is not a valid command\n\r";
		CLI_Transmit(message, sizeof(message));
	}
}
