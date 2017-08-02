#ifndef _MAIN_TASK_H
#define _MAIN_TASK_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

/******************************************************************************/
#define MAINPROCESS_TICKS_TO_TIMEOUT			(20000)		/* 信号量等待超时 */
#define MAINPROCESS_GPS_CONVERT_FINISH			(1 << 1)	/* GPS转换完成 */
#define MAINPROCESS_GPRS_SEND_FINISHED			(1 << 2)	/* GPRS数据发送完成 */



/******************************************************************************/
void MAINPROCESS_Task(void);

#endif
