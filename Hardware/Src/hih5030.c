#include "hih5030.h"

/*******************************************************************************
 * HIH5030温度补偿。
 * @param currentHumi：当前湿度
 * @param currentTemp：当前温度
 * return 温度补偿后的值。
 *
 * HIH5030温度补偿公式：真实RH = （传感器RH）/ （1.0546 - 0.00216 * 当前温度）；
 */
static float HIH5030_Adjust(float currentHumi, float currentTemp)
{
	return (float)(currentHumi / (1.0546 - 0.00216 * currentTemp));
}

/*******************************************************************************
 * function：HIH5030获取湿度
 * @param analogValue:AD转换值
 * @param currentTemp：当前温度值，用于湿度传感器的温度补偿
 */
float HIH5030_GetHumi(uint16_t analogValue, float currentTemp)
{
	float humiValue, voltage;

	/* 获取电压值 */
	voltage = (float)((3.300 * analogValue) / 4096);

	/* 获取湿度值 */
	humiValue = (float)(((float)(voltage / 3.300) - 0.1515) / 0.00636);

	/* 温度补偿值 */
	humiValue = HIH5030_Adjust(humiValue, currentTemp);
	
	return humiValue;
}


