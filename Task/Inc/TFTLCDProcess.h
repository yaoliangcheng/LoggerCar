#ifndef __TFTLCDPROCESS_H
#define __TFTLCDPROCESS_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

/******************************************************************************/
#define TFTLCD_TASK_RECV_ENABLE				(1 << 0)

/******************************************************************************/
typedef enum
{
	TFT_PRINT_START_TIME,
	TFT_PRINT_END_TIME,
} TFTTASK_StatusEnum;

/******************************************************************************/
typedef struct
{
	union
	{
		struct
		{
			uint8_t ch1:1;
			uint8_t ch2:1;
			uint8_t ch3:1;
			uint8_t ch4:1;
			uint8_t ch5:1;
			uint8_t ch6:1;
			uint8_t ch7:1;
			uint8_t ch8:1;
		} bit;
		uint8_t all;
	} status;
} PRINT_ChannelSelectTypedef;

/******************************************************************************/
void TFTLCD_Task(void);

#endif
