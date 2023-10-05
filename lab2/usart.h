#include <stm32f10x.h>
#include <stdint.h>

void serial_open(void);
void serial_close(void);
int sendByte(uint8_t);
uint8_t getByte(void);
