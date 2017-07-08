#include "common.h"

/*******************************************************************************
 *
 */
uint8_t HalfWord_GetHighByte(uint16_t value)
{
	return (uint8_t)((value & 0xFF00) >> 8);
}

uint8_t HalfWord_GetLowByte(uint16_t value)
{
	return (uint8_t)(value & 0x00FF);
}

/******************************************************************************/
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&DEBUG_UART, (uint8_t *)&ch, 1, 0xffff);
	return ch;
}

int fgetc(FILE * f)
{
	uint8_t ch = 0;
	HAL_UART_Receive(&DEBUG_UART, &ch, 1, 0xffff);
	return ch;
}


