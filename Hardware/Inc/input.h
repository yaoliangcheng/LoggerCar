#ifndef __INPUT_H
#define __INPUT_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

/******************************************************************************/
#define INPUT_CheckPwrOnStatus() \
					HAL_GPIO_ReadPin(I_PWR_ON_GPIO_Port, I_PWR_ON_Pin)

/******************************************************************************/
//uint8_t INPUT_CheckPwrOnStatus(void);

#endif
