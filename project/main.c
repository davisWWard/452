//#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"
#include <stdio.h>
#include "functions.h" 

#define mainINPUT_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainTRANSITION_TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define mainPROCESS_TASK_PRIORITY (tskIDLE_PRIORITY + 5)
#define mainUPDATE_TASK_PRIORITY (tskIDLE_PRIORITY + 2)

static void vInput( void * parameters);
static void processMessage( void * parameters);
static void moveFloors( void * parameters);
static void updateScreen( void * parameters);

void openDoors(void);
void closeDoors(void);
void CLI_Process(uint8_t *pData);


void USART2_IRQHandler(void);
void EXTI4_IRQHandler(void);

QueueHandle_t xQueueMessage;
QueueHandle_t xCurrentFloor;
QueueHandle_t xDestinationFloor;
QueueHandle_t xCurrentState;
QueueHandle_t xDoorState;

int main(void)
{
	// QueueMessage holds the input from the CLI
	xQueueMessage = xQueueCreate(6, sizeof(char));
	// CurrentFloor holds current floor, from 1 to 4
	xCurrentFloor = xQueueCreate(1, sizeof(uint8_t));
	// DestinationFloor holds the queued floors, up to 3
	xDestinationFloor = xQueueCreate(3, sizeof(uint8_t));
	// CurrentState holds the current state, 
	// r for regular, m for maintenance, e for emergency
	xCurrentState = xQueueCreate(1, sizeof(char));
	// DoorState holds the state of the doors
	// 1 for open, 0 for closed
	xDoorState = xQueueCreate(1, sizeof(uint8_t));
	
	// start at floor 1, put 1 into current floor queue
	uint8_t startFloor = 1;
	xQueueSend(xCurrentFloor, (void *) &startFloor, 100);
	
	// system init
	clockInit();
	usart2Init();
	setupScreen();
	welcomeScreen();
	
	
	// create tasks for the scheduler
	// Task to receive input from the pushbuttons
	xTaskCreate(vInput, "Input", configMINIMAL_STACK_SIZE, 
		NULL, mainINPUT_TASK_PRIORITY, NULL);
	// task to process the CLI message
	xTaskCreate(processMessage, "Message Process",
		configMINIMAL_SECURE_STACK_SIZE, NULL, mainPROCESS_TASK_PRIORITY, NULL);
	// task to transition floors
	xTaskCreate(moveFloors, "Transition Floors", configMINIMAL_STACK_SIZE, 
		NULL, mainTRANSITION_TASK_PRIORITY, NULL);
	// task to update the CLI status window
	xTaskCreate(updateScreen, "Update Screen", configMINIMAL_STACK_SIZE, 
		NULL, mainUPDATE_TASK_PRIORITY, NULL);
	
	// start scheduler
	vTaskStartScheduler();
	return 0;
}

