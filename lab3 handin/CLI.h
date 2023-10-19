#include <stm32f10x.h>
#include <stdint.h>

void CLI_Transmit(uint8_t *pData, uint16_t Size);
void CLI_Receive(uint8_t *pData, uint16_t Size);
void CLI_Process(uint8_t *pData);
