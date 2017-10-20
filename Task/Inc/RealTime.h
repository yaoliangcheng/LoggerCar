#ifndef __REAL_TIME_H
#define __REAL_TIME_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "rt.h"

/******************************************************************************/
#define REALTIME_SAVE_INTERVAL					(1)

/******************************************************************************/
#define REALTIME_TASK_TIME_ANALOG_UPDATE		(1 << 0)	/* 时间和模拟量更新 */
#define REALTIME_TASK_ALRAM_RECORD				(1 << 1)	/* 闹钟触发记录 */
#define REALTIME_TASK_SENSOR_CONVERT_FINISH		(1 << 2)	/* 传感器转换完成  */

/******************************************************************************/
void REALTIME_Task(void);

#endif

