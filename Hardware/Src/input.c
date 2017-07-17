#include "input.h"

/*******************************************************************************
 * 检测外部电源状态
 */
uint8_t INPUT_CheckPwrOnStatus(void)
{
	uint8_t stat;

	stat = HAL_GPIO_ReadPin(PWR_ON_CHECK_GPIO_Port, PWR_ON_CHECK_Pin);
	osDelay(2);
	if (stat == HAL_GPIO_ReadPin(PWR_ON_CHECK_GPIO_Port, PWR_ON_CHECK_Pin))
		return stat;
	else
		return !stat;
}



