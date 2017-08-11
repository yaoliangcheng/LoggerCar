#include "led.h"

/******************************************************************************/

/******************************************************************************/
void LED_Task(void)
{
	while(1)
	{
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		HAL_Delay(50);
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
		HAL_Delay(3000);
	}
}

