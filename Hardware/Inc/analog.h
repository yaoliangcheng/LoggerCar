#ifndef __ANALOG_H
#define __ANALOG_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "adc.h"
#include "public.h"

/******************************************************************************/
#define ANALOG_ADC								(hadc1)
/* 传感器电源控制引脚 */
#define ANALOG_PWR_ENABLE() \
	HAL_GPIO_WritePin(O_VSENSOR_GPIO_Port, O_VSENSOR_Pin, GPIO_PIN_RESET);
#define ANALOG_PWR_DISABLE() \
	HAL_GPIO_WritePin(O_VSENSOR_GPIO_Port, O_VSENSOR_Pin, GPIO_PIN_SET);

/* 锂电池电压采集电源控制引脚 */
#define VBAT_PWR_CHECK_ENABLE() \
	HAL_GPIO_WritePin(O_VBAT_GPIO_Port, O_VBAT_Pin, GPIO_PIN_RESET);
#define VBAT_PWR_CHECK_DISABLE() \
	HAL_GPIO_WritePin(O_VBAT_GPIO_Port, O_VBAT_Pin, GPIO_PIN_SET);

#define ANALOG_SAMPLE_NUMB						(20)	/* 模拟量采样数 */

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)											/* 按字节对齐 */

typedef struct
{
	uint16_t temp1;				/* temp1转换值 */
	uint16_t temp2;				/* temp2转换值 */
	uint16_t temp3;				/* temp3转换值 */
	uint16_t temp4;				/* temp4转换值 */

	uint16_t humi1;				/* humi1转换值 */
	uint16_t humi2;				/* humi2转换值 */
	uint16_t humi3;				/* humi3转换值 */
	uint16_t humi4;				/* humi4转换值 */

	uint16_t batVoltage;			/* 电池电压 */
} ANALOG_ConvertValueTypedef;

typedef struct
{
	float temp1;				/* temp1转换值 */
	float temp2;				/* temp2转换值 */
	float temp3;				/* temp3转换值 */
	float temp4;				/* temp4转换值 */

	float humi1;				/* humi1转换值 */
	float humi2;				/* humi2转换值 */
	float humi3;				/* humi3转换值 */
	float humi4;				/* humi4转换值 */

	uint8_t batVoltage;				/* 电池电压 */
} ANALOG_ValueTypedef;

typedef struct
{
	union
	{
		struct
		{
			uint8_t ch1:1;
			uint8_t ch2:1;
			uint8_t ch3:1;
			uint8_t ch4:1;
			uint8_t ch5:1;
			uint8_t ch6:1;
			uint8_t ch7:1;
			uint8_t ch8:1;
		} bit;
		uint8_t all;
	} status;
} ANALOG_AlarmStatusTypedef;

#pragma pack(pop)

/******************************************************************************/
extern ANALOG_ValueTypedef ANALOG_value;
extern ANALOG_AlarmStatusTypedef ANALOG_alarmStatus;
extern BOOL ANALOG_alarmOccur;

/******************************************************************************/
void ANALOG_Init(void);
void ANALOG_ConvertEnable(void);
void ANALOG_ConvertDisable(void);
void ANALOG_GetSensorValue(void);
void ANALOG_Float2ASCII(char* buffer, float value);

#endif
