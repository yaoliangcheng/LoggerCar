#ifndef __ANALOG_H
#define __ANALOG_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "adc.h"
#include "public.h"
//#include "file.h"

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
#define ANALOG_CHANNEL_AD_VALUE_MIN				(20)	/* 通道AD最低值 */
#define ANALOG_CHANNLE_INVALID_VALUE			(float)(-127)	/* 通道无效值标志值 */

#define ANALOG_INVALID_VALUE					(" NULL") /* 无效值 */

/******************************************************************************/
typedef enum
{
	ANALOG_MODE_NORMAL,				/* 正常模式 */
	ANALOG_MODE_PERWARM,			/* 预警模式 */
	ANALOG_MODE_ALARM				/* 报警模式 */
} ANALOG_ModeEnum;					/* 模拟量模式 */

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)						/* 按字节对齐 */

typedef struct
{
	uint16_t batVoltage;			/* 电池电压 */
	uint16_t channel1;				/* channel1转换值 */
	uint16_t channel2;				/* channel2转换值 */
	uint16_t channel3;				/* channel3转换值 */
	uint16_t channel4;				/* channel4转换值 */
	uint16_t channel5;				/* channel5转换值 */
	uint16_t channel6;				/* channel6转换值 */
	uint16_t channel7;				/* channel7转换值 */
	uint16_t channel8;				/* channel8转换值 */
} ANALOG_ConvertValueTypedef;

typedef struct
{
	uint8_t batVoltage;				/* 电池电压 */
	float channel1;					/* channel1转换值 */
	float channel2;					/* channel2转换值 */
	float channel3;					/* channel3转换值 */
	float channel4;					/* channel4转换值 */
	float channel5;					/* channel5转换值 */
	float channel6;					/* channel6转换值 */
	float channel7;					/* channel7转换值 */
	float channel8;					/* channel8转换值 */
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
//void ANALOG_Float2ASCII(FILE_SaveInfoAnalogTypedef* buffer, float value);

#endif
