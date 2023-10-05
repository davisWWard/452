#include "CLI.h"
#include "usart.h"

void CLI_Transmit(uint8_t *pData, uint16_t Size)
{
	
	uint8_t byte;
	// help state
	if (pData[0] == 'h' && pData[1] == 'e' && pData[2] == 'l' && pData[3] == 'p' && pData[4] == '\r')
	{
		char message[] = "\n\rHelp Screen\n\r"
										"Type \"on\" to turn the LED on\n\r"
										"Type \"off\" to turn the LED off\n\r"
										"Type \"state\" to query the state of the LED\n\r";
		for (int i = 0; i < sizeof(message); i++)
		{
			char send = message[i];
			sendByte(send);
			while ((USART2->SR & USART_SR_TXE) == 0);		// wait for transmission to be complete
		}
	}
	// turn led on
	else if (pData[0] == 'o' && pData[1] == 'n' && pData[2] == '\r' && pData[3] == '\r' && pData[4] == '\r')
	{
		GPIOA->BSRR |= GPIO_BSRR_BS5;
		char message[] = "\nTurning on LED\n\r";
		for (int i = 0; i < sizeof(message); i++)
		{
			char send = message[i];
			sendByte(send);
			while ((USART2->SR & USART_SR_TXE) == 0);		// wait for transmission to be complete
		}
	}
	// turn led off
	else if (pData[0] == 'o' && pData[1] == 'f' && pData[2] == 'f' && pData[3] == '\r' && pData[4] == '\r')
	{
		GPIOA->BSRR |= GPIO_BSRR_BR5;
		char message[] = "\nTurning off LED\n\r";
		for (int i = 0; i < sizeof(message); i++)
		{
			char send = message[i];
			sendByte(send);
			while ((USART2->SR & USART_SR_TXE) == 0);		// wait for transmission to be complete
		}
	}
	// state query
	else if (pData[0] == 's' && pData[1] == 't' && pData[2] == 'a' && pData[3] == 't' && pData[4] == 'e')
	{
		// led on
		if ((GPIOA->ODR & GPIO_ODR_ODR5) != 0)
		{
			char message[] = "\n\rThe LED is on\n\r";;
			for (int i = 0; i < sizeof(message); i++)
			{
				char send = message[i];
				sendByte(send);
				while ((USART2->SR & USART_SR_TXE) == 0);		// wait for transmission to be complete
			}
		}
		// led off
		else
		{
			char message[] = "\n\rThe LED is off\n\r";
			for (int i = 0; i < sizeof(message); i++)
			{
				char send = message[i];
				sendByte(send);
				while ((USART2->SR & USART_SR_TXE) == 0);		// wait for transmission to be complete
			}
		}
	}
	// empty line
	else if (pData[0] == '\r' && pData[1] == '\r' && pData[2] == '\r' && pData[3] == '\r' && pData[4] == '\r')
	{
		char message[] = "\n\rWelcome to the STM32F103RB CLI!\n\r"
										"Type \"help\" for help\n\r";
		for (int i = 0; i < sizeof(message); i++)
		{
			char send = message[i];
			sendByte(send);
			while ((USART2->SR & USART_SR_TXE) == 0);		// wait for transmission to be complete
		}
	}
	// not a valid command
	else 
	{
		char message[] = "\n\rThat is not a valid command\n\r";
		for (int i = 0; i < sizeof(message); i++)
		{
			char send = message[i];
			sendByte(send);
			while ((USART2->SR & USART_SR_TXE) == 0);		// wait for transmission to be complete
		}
	}

}

void CLI_Receive(uint8_t *pData, uint16_t Size)
{
	uint8_t byte;
	for (int i = 0; i <= Size; i ++)
	{
			if (i < 0)				// dont let i go below 0
				i = 0;
			if (byte != '\r')	// if enter is pressed, stop taking input
				byte = getByte();
			pData[i] = byte;
			sendByte(pData[i]);
			//sendByte(i);
			//sendByte(i);
			//sendByte(byte);
			
				
			if (byte == 8)		// backspace character
				i = i - 2;
		//}
	}
}
