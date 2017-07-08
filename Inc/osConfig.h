#ifndef _OS_CONFIG_H
#define _OS_CONFIG_H

#include "stm32f1xx_hal.h"
#include "main.h"

/******************************************************************************/
extern osThreadId ledTaskHandle;
extern osThreadId debugTaskHandle;
extern osThreadId realtimeTaskHandle;

#endif
