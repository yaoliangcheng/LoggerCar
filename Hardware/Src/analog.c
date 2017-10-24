#include "analog.h"

#include "ntc.h"
#include "hih5030.h"
#include "param.h"

/******************************************************************************/
static uint16_t convertValueBuffer[ANALOG_SAMPLE_NUMB][ANALOG_CHANNEL_NUMB_TOTLE];
ANALOG_ValueTypedef ANALOG_value;
ANALOG_AlarmStatusTypedef ANALOG_alarmStatus;
BOOL ANALOG_alarmOccur = FALSE;				/* 模拟量报警发生 */
ANALOG_ModeEnum ANALOG_Mode;				/* 模拟量报警模式 */

/******************************************************************************/
static void ANALOG_GetAverageValue(ANALOG_ConvertValueTypedef* convertValue);
static uint8_t ANALOG_GetBatVoltage(uint16_t value);

/*******************************************************************************
 * @brief 模拟量初始化
 * 注意：没有经过电压校准的ADC采样会有较大偏差
 */
void ANALOG_Init(void)
{
	/* 校准ADC */
	HAL_ADCEx_Calibration_Start(&ANALOG_ADC);
}

/*******************************************************************************
 * @brief 使能ADC转换，转换之前需控制模拟量电源开关，并有适当的延时
 */
void ANALOG_ConvertEnable(void)
{
	/* 传感器电源控制到采集，加入适当的延时 */
	ANALOG_PWR_ENABLE();
	VBAT_PWR_CHECK_ENABLE();
	osDelay(50);
	HAL_ADC_Start_DMA(&ANALOG_ADC, (uint32_t*)convertValueBuffer,
								sizeof(convertValueBuffer));
}

/*******************************************************************************
 * function：模拟量停止转换，并关闭模拟量电源开关
 */
void ANALOG_ConvertDisable(void)
{
	HAL_ADC_Stop_DMA(&ANALOG_ADC);
	ANALOG_PWR_DISABLE();
	VBAT_PWR_CHECK_DISABLE();
}

/*******************************************************************************
 * @brief 获取模拟量的值,如果AD值<通道AD最低值ANALOG_CHANNEL_AD_VALUE_MIN，
 * 		  则证明该通道传感器未连接或者已损坏,则该通道数值标志位ANALOG_CHANNLE_INVALID_VALUE
 */
void ANALOG_GetSensorValue(void)
{
	ANALOG_ConvertValueTypedef ANALOG_convertValue;

	/* 获取AD转换值 */
	ANALOG_GetAverageValue(&ANALOG_convertValue);

	/* 获取温度 */
	ANALOG_value.temp1 = NTC_GetTemp(ANALOG_convertValue.temp1);
	ANALOG_value.temp2 = NTC_GetTemp(ANALOG_convertValue.temp2);
	ANALOG_value.temp3 = NTC_GetTemp(ANALOG_convertValue.temp3);
	ANALOG_value.temp4 = NTC_GetTemp(ANALOG_convertValue.temp4);

	/* 获取湿度，并补偿 */
	ANALOG_value.humi1 = HIH5030_GetHumi(ANALOG_convertValue.humi1, ANALOG_value.temp1);
	ANALOG_value.humi2 = HIH5030_GetHumi(ANALOG_convertValue.humi2, ANALOG_value.temp2);
	ANALOG_value.humi3 = HIH5030_GetHumi(ANALOG_convertValue.humi3, ANALOG_value.temp3);
	ANALOG_value.humi4 = HIH5030_GetHumi(ANALOG_convertValue.humi4, ANALOG_value.temp4);

	/* 获取电池电压 */
	ANALOG_value.batVoltage = ANALOG_GetBatVoltage(ANALOG_convertValue.batVoltage);
}

/*******************************************************************************
 * @brief 把模拟量值转换为ASCII
 * @param buffer：储存地址指针
 * @param value:模拟量
 */
void ANALOG_Float2ASCII(FILE_SaveInfoAnalogTypedef* buffer, float value)
{
	if (value == ANALOG_CHANNLE_INVALID_VALUE)
		memcpy(buffer->value, ANALOG_INVALID_VALUE, 5);
	else
	{
		/* %5.1表示有效数据长度为5，小数1位 */
		sprintf(buffer->value, "%5.1f", value);
		buffer->str = ',';
	}
}

/*******************************************************************************
 * function：将AD值从大到小排序，取中间的数值
 */
static void ANALOG_GetAverageValue(ANALOG_ConvertValueTypedef* convertValue)
{
	uint8_t i, j, k;
	uint16_t value;
	uint32_t average;

	/* 通道数 */
	for (i = RESET; i < ANALOG_CHANNEL_NUMB_TOTLE; i++)
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
 * @brief 获取锂电池电压，采用3颗等值电阻分压
 * value：锂电池分压后的AD值
 * 注意：两节锂电池的电压范围为6.4~8.4V
 */
static uint8_t ANALOG_GetBatVoltage(uint16_t value)
{
	uint16_t voltage;
	uint8_t  percent;

	/* 获取电压值 */
	voltage = (uint16_t)((((uint32_t)value * 3300) / 4096) * 3 - 6000);

	percent = (voltage * 100) / 2400;

	if (percent > 100)
	{
		percent = 100;
	}

	return percent;
}




