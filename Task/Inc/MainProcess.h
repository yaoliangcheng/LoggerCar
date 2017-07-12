#ifndef _MAIN_TASK_H
#define _MAIN_TASK_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

/******************************************************************************/
#define MAINPROCESS_TICKS_TO_TIMEOUT		(20000)
#define MAINPROCESS_ENABLE_EVENT			(1 << 0)
#define MAINPROCESS_GET_SENSOR_ENABLE		(1 << 1)



/******************************************************************************/
void MAINPROCESS_Task(void);

#endif
