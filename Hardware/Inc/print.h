#ifndef __PRINT_H
#define __PRINT_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "TFTLCDProcess.h"
#include "display.h"
#include "file.h"
#include "fatfs.h"
#include "public.h"

/******************************************************************************/
#define PRINT_UART							(huart5)
#define PRINT_SEND_BYTES_MAX				(100)

#define PRINT_PWR_ENABLE() \
		HAL_GPIO_WritePin(PRINT_PWR_CTRL_GPIO_Port, PRINT_PWR_CTRL_Pin, GPIO_PIN_RESET);
#define PRINT_PWR_DISABLE() \
		HAL_GPIO_WritePin(PRINT_PWR_CTRL_GPIO_Port, PRINT_PWR_CTRL_Pin, GPIO_PIN_SET);

/******************************************************************************/
#define PRINT_ALARM_INVALID					(0xFF)

/******************************************************************************/
typedef enum
{
	PRINT_DATA_NORMAL,
	PRINT_DATA_OVERlIMITED,
	PRINT_DATA_END,
} PRINT_DataStatusEnum;

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)



//typedef struct
//{
//	uint32_t date;
//	uint16_t time;
//} PRINT_TimeCompareTypedef;

#pragma pack(pop)

/******************************************************************************/
void PRINT_Date(char* fileName);
void PRINT_TitleOut(void);
void PRINT_TailOut(void);
void PRINT_SetMode(void);
void PRINT_PrintProcess(DISPLAY_CompareTimeTypedef* startTime,
						DISPLAY_CompareTimeTypedef* endTime,
						ChannelSelectTypedef* select);















#endif
