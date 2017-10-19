#include "led.h"
#include "analog.h"

/******************************************************************************/
extern ANALOG_ModeEnum ANALOG_Mode;

/*******************************************************************************
 * 正常模式3s闪灯一次，
 * 预警模式2s闪灯一次，2s蜂鸣器响一次，一次持续50ms
 * 报警模式1s闪灯一次，蜂鸣器响1s，停1s
 *
 */
void LED_Task(void)
{
	while(1)
	{
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		HAL_Delay(50);
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
//		if (ANALOG_Mode == ANALOG_MODE_ALARM)
//		{
//			HAL_GPIO_TogglePin(O_VLIGHT_GPIO_Port, O_VLIGHT_Pin);
//			osDelay(1000);
//		}
//		else if (ANALOG_Mode == ANALOG_MODE_PERWARM)
//		{
//			HAL_GPIO_WritePin(O_VLIGHT_GPIO_Port, O_VLIGHT_Pin, GPIO_PIN_SET);
//			osDelay(50);
//			HAL_GPIO_WritePin(O_VLIGHT_GPIO_Port, O_VLIGHT_Pin, GPIO_PIN_RESET);
//			osDelay(2000);
//		}
//		else
		{
			osDelay(3000);
		}
	}
}

