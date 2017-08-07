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
#define DEBUG_UART				(huart1)

/******************************************************************************/
typedef enum {FALSE = 0, TRUE = !FALSE} BOOL;

uint8_t HalfWord_GetHighByte(uint16_t value);
uint8_t HalfWord_GetLowByte(uint16_t value);
HAL_StatusTypeDef UART_DMAIdleConfig(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
void str2numb(uint8_t* buf, uint8_t* info, uint8_t size);
void BCD2ASCII(char* pASCII, uint8_t* pBCD, uint8_t size);
void HEX2BCD(uint8_t* pHEX, uint8_t* pBCD, uint8_t size);
void HEX2ASCII(uint8_t* pHEX, uint8_t* pASCII, uint8_t size);

#endif