static void vInput( void * parameters)
{
	for (;;)
	{
		// Up button
		if ((GPIOA->IDR & GPIO_IDR_IDR0) != 0)
		{
			uint8_t curr;
			xQueuePeek(xCurrentFloor, &curr, 1);
			if (curr == 1)
			{
				uint8_t dest = 2;
				xQueueSend(xDestinationFloor, (void *) &dest, 1);
			}
			else if (curr == 2)
			{
				uint8_t dest = 3;
				xQueueSend(xDestinationFloor, (void *) &dest, 1);
			}
			else if (curr == 3)
			{
				uint8_t dest = 4;
				xQueueSend(xDestinationFloor, (void *) &dest, 1);
			}
			uint8_t message[] = "\r\n UP \r\n";
			CLI_Transmit(message, sizeof(message));
		}
		// Down button
		else if ((GPIOA->IDR & GPIO_IDR_IDR1) != 0)
		{
			uint8_t curr;
			xQueuePeek(xCurrentFloor, &curr, 1);
			if (curr == 2)
			{
				uint8_t dest = 1;
				xQueueSend(xDestinationFloor, (void *) &dest, 1);
			}
			else if (curr == 3)
			{
				uint8_t dest = 2;
				xQueueSend(xDestinationFloor, (void *) &dest, 1);
			}
			else if (curr == 4)
			{
				uint8_t dest = 3;
				xQueueSend(xDestinationFloor, (void *) &dest, 1);
			}
			uint8_t message[] = "\r\n DOWN \r\n";
			CLI_Transmit(message, sizeof(message));
		}
		// Close button
		else if ((GPIOA->IDR & GPIO_IDR_IDR6) != 0)
		{
			closeDoors();
			uint8_t message[] = "\r\n CLOSE \r\n";
			CLI_Transmit(message, sizeof(message));
		}
		// Open button
		else if ((GPIOA->IDR & GPIO_IDR_IDR7) != 0)
		{
			openDoors();
			uint8_t message[] = "\r\n OPEN \r\n";
			CLI_Transmit(message, sizeof(message));
		}
		// Maintenance button
		else if ((GPIOB->IDR & GPIO_IDR_IDR0) != 0)
		{
			uint8_t state;
			xQueuePeek(xCurrentState, (void *) &state, 10);
			if ((state != 'e')	&& (state == 'm'))
			// exit maintenance mode if in maintenance mode and not in e mode
			{
				uint8_t state = 'n';
				xQueueOverwrite(xCurrentState, (void *) &state);
			}
			else if (state != 'e')
			// go into emergency mode
			{
				uint8_t state = 'm';
				xQueueOverwrite(xCurrentState, (void *) &state);
			}
				uint8_t message[] = "\r\n MAINTENANCE \r\n";
				CLI_Transmit(message, sizeof(message));
		}
		// Call button
		else if ((GPIOB->IDR & GPIO_IDR_IDR5) != 0)
		{
			uint8_t dest = 1;
			xQueueSend(xDestinationFloor, (void *) &dest, 10);
			uint8_t message[] = "\r\n CALL \r\n";
			CLI_Transmit(message, sizeof(message));
		}
		// 100ms button debounce
		vTaskDelay(100);
	}
}

static void processMessage( void * parameters )
{
	for (;;)
	{
		if ( uxQueueMessagesWaiting(xQueueMessage) == 6)
		{
			uint8_t temp[6];
			// move the contents of the message queue into the temp array
			for (int i = 0; uxQueueMessagesWaiting(xQueueMessage) > 0; i++)
			{
				xQueueReceive(xQueueMessage, &temp[5-i], 1);
			}
			CLI_Process(temp);
		}
		vTaskDelay(10);
	}
}

static void moveFloors( void * parameters)
{
	for (;;)
	{
		uint8_t state;
		xQueuePeek(xCurrentState, (void *) &state, 10);
		if (state != 'e' && state != 'm' && ((uxQueueMessagesWaiting(xDestinationFloor) >= 1) 
			&& (uxQueueMessagesWaiting(xDestinationFloor) <= 3)))
		// There is a request to move floors
		{
			
			// receive the destination and put into dest variable
			uint8_t dest;
			xQueueReceive(xDestinationFloor, (void *) &dest, 1);
			
			// get current floor to compare to to destination floor
			uint8_t curr;
			xQueueReceive(xCurrentFloor, (void *) &curr, 1);
			// move up if destination is higher than curr
			if (curr < dest)
			{
				closeDoors();
				vTaskDelay(1000); // delay to simulate closing doors before moving
				curr += 1;
				xQueueSend(xCurrentFloor, (void *) &curr, 1);
				xQueueSendToFront(xDestinationFloor, (void *) &dest, 1);
			}
			// move down if destination is lower than curr
			else if (curr > dest)
			{
				closeDoors();
				vTaskDelay(1000); // delay to simulate closing doors before moving
				curr -= 1;
				xQueueSend(xCurrentFloor, (void *) &curr, 1);
				xQueueSendToFront(xDestinationFloor, (void *) &dest, 1);
			}
			// we are at the destination floor
			else
			{
				xQueueSend(xCurrentFloor, (void *) &curr, 1);
				openDoors();
				vTaskDelay(1000); // delay to simulate opening doors before moving
			}
		}
		// long delay since this doesnt need to run much
		vTaskDelay(1000);
	}
}
	
