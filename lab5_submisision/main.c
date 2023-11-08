//#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"
//#include <stdint.h>
#include "functions.h" 

#define mainBLINKY_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainCOUNTER_TASK_PRIORITY (tskIDLE_PRIORITY + 2)

static void vBlinkTask( void * parameters);
static void updateCount( void * parameters);
void USART2_IRQHandler(void);

QueueHandle_t xQueueCounter;
QueueHandle_t xQueueMessage;
int main(void)
{
	// create counter queue
	xQueueCounter = xQueueCreate(1, sizeof(int));
	// send a 0 to the queue. This is the position monitor
	xQueueSend(xQueueCounter, (void *) 0, (TickType_t) 0);
	// create message queue
	xQueueMessage = xQueueCreate(5, sizeof(char));
	
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
	
	// start scheduler
	vTaskStartScheduler();
	
	return 0;
}


static void vBlinkTask( void * parameters)
{
	for (;;)
	{
		GPIOA->ODR |= (1u<<5);
		vTaskDelay(1000);
		GPIOA->ODR &= ~(1u<<5);
		vTaskDelay(1000);
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

void USART2_IRQHandler()
{
	BaseType_t xTaskWokenByReceive1 = pdFALSE;
	BaseType_t xTaskWokenByReceive2 = pdFALSE;
	BaseType_t xHigherPriorityTaskWoken1 = pdFALSE;
	BaseType_t xHigherPriorityTaskWoken2 = pdFALSE;
	int pos;
	// receive the current position from the queue
	while (xQueueReceiveFromISR(xQueueCounter, (void *) &pos, &xTaskWokenByReceive1))
	{
		// get the byte
		uint8_t byte = getByte();

		// send the byte to the CLI
		sendByte(byte);
		if (byte == 8)		// compensate for backspace
		{
			pos = pos - 2;
			xQueueReceiveFromISR(xQueueMessage, (void *) &byte, &xTaskWokenByReceive2);
			if (xTaskWokenByReceive2 != pdFALSE)
				taskYIELD();
		}
		else if (byte == '\r')	// if enter is pressed, stop taking input
		{
			pos = 10;
		}
		else 
		{
			// send the byte to the message queue if its not a backspace or a \r
			xQueueSendToFrontFromISR(xQueueMessage, (void *) &byte, &xTaskWokenByReceive1);
			if(xHigherPriorityTaskWoken1)
			taskYIELD();
		}
		pos ++;
		// send the new current position to the position queue
		xQueueSendFromISR(xQueueCounter, (void *) &pos, &xHigherPriorityTaskWoken2);
		if(xHigherPriorityTaskWoken2)
			taskYIELD();
	}
	if (xTaskWokenByReceive1 != pdFALSE)
		taskYIELD();
}