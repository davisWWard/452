#include "functions.h"


void clockInit(void)
{
	// enable clocks for USART2, port A and B
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	
	// get A5 to output, push pull
	GPIOA->CRL |= GPIO_CRL_MODE5;
	GPIOA->CRL &= ~GPIO_CRL_CNF5;
	
	// Set A8 A9 A10 to input
	GPIOA->CRH &= ~GPIO_CRH_MODE8 & ~GPIO_CRH_MODE9 & ~GPIO_CRH_MODE10;
	GPIOA->CRH |= GPIO_CRH_CNF8_1 | GPIO_CRH_CNF9_1 | GPIO_CRH_CNF10_1;
	GPIOA->CRH &= ~GPIO_CRH_CNF8_0 & ~GPIO_CRH_CNF9_0 & ~GPIO_CRH_CNF10_0;
	
	// B8, B9, B10 as input
	GPIOB->CRH &= ~GPIO_CRH_MODE8 & ~GPIO_CRH_MODE9 & ~GPIO_CRH_MODE10;
	GPIOB->CRH |= GPIO_CRH_CNF8_1 | GPIO_CRH_CNF9_1 | GPIO_CRH_CNF10_1;
	GPIOB->CRH &= ~GPIO_CRH_CNF8_0 & ~GPIO_CRH_CNF9_0 & ~GPIO_CRH_CNF10_0;
	
	// B6 as input
	GPIOB->CRL &= ~GPIO_CRL_MODE6;
	GPIOB->CRL |= GPIO_CRL_CNF6_1;
	GPIOB->CRL &= ~GPIO_CRL_CNF6_0;

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
	uint8_t clear[] = "\x1b[2J";
	CLI_Transmit(clear, sizeof(clear));
	
	// create scroll window of size 7
	uint8_t scrollWindow[] = "\x1b[7;r";
	CLI_Transmit(scrollWindow, sizeof(scrollWindow));
	
	// create current floor message
	uint8_t currFloor[] = "1     2     3     4\r\n"
											"^                      ";
	CLI_Transmit(currFloor, sizeof(currFloor));
	
	// set cursor to third row
	uint8_t moveToThird[] = "\x1b[3;0H";
	CLI_Transmit(moveToThird, sizeof(moveToThird));
	
	// create destination floor message
	uint8_t destFloor[] = "Destination:";
	CLI_Transmit(destFloor, sizeof(destFloor));
	
	// set cursor to fourth row
	uint8_t moveToFourth[] = "\x1b[4;0H";
	CLI_Transmit(moveToFourth, sizeof(moveToFourth));
	
	// create direction message
	uint8_t direction[] = "Direction:";
	CLI_Transmit(direction, sizeof(direction));
	
	// set cursor to fifth row
	uint8_t moveToFifth[] = "\x1b[5;0H";
	CLI_Transmit(moveToFifth, sizeof(moveToFifth));
	
	// create state message
	uint8_t state[] = "State:";
	CLI_Transmit(state, sizeof(state));
	
	// set cursor to 8,0
	uint8_t moveCommandLine[] = "\x1b[8;0H";
	CLI_Transmit(moveCommandLine, sizeof(moveCommandLine));

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

