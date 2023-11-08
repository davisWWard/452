#include "functions.h"


void clockInit(void)
{
	// enable clocks for USART2 and port A 
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	
	// get GPIOA to output, push pull
	GPIOA->CRL |= GPIO_CRL_MODE5;
	GPIOA->CRL &= ~GPIO_CRL_CNF;
}

void usart2Init(void)
{
	// set A2 to AFIO, 50MHz
	GPIOA->CRL |= GPIO_CRL_MODE2;
	GPIOA->CRL |= GPIO_CRL_CNF2;
	
	// set A3 to input pullup/down
	GPIOA->CRL &= ~GPIO_CRL_MODE3;
	GPIOA->CRL |= GPIO_CRL_CNF3_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF3_0;
	
	//USART config
	USART2->CR1 |= USART_CR1_TE;
	USART2->CR1 |= USART_CR1_RE;
	USART2->CR1 |= USART_CR1_UE;
	USART2->BRR = 0x138;	// set baud rate to 115200
	USART2->CR1 &= ~USART_CR1_M & ~USART_CR1_PCE;	// 8 bit words, no parity
	
	USART2->CR1 |= USART_CR1_RXNEIE; // enable rx interrupts in usart
	NVIC_EnableIRQ(USART2_IRQn);	// enable interrupts in nvic
}

void usart2Close(void)
{
	// disable clocks for USART2 and port A 
	RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN;
	RCC->APB2ENR &= ~RCC_APB2ENR_IOPAEN;
	
	// reset A2
	GPIOA->CRL &= ~GPIO_CRL_MODE2;
	GPIOA->CRL &= ~GPIO_CRL_CNF2;
	
	// reset A3
	GPIOA->CRL &= ~GPIO_CRL_MODE3;
	GPIOA->CRL &= ~GPIO_CRL_CNF3;
	
	// reset USART
	USART2->CR1 &= ~USART_CR1_TE;
	USART2->CR1 &= ~USART_CR1_RE;
}

uint8_t getByte(void)
{
	while ((USART2->SR & USART_SR_RXNE) == 0);
	// wait for the receiver to not be empty
	return USART2->DR;
}

int sendByte(uint8_t b)
{
	if (USART2->CR1 & USART_CR1_TE)
	// if the USART transmitter is enabled, ie when serial is open
	{
		USART2->DR = b;
		while ((USART2->SR & USART_SR_TC) == 0)
		// wait for transmition to be complete
		return 0;
	}
	return -1;
}
void CLI_Transmit(uint8_t *pData, uint16_t Size)
{
		for (int i = 0; i < Size; i++)
		{
			char send = pData[i];
			sendByte(send);
			while ((USART2->SR & USART_SR_TXE) == 0);		
			// wait for transmission to be complete
		}
}


void setupScreen(void)
{
		
	// clear screen
	uint8_t message2[] = "\x1b[2J";
	CLI_Transmit(message2, sizeof(message2));
	
	// create scroll window
	uint8_t message3[] = "\x1b[5;r";
	CLI_Transmit(message3, sizeof(message3));
	
	// create count message
	uint8_t message[] = "Count:";
	CLI_Transmit(message, sizeof(message));
	
	// set cursor to 1,0
	uint8_t message4[] = "\x1b[2;0H";
	CLI_Transmit(message4, sizeof(message4));
	
	// set cursor to 5,0
	uint8_t message1[] = "\x1b[5;0H";
	CLI_Transmit(message1, sizeof(message1));

}


void welcomeScreen(void)
{
		char message[] = "\n\rWelcome to the STM32F103RB CLI!\n\r"
										"Type \"help\" for help\n\r";
		for (int i = 0; i < sizeof(message); i++)
		{
			char send = message[i];
			sendByte(send);
			while ((USART2->SR & USART_SR_TXE) == 0);		
			// wait for transmission to be complete
		}
}

