#include "ntc.h"
#include "analog.h"

/*******************************************************************************
 * 根据电阻值查表得出温度值
 * @ param：analogValue:采样AD值
 * @ output：float输出温度浮点型数值
 */
static float NTC_CheckTable(volatile uint32_t resValue)
{
	volatile uint8_t vIndex, hIndex;
	float temp;

	/* 电阻值无效 */
	if ((resValue > NTC_Table[0][0])
			|| (resValue < NTC_Table[NTC_TABLE_LAYER - 1][NTC_TABLE_INDEX - 1]))
	{
		return temp = NULL;
	}

	/* 纵向查表 */
	for (vIndex = RESET; vIndex < NTC_TABLE_LAYER; vIndex++)
	{
		if (resValue > NTC_Table[vIndex][1])
		{
			vIndex--;
			break;
		}
	}

	/* 横向查表 */
	for (hIndex = 1; hIndex < NTC_TABLE_INDEX; hIndex++)
	{
		if (resValue > NTC_Table[vIndex][hIndex])
		{
			break;
		}
	}

	/* 二维数组之间的值 */
	if (NTC_TABLE_INDEX == hIndex)
	{
		temp = (float)(NTC_Table[vIndex][hIndex - 1] - resValue)
			/ (float)(NTC_Table[vIndex][hIndex - 1] - NTC_Table[vIndex + 1][0]);

		temp += (float)(((vIndex * 10) + (hIndex - 1)) - NTC_TABLE_TEMP_OFFSET);
	}
	else /* 正常列表中的值 */
	{
		temp = (float)(NTC_Table[vIndex][hIndex - 1] - resValue)
		   / (float)(NTC_Table[vIndex][hIndex - 1] - NTC_Table[vIndex][hIndex]);

		temp += (float)(((vIndex * 10) + (hIndex - 1)) - NTC_TABLE_TEMP_OFFSET);
	}

	return temp;
}

/*******************************************************************************
 * @analogValue:AD值
 */
float NTC_GetTemp(uint16_t analogValue)
{
	uint32_t resValue;

	if (analogValue < ANALOG_CHANNEL_AD_VALUE_MIN)
		return ANALOG_CHANNLE_INVALID_VALUE;

	/* 将AD值转换成电阻值 */
	resValue = (uint32_t)(((uint32_t)(STM32_AD_FULL_VALUE * NTC_REF_RES)
					/ analogValue) - NTC_REF_RES);

	/* 计算温度并返回 */
	return NTC_CheckTable(resValue);
}
