#ifndef __PARAM_H
#define __PARAM_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "common.h"

/******************************************************************************/
#define ANALOG_CHANNEL_NUMB_MAX					(14)	/* 最大支持14通道的模拟量 */
#define ANALOG_CHANNEL_NUMB_TOTLE      			(9)		/* 模拟量通道数(包含温湿度和锂电池电压采集) */
#define ANALOG_CHANNEL_NUMB      				(8)		/* 模拟量通道数 */

/******************************************************************************/
typedef enum
{
	LOCATION_BASE_SATTION,
	LOCATION_GPS
} LocationTypdEnum;

typedef enum
{
	TYPE_TEMP = 1,									/* 温度 */
	TYPE_HUMI,										/* 湿度 */
	TYPE_RAINFALL,									/* 雨量 */
	TYPE_SOIL_MOISTURE,								/* 土壤水分 */
	TYPE_VOLTAGE,									/* 电压 */
	TYPE_ELECTRIC_CURRENT,							/* 电流 */
	TYPE_POWER,										/* 功率 */
	TYPE_ILLUMINATION_INTENSITY,					/* 光照强度 */
	TYPE_WIND_SPEED,								/* 风速 */
	TYPE_WIND_DIR,									/* 风向 */
	TYPE_CO2,										/* CO2 */
	TYPE_BAROMETRIC_PRESSURE,						/* 大气压 */
	TYPE_O2,										/* O2 */
	TYPE_NUMB_OF_TIMES,								/* 次数 */
	TYPE_WATER_LEVEL,								/* 水位 */
	TYPE_PRESSURE,									/* 压力 */
	TYPE_PH_VALUE,									/* PH值 */
} ChannelTypeEnum;

typedef enum
{
	UNIT_TEMP = 1,									/* 温度 */
	UNIT_HUMI,										/* 湿度 */
	UNIT_RAINFALL,									/* 雨量 */
	UNIT_SOIL_MOISTURE,								/* 土壤水分 */
	UNIT_VOLTAGE,									/* 电压 */
	UNIT_ELECTRIC_CURRENT,							/* 电流 */
	UNIT_POWER,										/* 功率 */
	UNIT_ILLUMINATION_INTENSITY,					/* 光照强度 */
	UNIT_WIND_SPEED,								/* 风速 */
	UNIT_WIND_DIR,									/* 风向 */
	UNIT_CO2,										/* CO2 */
	UNIT_BAROMETRIC_PRESSURE,						/* 大气压 */
	UNIT_O2,										/* O2 */
	UNIT_NUMB_OF_TIMES,								/* 次数 */
	UNIT_WATER_LEVEL,								/* 水位 */
	UNIT_PRESSURE,									/* 压力 */
	UNIT_PH_VALUE,									/* PH值 */
} ChannelUnitEnum;

typedef enum
{
	FORMAT_INT = 1,									/* 数据格式整数 */
	FORMAT_ONE_DECIMAL,								/* 数据格式1位小数 */
	FORMAT_TWO_DECIMAL,								/* 数据格式2位小数 */
} DataFormatEnum;

/**************************参数描述部分*******************************************/
typedef struct
{
	ChannelTypeEnum channelType;					/* 通道类型 */
	ChannelUnitEnum channelUnit;					/* 通道单位 */
	DataFormatEnum  dataFormat;						/* 数据形式 */
} ParamTypeTypedef;

typedef struct
{
	float alarmValueUp;								/* 报警上限 */
	float alarmValueLow;							/* 报警下限 */
	float perwarningValueUp;						/* 预警上限 */
	float perwarningValueLow;						/* 预警下限 */
} ParamAlarmTypedef;

typedef struct
{
	char    deviceSN[10];									/* 设备SN号 */
	LocationTypdEnum locationType;							/* 定位标记 */
	uint8_t firmwareVersion;								/* 固件版本号 */
	uint8_t recordInterval;									/* 记录间隔 */
	uint8_t overLimitRecordInterval;						/* 超标记录间隔 */
	uint8_t exitAnalogChannelNumb;							/* 外部模拟量通道数 */
	ParamTypeTypedef param[ANALOG_CHANNEL_NUMB];			/* 模拟量参数 */
	ParamAlarmTypedef channel[ANALOG_CHANNEL_NUMB];			/* 报警值 */
} PARAM_DeviceParamTypedef;

/******************************************************************************/
extern PARAM_DeviceParamTypedef PARAM_DeviceParam;

/******************************************************************************/
ErrorStatus PARAM_ParamFileInit(void);

#endif
