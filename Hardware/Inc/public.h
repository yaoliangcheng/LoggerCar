#ifndef __COMMON_H
#define __COMMON_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include <string.h>
#include <stdio.h>
//#include <malloc.h>
#include <math.h>
#include <stdlib.h>

#include "usart.h"

/******************************************************************************/
#define DEBUG_UART				(huart4)

#define GET_COMPARE_TIME(year, month, day, hour, min) \
	(((uint32_t)((year << 24) | (month << 16) | (day << 8) | hour) << 8) | min)

#define CHAR2LONG(value24, value16, value8, value) \
	(uint32_t)((value24 << 24) | (value16 << 16) | (value8 << 8) | value)

#define HALFWORD_BYTE_H(value)      ((uint8_t)((value & 0xFF00) >> 8))
#define HALFWORD_BYTE_L(value)      ((uint8_t)(value & 0x00FF))

/******************************************************************************/
typedef enum {FALSE = 0, TRUE = !FALSE} BOOL;

/******************************************************************************/
HAL_StatusTypeDef UART_DMAIdleConfig(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);

void BCD2ASCII(char* pASCII, uint8_t* pBCD, uint8_t size);
void HEX2BCD(uint8_t* pHEX, uint8_t* pBCD, uint8_t size);
void HEX2ASCII(uint8_t* pHEX, uint8_t* pASCII, uint8_t size);
void ASCII2HEX(uint8_t* pASCII, uint8_t* pHEX, uint8_t size);
void DebugPrintf(char* str);

void HEX2ASCII(uint8_t* pASCII, uint8_t* pHEX, uint8_t size);
void ASCII2BCD(uint8_t* pBCD, char* pASCII, uint8_t size);
void str2numb(uint8_t* pNumb, uint8_t* pStr, uint8_t size);

#endif
