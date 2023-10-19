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
	
	// enable timer
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->PSC = 7199;
	TIM2->ARR = 100;			// 10ms timeout
	TIM2->SR = 0;
	TIM2->CR1 = 1;
	TIM2->CR1 |= TIM_CR1_CEN;
	NVIC_EnableIRQ(TIM2_IRQn);

	// SETUP END
	
	serial_open();
	
	uint8_t received[5];
	
	welcomeScreen();
	while(1)
	{
		CLI_Receive(received, 5);
		CLI_Process(received);
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
