#include "analog.h"

#include "ntc.h"
#include "hih5030.h"

/******************************************************************************/
static uint16_t convertValueBuffer[ANALOG_SAMPLE_NUMB][ANALOG_CHANNEL_NUMB];

/*******************************************************************************
 *
 */
static void ANALOG_GetAverageValue(ANALOG_ConvertValueTypedef* convertValue)
{
	uint8_t i, j, k;
	uint16_t value;
	uint32_t average;

	/* 通道数 */
	for (i = RESET; i < ANALOG_CHANNEL_NUMB; i++)
	{
		/* 排序,按降序排列 */
		for (j = RESET; j < ANALOG_SAMPLE_NUMB - 1; j++)
		{
			for (k = j; k < ANALOG_SAMPLE_NUMB; k++)
			{
				if (convertValueBuffer[k][i] > convertValueBuffer[j][i])
				{
					value = convertValueBuffer[j][i];
					convertValueBuffer[j][i] = convertValueBuffer[k][i];
					convertValueBuffer[k][i] = value;
				}
			}
		}

		/* 清空平均值 */
		average = RESET;

		/* 取中间的数值的平均数 */
		for(j = ANALOG_SAMPLE_NUMB / 2 - 5; j < ANALOG_SAMPLE_NUMB / 2 + 5; j++)
		{
			average += convertValueBuffer[j][i];
		}
		*(&convertValue->temp1 + i) = (uint16_t)(average / 10);
	}
}

/*******************************************************************************
 *
 */
static uint8_t ANALOG_GetBatVoltage(uint16_t value)
{
	uint32_t voltage;
	float percent;

	voltage = (uint32_t)((value * 3300) * 2 / 4096);

	percent = (float)voltage / 4200;

	if (percent > 1)
		percent = 1;

	return (uint8_t)(percent * 100);
}

/*******************************************************************************
 *
 */
void ANALOG_GetSensorValue(ANALOG_ValueTypedef* value)
{
	ANALOG_ConvertValueTypedef ANALOG_convertValue;

	/* 获取AD转换值 */
	ANALOG_GetAverageValue(&ANALOG_convertValue);

	/* 获取温度 */
	value->temp1 = NTC_GetTemp(ANALOG_convertValue.temp1);
	value->temp2 = NTC_GetTemp(ANALOG_convertValue.temp2);
	value->temp3 = NTC_GetTemp(ANALOG_convertValue.temp3);
	value->temp4 = NTC_GetTemp(ANALOG_convertValue.temp4);

	/* 获取湿度，并补偿 */
	value->humi1 = HIH5030_GetHumi(ANALOG_convertValue.humi1, value->temp1);
	value->humi2 = HIH5030_GetHumi(ANALOG_convertValue.humi2, value->temp2);
	value->humi3 = HIH5030_GetHumi(ANALOG_convertValue.humi3, value->temp3);
	value->humi4 = HIH5030_GetHumi(ANALOG_convertValue.humi4, value->temp4);

	/* 获取电池电压 */
	value->batVoltage = ANALOG_GetBatVoltage(ANALOG_convertValue.batVoltage);
}

/*******************************************************************************
 *
 */
void ANALOG_ConvertEnable(void)
{
	/* 传感器电源控制到采集，加入适当的延时 */
	ANALOG_PWR_ENABLE();
	osDelay(10);
	HAL_ADC_Start_DMA(&ANALOG_ADC, (uint32_t*)convertValueBuffer,
								sizeof(convertValueBuffer));
}

/*******************************************************************************
 *
 */
void ANALOG_ConvertDisable(void)
{
	HAL_ADC_Stop_DMA(&ANALOG_ADC);
	ANALOG_PWR_DISABLE();
}

