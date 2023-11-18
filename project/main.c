//#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"
#include <stdio.h>
#include "functions.h" 

#define mainBLINKY_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainPROCESS_TASK_PRIORITY (tskIDLE_PRIORITY + 3)

static void vInputTest( void * parameters);
static void processMessage( void * parameters);
void CLI_Process(uint8_t *pData);
void USART2_IRQHandler(void);

QueueHandle_t xQueueMessage;
int main(void)
{
	xQueueMessage = xQueueCreate(6, sizeof(char));
	// create the blink frequency
	
	// system init
	clockInit();
	usart2Init();
	setupScreen();
	welcomeScreen();
	
	// create tasks for the scheduler
	xTaskCreate(vInputTest, "In Test", configMINIMAL_STACK_SIZE, 
		NULL, mainBLINKY_TASK_PRIORITY, NULL);
	xTaskCreate(processMessage, "Message Process",
		configMINIMAL_SECURE_STACK_SIZE, NULL, mainPROCESS_TASK_PRIORITY, NULL);
	
	// start scheduler
	vTaskStartScheduler();
	return 0;
}


static void vInputTest( void * parameters)
{
	for (;;)
	{
		if ((GPIOA->IDR & GPIO_IDR_IDR8) != 0)
		{
			uint8_t test[] = "\r\n UP \r\n";
			CLI_Transmit(test, sizeof(test));
		}
		else if ((GPIOA->IDR & GPIO_IDR_IDR9) != 0)
		{
			uint8_t test[] = "\r\n DOWN \r\n";
			CLI_Transmit(test, sizeof(test));
		}
		else if ((GPIOA->IDR & GPIO_IDR_IDR10) != 0)
		{
			uint8_t test[] = "\r\n CLOSE \r\n";
			CLI_Transmit(test, sizeof(test));
		}
		else if ((GPIOB->IDR & GPIO_IDR_IDR6) != 0)
		{
			uint8_t test[] = "\r\n OPEN \r\n";
			CLI_Transmit(test, sizeof(test));
		}
		else if ((GPIOB->IDR & GPIO_IDR_IDR8) != 0)
		{
			uint8_t test[] = "\r\n EMERGENCY \r\n";
			CLI_Transmit(test, sizeof(test));
		}
		else if ((GPIOB->IDR & GPIO_IDR_IDR9) != 0)
		{
			uint8_t test[] = "\r\n MAINTENANCE \r\n";
			CLI_Transmit(test, sizeof(test));
		}
		else if ((GPIOB->IDR & GPIO_IDR_IDR10) != 0)
		{
			uint8_t test[] = "\r\n CALL \r\n";
			CLI_Transmit(test, sizeof(test));
		}
		vTaskDelay(50);
	}
}

static void processMessage( void * parameters )
{
	for (;;)
	{
		if (uxQueueMessagesWaiting(xQueueMessage) == 6)
		{
			uint8_t temp[6];
			// move the contents of the message queue into the temp array
			for (int i = 0; uxQueueMessagesWaiting(xQueueMessage) > 0; i++)
			{
				xQueueReceive(xQueueMessage, &temp[5-i], 10);
			}
			CLI_Process(temp);
		}
		vTaskDelay(10);
	}
}
void USART2_IRQHandler()
{
	BaseType_t xTaskWokenByReceive1 = pdFALSE;
	BaseType_t xHigherPriorityTaskWoken1 = pdFALSE;
	int freeSpace = 48;
	// get the byte
	uint8_t byte = getByte();
	sendByte(byte);
	//send the byte to the message queue if its not a backspace or delete char
	if (byte == 8 || byte == 127)
	{
		// backspace
		xQueueReceiveFromISR(xQueueMessage, (void *) &byte, &xHigherPriorityTaskWoken1);
		if(xHigherPriorityTaskWoken1)
		taskYIELD();
	}
	else if (byte == '\r')
	{
		// enter key, fill the queue
		while (uxQueueMessagesWaiting(xQueueMessage) <= 5)
		{
			xQueueSendToFrontFromISR(xQueueMessage, (void *) '\r', &xTaskWokenByReceive1);
		}
	}
	else 
	{
		xQueueSendToFrontFromISR(xQueueMessage, (void *) &byte, &xTaskWokenByReceive1);
	}
}
		
	
void CLI_Process(uint8_t *pData)
{
	// help state
	if (pData[0] == 'h' && pData[1] == 'e' && pData[2] == 'l' 
							&& pData[3] == 'p')
	{
		uint8_t message[] = "\n\rHelp Screen\n\r"
										"Elevator Control System\n\r"
										"Theres no functions yet. Sorry!\n\r";
		CLI_Transmit(message, sizeof(message));
	}
	// state query
	else if (pData[0] == ' ' && pData[1] == ' ' && pData[2] == ' ' && pData[3] == ' ')
	{
		
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

