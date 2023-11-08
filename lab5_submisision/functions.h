#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdint.h>



void clockInit(void);
void usart2Init(void);
void usart2Close(void);
uint8_t getByte(void);
int sendByte(uint8_t b);

void CLI_Receive(uint8_t *pData, uint16_t Size);

void CLI_Process(uint8_t *pData);
void setupScreen(void);

void updateStatus(void);
void welcomeScreen(void);
