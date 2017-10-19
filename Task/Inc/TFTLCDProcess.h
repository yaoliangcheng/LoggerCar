#ifndef __TFTLCDPROCESS_H
#define __TFTLCDPROCESS_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

/******************************************************************************/
#define TFTLCD_TASK_RECV_ENABLE				(1 << 0)
#define TFTLCD_TASK_STATUS_BAR_UPDATE		(1 << 1)
#define TFTLCD_TASK_ANALOG_UPDATE			(1 << 2)
#define TFTLCD_TASK_BLE_RECV_ENABLE			(1 << 3)

/******************************************************************************/
typedef enum
{
	TFT_PRINT_START_TIME,
	TFT_PRINT_END_TIME,
} TFTTASK_StatusEnum;



/******************************************************************************/
void TFTLCD_Task(void);

#endif