static void updateScreen( void * parameters)
{
	for(;;)
	{
		NVIC_DisableIRQ(USART2_IRQn);	// disable usart while updating
		// update current floor message
		uint8_t savePos[] = "\x1b[s";
		CLI_Transmit(savePos, sizeof(savePos));
		
		uint8_t curr;
		xQueuePeek(xCurrentFloor, &curr, 0);
		if (curr == 1)
		{
			uint8_t move[] = "\x1b[2;0H";
			CLI_Transmit(move, sizeof(move));
			uint8_t currFloor[] = "^                  \r\n";
			CLI_Transmit(currFloor, sizeof(currFloor));
		}
		else if (curr == 2)
		{
			uint8_t move[] = "\x1b[2;0H";
			CLI_Transmit(move, sizeof(move));
			uint8_t currFloor[] = "      ^            \r\n";
			CLI_Transmit(currFloor, sizeof(currFloor));
		}
		else if (curr == 3)
		{
			uint8_t move[] = "\x1b[2;0H";
			CLI_Transmit(move, sizeof(move));
			uint8_t currFloor[] = "            ^      \r\n";
			CLI_Transmit(currFloor, sizeof(currFloor));
		}
		else if (curr == 4)
		{
			uint8_t move[] = "\x1b[2;0H";
			CLI_Transmit(move, sizeof(move));
			uint8_t currFloor[] = "                  ^\r\n";
			CLI_Transmit(currFloor, sizeof(currFloor));
		}
		
		// update Destination message
		uint8_t dest;
		xQueuePeek(xDestinationFloor, &dest, 0);
		if (dest == 1)
		{
			uint8_t moveToThird[] = "\x1b[3;14H";
			CLI_Transmit(moveToThird, sizeof(moveToThird));
			uint8_t destFloor[] = "1";
			CLI_Transmit(destFloor, sizeof(destFloor));
		}
		else if (dest == 2)
		{
			uint8_t moveToThird[] = "\x1b[3;14H";
			CLI_Transmit(moveToThird, sizeof(moveToThird));
			uint8_t destFloor[] = "2";
			CLI_Transmit(destFloor, sizeof(destFloor));
		}
		else if (dest == 3)
		{
			uint8_t moveToThird[] = "\x1b[3;14H";
			CLI_Transmit(moveToThird, sizeof(moveToThird));
			uint8_t destFloor[] = "3";
			CLI_Transmit(destFloor, sizeof(destFloor));
		}
		else if (dest == 4)
		{
			uint8_t moveToThird[] = "\x1b[3;14H";
			CLI_Transmit(moveToThird, sizeof(moveToThird));
			uint8_t destFloor[] = "4";
			CLI_Transmit(destFloor, sizeof(destFloor));
		}
		
		// update doors message
		uint8_t doors;
		xQueuePeek(xDoorState, &doors, 0);
		if (doors == 1)
		{
			uint8_t moveToFourth[] = "\x1b[4;8H";
			CLI_Transmit(moveToFourth, sizeof(moveToFourth));
			uint8_t doors[] = "Open";
			CLI_Transmit(doors, sizeof(doors));
		}
		else if (doors == 0)
		{
			uint8_t moveToFourth[] = "\x1b[4;8H";
			CLI_Transmit(moveToFourth, sizeof(moveToFourth));
			uint8_t doors[] = "Closed";
			CLI_Transmit(doors, sizeof(doors));
		}
		
		// update state message
		uint8_t state;
		xQueuePeek(xCurrentState, &state, 0);
		if (state == 'n')
		{
			uint8_t moveToFifth[] = "\x1b[5;8H";
			CLI_Transmit(moveToFifth, sizeof(moveToFifth));
			uint8_t state[] = "Normal     ";
			CLI_Transmit(state, sizeof(state));
		}
		else if (state == 'e')
		{
			uint8_t moveToFifth[] = "\x1b[5;8H";
			CLI_Transmit(moveToFifth, sizeof(moveToFifth));
			uint8_t state[] = "Emergency  ";
			CLI_Transmit(state, sizeof(state));
		}
		else if (state == 'm')
		{
			uint8_t moveToFifth[] = "\x1b[5;8H";
			CLI_Transmit(moveToFifth, sizeof(moveToFifth));
			uint8_t state[] = "Maintenance";
			CLI_Transmit(state, sizeof(state));
		}

		// return cursor to old position
		uint8_t restorePos[] = "\x1b[u";
		CLI_Transmit(restorePos, sizeof(restorePos));
		NVIC_EnableIRQ(USART2_IRQn);	// reenable usart
		vTaskDelay(100);
	}
}

void openDoors()
{
	uint8_t open = 1;
	xQueueOverwrite(xDoorState, &open);
}

void closeDoors()
{
	uint8_t close = 0;
	xQueueOverwrite(xDoorState, &close);
}

