//#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"
#include <stdio.h>
#include "functions.h" 

#define mainBLINKY_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainCOUNTER_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainPROCESS_TASK_PRIORITY (tskIDLE_PRIORITY + 3)

static void vBlinkTask( void * parameters);
static void updateCount( void * parameters);
static void processMessage( void * parameters);
void CLI_Process(uint8_t *pData);
void USART2_IRQHandler(void);

QueueHandle_t xQueueMessage;
QueueHandle_t xQueueBlinkF;
int main(void)
{
	xQueueMessage = xQueueCreate(6, sizeof(char));
	// create the blink frequency
	xQueueBlinkF = xQueueCreate(1, sizeof(uint16_t));
	
	// system init
	clockInit();
	usart2Init();
	setupScreen();
	welcomeScreen();
	
	// create tasks for the scheduler
	xTaskCreate(vBlinkTask, "Blinky", configMINIMAL_STACK_SIZE, 
		NULL, mainBLINKY_TASK_PRIORITY, NULL);
	xTaskCreate(updateCount, "Counter", configMINIMAL_STACK_SIZE, 
		NULL, mainCOUNTER_TASK_PRIORITY, NULL);
	xTaskCreate(processMessage, "Message Process",
		configMINIMAL_SECURE_STACK_SIZE, NULL, mainPROCESS_TASK_PRIORITY, NULL);
	
	xQueueSend(xQueueBlinkF, (void *) 1000, 1);
	
	// start scheduler
	vTaskStartScheduler();
	return 0;
}


static void vBlinkTask( void * parameters)
{
	for (;;)
	{
		uint16_t freq;
		xQueuePeek(xQueueBlinkF, (void *) &freq, 1);
		if (freq > 9999)
			freq = 9999;
		GPIOA->ODR |= (1u<<5);
		vTaskDelay(freq);
		GPIOA->ODR &= ~(1u<<5);
		vTaskDelay(freq);
	}
}

static void updateCount(void * parameters)
{
	long count = 0;
	for (;;)
	{
		// update counter info before moving
		char message[4];
		sprintf(message, "%ld", count);

		uint8_t dat[4];
		for (int i = 0; i < sizeof(message); i++)
		{
			dat[i] = message[i];	
		}
		
		// save position of cursor
		uint8_t oldPos[] = "\x1b[s";
		CLI_Transmit(oldPos, sizeof(oldPos));
		
		// move to 0,0
		uint8_t moveToStart[] = "\x1b[0;8H";
		CLI_Transmit(moveToStart, sizeof(moveToStart));
		
		// update counter value
		CLI_Transmit(dat, sizeof(dat));
		
		// move cursor to old position
		uint8_t oldPosR[] = "\x1b[u";
		CLI_Transmit(oldPosR, sizeof(oldPosR));
		count ++;
		vTaskDelay(1000);
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
										"Type \"fxxxx\" to change the frequency to xxxx (up to 9999)\n\r"
										"Type \"freq\" to query the frequency of the LED\n\r";
		CLI_Transmit(message, sizeof(message));
	}
	// state query
	else if (pData[0] == 'f' && pData[1] == 'r' && pData[2] == 'e' 
										&& pData[3] == 'q')
	{
		uint16_t freq;
		xQueuePeek(xQueueBlinkF, (void *) &freq, 1);
		char charStr[4];
		uint8_t uintStr[6];
		sprintf(charStr, "%d", freq);
		for (int i = 0; i < 4; i ++)
		{
			uintStr[i] = charStr[i];
		}
		uintStr[4] = '\r';
		uintStr[5] = '\n';
		uint8_t message[] = "\n\rThe frequency is ";;
		CLI_Transmit(message, sizeof(message));
		CLI_Transmit(uintStr, sizeof(uintStr));
	}
	// set frequency
	else if (pData[0] == 'f')
	{
		uint16_t newF = 0;
		if ((pData[4] < 58 && pData[4] > 47) && (pData[3] < 58 && pData[3] > 47) 
				&& (pData[2] < 58 && pData[2] > 47) && (pData[1] < 58 && pData[1] > 47))
		// the new F has 4 digits
		{
			newF = (pData[4] - 48) * 1 + (pData[3] - 48) * 10 
						+ (pData[2] - 48) * 100 + (pData[1] - 48) * 1000;
			xQueueOverwrite(xQueueBlinkF, (void *) &newF);
			uint8_t message[] = "\n\rNew frequency is \n\r";
			CLI_Transmit(message, sizeof(message));
		}
		else if ((pData[3] < 58 && pData[3] > 47) 
				&& (pData[2] < 58 && pData[2] > 47) && (pData[1] < 58 && pData[1] > 47))
		// the new F has 3 digits
		{
			newF = (pData[3] - 48) * 1 + (pData[2] - 48) * 10 + (pData[1] - 48) * 100;
			xQueueOverwrite(xQueueBlinkF, (void *) &newF);
			uint8_t message[] = "\n\rNew frequency is \n\r";
			CLI_Transmit(message, sizeof(message));
		}
		else if ((pData[2] < 58 && pData[2] > 47) && (pData[1] < 58 && pData[1] > 47))
		// the new F has 2 digits
		{
			newF = (pData[2] - 48) * 1 + (pData[1] - 48) * 10;
			xQueueOverwrite(xQueueBlinkF, (void *) &newF);
			uint8_t message[] = "\n\rNew frequency is \n\r";
			CLI_Transmit(message, sizeof(message));
		}
		else if (pData[1] < 58 && pData[1] > 47)	// the new F has 1 digits
		{
			newF = (pData[1] - 48) * 1;
			xQueueOverwrite(xQueueBlinkF, (void *) &newF);
			uint8_t message[] = "\n\rNew frequency is \n\r";
			CLI_Transmit(message, sizeof(message));
		}
		else 
		{
			uint8_t message[] = "\n\rPlease enter a 4 digit number after f\n\r";
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

