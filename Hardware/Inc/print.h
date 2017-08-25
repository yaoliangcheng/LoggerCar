#ifndef __PRINT_H
#define __PRINT_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "TFTLCDProcess.h"
#include "file.h"

/******************************************************************************/
#define PRINT_UART							(huart3)
#define PRINT_SEND_BYTES_MAX				(100)

#define PRINT_PWR_ENABLE() \
		HAL_GPIO_WritePin(PRINT_PWR_CTRL_GPIO_Port, PRINT_PWR_CTRL_Pin, GPIO_PIN_RESET);
#define PRINT_PWR_DISABLE() \
		HAL_GPIO_WritePin(PRINT_PWR_CTRL_GPIO_Port, PRINT_PWR_CTRL_Pin, GPIO_PIN_SET);

/******************************************************************************/
#define PRINT_ALARM_INVALID					(0xFF)

/******************************************************************************/
void PRINT_Date(char* fileName);
BOOL PRINT_DataOut(FILE_InfoTypedef* info, PRINT_ChannelSelectTypedef* select);
void PRINT_TitleOut(void);
void PRINT_TailOut(void);
void PRINT_SetMode(void);
















#endif
