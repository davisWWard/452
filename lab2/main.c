#include "usart.h"
#include "CLI.h"

void welcomeScreen(void);

int main()
{
	// SETUP
	// enable clocks for USART2 and port A 
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	
	// get GPIOA to output, push pull
	GPIOA->CRL |= GPIO_CRL_MODE5;
	GPIOA->CRL &= ~GPIO_CRL_CNF;
	// SETUP END
	
	serial_open();
	
	uint8_t received[5];
	uint8_t byte;
	
	welcomeScreen();
	while(1)
	{
		CLI_Receive(received, 5);
		CLI_Transmit(received, 1);
	}
}


void welcomeScreen(void)
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
