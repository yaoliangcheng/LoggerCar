#ifndef _MAIN_TASK_H
#define _MAIN_TASK_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

/******************************************************************************/
#define MAINPROCESS_TICKS_TO_TIMEOUT			(20000)		/* 信号量等待超时 */
#define MAINPROCESS_START_TASK					(1 << 1)	/* 开启任务 */
#define MAINPROCESS_GPS_CONVERT_FINISH			(1 << 2)	/* GPS转换完成 */
#define MAINPROCESS_GPRS_SEND_FINISHED			(1 << 3)	/* GPRS数据发送完成 */
#define MAINPROCESS_GPRS_SEND_ERROR				(1 << 4)	/* GPRS数据发送错误 */


/******************************************************************************/
void MAINPROCESS_Task(void);

#endif
