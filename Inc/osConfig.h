#ifndef _OS_CONFIG_H
#define _OS_CONFIG_H

#include "stm32f1xx_hal.h"
#include "main.h"

/******************************************************************************/
extern osThreadId ledTaskHandle;
extern osThreadId debugTaskHandle;
extern osThreadId realtimeTaskHandle;
extern osThreadId tftlcdTaskHandle;
extern osThreadId mainprocessTaskHandle;
extern osThreadId gprsprocessTaskHandle;

extern osMessageQId realtimeMessageQId;
extern osMessageQId analogMessageQId;
extern osMessageQId infoMessageQId;
extern osMessageQId infoCntMessageQId;

#endif
