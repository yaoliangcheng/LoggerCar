#include "../Inc/lowPwr.h"
#include "gprs.h"

/******************************************************************************/
static FunctionalState LOWPWR_KeyStatusCheck(void);
static void LOWPWR_EnterStandbyMode(void);

/*******************************************************************************
 *
 */
void LOWPWR_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* 如果是低功耗模式的启动才需要长按开机键 */
	if (__HAL_PWR_GET_FLAG(PWR_FLAG_WU) == TRUE)
	{
		/* 按键无效，重新进入待机模式 */
		if (DISABLE == LOWPWR_KeyStatusCheck())
		{
			/* 进入待机模式 */
			LOWPWR_EnterStandbyMode();
		}
	}

	/* 清除所有复位标志位 */
	__HAL_RCC_CLEAR_RESET_FLAGS();
}

/*******************************************************************************
 *
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_PIN_0 == GPIO_Pin)
	{
		/* 如果按键被按下超过3s */
		if (ENABLE == LOWPWR_KeyStatusCheck())
		{
			/* 进入待机模式 */
			LOWPWR_EnterStandbyMode();
		}
	}
}

/*******************************************************************************
 * function:检测按键状态
 * return：enable = 按键被按下
 *        disable = 按键无效
 */
static FunctionalState LOWPWR_KeyStatusCheck(void)
{
	uint8_t pressCnt = 0, releaseCnt = 0;

	/* 检测PA0引脚上的电平为持续50 * 100ms = 5s的高电平 */
	while (1)
	{
		if (HAL_GPIO_ReadPin(WKUP_GPIO_Port, WKUP_Pin) == GPIO_PIN_SET)
		{
			pressCnt++;
			releaseCnt = 0;

			GPRS_PWR_CTRL_ENABLE();
			if (pressCnt >= 50)
			{
				return ENABLE;
			}
		}
		else
		{
			pressCnt = 0;
			releaseCnt++;
			/* 检测到连续释放5次 */
			if (releaseCnt > 5)
			{
				GPRS_PWR_CTRL_DISABLE();
				return DISABLE;
			}
		}
		osDelay(100);
	}
}

/*******************************************************************************
 *
 */
static void LOWPWR_EnterStandbyMode(void)
{
	/* 复位电源 */
//	__HAL_RCC_PWR_FORCE_RESET();

	/* 使能电源接口时钟 */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* 清除唤醒标志位 */
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

	HAL_PWR_EnterSTANDBYMode();
}