void CLI_Process(uint8_t *pData)
{
	// help state
	if (pData[0] == 'h' && pData[1] == 'e' && pData[2] == 'l' 
							&& pData[3] == 'p')
	{
		uint8_t message[] = "\n\r Help Screen\n\r"
										" Elevator Control System\n\r"
										" \"floor\" to query the current floor\n\r"
										" \"up\" to move up a floor\n\r"
										" \"down\" to move down a floor\n\r"
										" \"close\" to close the doors\n\r"
										" \"open\" to open the doors\n\r"
										" \"maint\" to enter maintenance mode\n\r"
										" \"emerg\" to enter emergency mode\n\r"
										" \"call\" to return to the first floor\n\r"
										" \"gotox\" where x is from 1 - 4 to go to the xth floor\n\r";
		CLI_Transmit(message, sizeof(message));
	}
	// floor query
	if (pData[0] == 'f' && pData[1] == 'l' && pData[2] == 'o' 
							&& pData[3] == 'o' && pData[4] == 'r')
	{
		uint8_t floor[] = "\n\r    \n\r";
		xQueuePeek(xCurrentFloor, (void *) &floor[5], 0);
		floor[5] += 48;
		CLI_Transmit(floor, sizeof(floor));
	}
	// up
	else if (pData[0] == 'u' && pData[1] == 'p')
	{
		uint8_t curr;
		xQueuePeek(xCurrentFloor, &curr, 1);
		if (curr == 1)
		{
			uint8_t dest = 2;
			xQueueSend(xDestinationFloor, (void *) &dest, 1);
		}
		else if (curr == 2)
		{
			uint8_t dest = 3;
			xQueueSend(xDestinationFloor, (void *) &dest, 1);
		}
		else if (curr == 3)
		{
			uint8_t dest = 4;
			xQueueSend(xDestinationFloor, (void *) &dest, 1);
		}
		uint8_t message[] = "\r\n UP \r\n";
		CLI_Transmit(message, sizeof(message));
	}
	// down
	else if (pData[0] == 'd' && pData[1] == 'o' 
		&& pData[2] == 'w' && pData[3] == 'n')
	{
		uint8_t curr;
		xQueuePeek(xCurrentFloor, &curr, 1);
		if (curr == 2)
		{
			uint8_t dest = 1;
			xQueueSend(xDestinationFloor, (void *) &dest, 1);
		}
		else if (curr == 3)
		{
			uint8_t dest = 2;
			xQueueSend(xDestinationFloor, (void *) &dest, 1);
		}
		else if (curr == 4)
		{
			uint8_t dest = 3;
			xQueueSend(xDestinationFloor, (void *) &dest, 1);
		}
		uint8_t message[] = "\r\n DOWN \r\n";
		CLI_Transmit(message, sizeof(message));
	}
	// close
	else if (pData[0] == 'c' && pData[1] == 'l' 
		&& pData[2] == 'o' && pData[3] == 's' && pData[4] == 'e')
	{
		closeDoors();
		uint8_t message[] = "\r\n CLOSE \r\n";
		CLI_Transmit(message, sizeof(message));
	}
	// open
	else if (pData[0] == 'o' && pData[1] == 'p' 
		&& pData[2] == 'e' && pData[3] == 'n')
	{
		openDoors();
		uint8_t message[] = "\r\n OPEN \r\n";
		CLI_Transmit(message, sizeof(message));
	}
	// maintenance
	else if (pData[0] == 'm' && pData[1] == 'a' 
		&& pData[2] == 'i' && pData[3] == 'n' && pData[4] == 't')
	{
		uint8_t state;
		xQueuePeek(xCurrentState, (void *) &state, 1);
		if ((state != 'e')	&& (state == 'm'))
		// exit maintenance mode if in maintenance mode and not in e mode
		{
			uint8_t state = 'n';
			xQueueOverwrite(xCurrentState, (void *) &state);
		}
		else if (state != 'e')
		// go into emergency mode
		{
			uint8_t state = 'm';
			xQueueOverwrite(xCurrentState, (void *) &state);
		}
			uint8_t message[] = "\r\n MAINTENANCE \r\n";
			CLI_Transmit(message, sizeof(message));
	}
	// emergency
	else if (pData[0] == 'e' && pData[1] == 'm' 
		&& pData[2] == 'e' && pData[3] == 'r' && pData[4] == 'g')
	{
		if ((GPIOA->ODR & GPIO_ODR_ODR5) != 0)
		// exit emergency mode
		{
			GPIOA->ODR &= ~GPIO_ODR_ODR5;
			uint8_t state = 'n';
			xQueueOverwrite(xCurrentState, (void *) &state);
		}
		else 
		// go into emergency mode
		{
			GPIOA->ODR |= GPIO_ODR_ODR5;
			uint8_t state = 'e';
			xQueueOverwrite(xCurrentState, (void *) &state);
		}
		uint8_t message[] = "\r\n EMERGENCY \r\n";
		CLI_Transmit(message, sizeof(message));
	}
	// call
	else if (pData[0] == 'c' && pData[1] == 'a' 
		&& pData[2] == 'l' && pData[3] == 'l')
	{
		uint8_t dest = 1;
		xQueueSend(xDestinationFloor, (void *) &dest, 1);
		uint8_t message[] = "\r\n CALL \r\n";
		CLI_Transmit(message, sizeof(message));
	}
	// go to floor 1
	else if (pData[0] == 'g' && pData[1] == 'o' 
		&& pData[2] == 't' && pData[3] == 'o' && pData[4] == '1')
	{
		uint8_t dest = 1;
		xQueueSend(xDestinationFloor, (void *) &dest, 1);
		uint8_t message[] = "\r\n Go To Floor 1 \r\n";
		CLI_Transmit(message, sizeof(message));
	}
	// go to floor 2
	else if (pData[0] == 'g' && pData[1] == 'o' 
		&& pData[2] == 't' && pData[3] == 'o' && pData[4] == '2')
	{
		uint8_t dest = 2;
		xQueueSend(xDestinationFloor, (void *) &dest, 1);
		uint8_t message[] = "\r\n Go To Floor 2 \r\n";
		CLI_Transmit(message, sizeof(message));
	}
	// go to floor 3
	else if (pData[0] == 'g' && pData[1] == 'o' 
		&& pData[2] == 't' && pData[3] == 'o' && pData[4] == '3')
	{
		uint8_t dest = 3;
		xQueueSend(xDestinationFloor, (void *) &dest, 1);
		uint8_t message[] = "\r\n Go To Floor 3 \r\n";
		CLI_Transmit(message, sizeof(message));
	}
	// go to floor 4
	else if (pData[0] == 'g' && pData[1] == 'o' 
		&& pData[2] == 't' && pData[3] == 'o' && pData[4] == '4')
	{
		uint8_t dest = 4;
		xQueueSend(xDestinationFloor, (void *) &dest, 1);
		uint8_t message[] = "\r\n Go To Floor 4 \r\n";
		CLI_Transmit(message, sizeof(message));
	}
	// empty line
	else if (pData[0] == '\r')
	{
		uint8_t message[] = "\n\rWelcome to the Elevator!\n\r"
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

void USART2_IRQHandler()
{
	BaseType_t xTaskWokenByReceive1 = pdFALSE;
	BaseType_t xHigherPriorityTaskWoken1 = pdFALSE;
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
		// enter key, fill the queue to trigger queue proccessing
		while (uxQueueMessagesWaiting(xQueueMessage) <= 5)
		{
			xQueueSendToFrontFromISR(xQueueMessage, (void *) &byte, &xTaskWokenByReceive1);
		}
	}
	else 
	{
		xQueueSendToFrontFromISR(xQueueMessage, (void *) &byte, &xTaskWokenByReceive1);
	}
}

void EXTI4_IRQHandler()
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint8_t message[] = "\n\r EMERGENCY \n\r";
	CLI_Transmit(message, sizeof(message));
	if ((GPIOA->ODR & GPIO_ODR_ODR5) != 0)
	// exit emergency mode
	{
		GPIOA->ODR &= ~GPIO_ODR_ODR5;
		uint8_t state = 'n';
		xQueueOverwriteFromISR(xCurrentState, (void *) &state, &xHigherPriorityTaskWoken);
		if(xHigherPriorityTaskWoken)
			taskYIELD();
	}
	else 
	// go into emergency mode
	{
		GPIOA->ODR |= GPIO_ODR_ODR5;
		uint8_t state = 'e';
		xQueueOverwriteFromISR(xCurrentState, (void *) &state, &xHigherPriorityTaskWoken);
		if(xHigherPriorityTaskWoken)
			taskYIELD();
	}
	EXTI->PR |= EXTI_PR_PR4;
}
