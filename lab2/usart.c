#include "usart.h"

void serial_open(void)
{
	// set A2 to AFIO, 50MHz
	GPIOA->CRL |= GPIO_CRL_MODE2;
	GPIOA->CRL |= GPIO_CRL_CNF2;
	
	// set A3 to input pullup/down
	GPIOA->CRL &= ~GPIO_CRL_MODE3;
	GPIOA->CRL |= GPIO_CRL_CNF3_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF3_0;
	
	// USART config
	USART2->CR1 |= USART_CR1_TE;
	USART2->CR1 |= USART_CR1_RE;
	USART2->CR1 |= USART_CR1_UE;
	USART2->BRR = 0x138;	// set baud rate to 115200
	USART2->CR1 &= ~USART_CR1_M & ~USART_CR1_PCE;	// 8 bit words, no parity
}

void serial_close(void)
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

int sendByte(uint8_t b)
{
	if (USART2->CR1 & USART_CR1_TE)
	// if the USART transmitter is enabled, ie when serial is open
	{
		USART2->DR = b;
		while ((USART2->SR & USART_SR_TC) == 0);
		// wait for transmition to be complete
		return 0;
	}
	
	return -1;
}

uint8_t getByte(void)
{
	while ((USART2->SR & USART_SR_RXNE) == 0);
	// wait for the receiver to not be empty
	return USART2->DR;
}

