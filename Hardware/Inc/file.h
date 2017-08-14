#ifndef __FILE_H
#define __FILE_H


/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "common.h"
#include "TFTLCDProcess.h"


#include "rt.h"
#include "analog.h"
#include "gps.h"
#include "fatfs.h"

/******************************************************************************/
#define ANALOG_VALUE_FORMAT						(FORMAT_ONE_DECIMAL)
#define PATCH_PACK_FILE_NAME					("patch.txt")
#define PARAM_FILE_NAME							("param.txt")

#define FILE_PRINT_TO_END						(0xFFFF)

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

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)									/* 按字节对齐 */

typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
} FILE_RealTime;

typedef struct
{
	uint16_t temp1;
	uint16_t humi1;

	uint16_t temp2;
	uint16_t humi2;

	uint16_t temp3;
	uint16_t humi3;

	uint16_t temp4;
	uint16_t humi4;
} FILE_AnalogValueTypedef;

typedef struct
{
	FILE_RealTime realTime;						/* 时间 */
	uint8_t  batteryLevel;						/* 电池电量 */
	uint8_t  externalPowerStatus;				/* 外部电池状态 */
	uint32_t longitude;							/* 经度 */
	uint32_t latitude;							/* 纬度 */
	uint8_t  resever;							/* 保留 */
	FILE_AnalogValueTypedef analogValue;		/* 模拟量值 */
} FILE_InfoTypedef;

typedef struct
{
	char patchFileName[11];						/* 开始补传的文件名 */
	uint16_t patchStructOffset;					/* 开始补传的结构体在文件中的偏移 */
	uint16_t patchPackOver_5;					/* 补传超过5条 */
	uint16_t patchPackOver_10;					/* 补传超过10条 */
	uint16_t patchPackOver_20;					/* 补传超过20条 */
	uint16_t patchPackOver_30;					/* 补传超过30条 */
} FILE_PatchPackTypedef;

/**************************参数描述部分*******************************************/
typedef struct
{
	ChannelTypeEnum channelType;					/* 通道类型 */
	ChannelUnitEnum channelUnit;					/* 通道单位 */
	DataFormatEnum  dataFormat;					/* 数据形式 */
} ParamTypeTypedef;

typedef struct
{
	char    deviceSN[10];									/* 设备SN号 */
	LocationTypdEnum locationType;							/* 定位标记 */
	uint8_t firmwareVersion;								/* 固件版本号 */
	uint8_t recordInterval;									/* 记录间隔 */
	uint8_t overLimitRecordInterval;						/* 超标记录间隔 */
	uint8_t exitAnalogChannelNumb;							/* 外部模拟量通道数 */
	ParamTypeTypedef param[ANALOG_CHANNEL_NUMB];			/* 模拟量参数 */
} FILE_DeviceParamTypedef;

#pragma pack(pop)

/******************************************************************************/
extern FILE_DeviceParamTypedef FILE_DeviceParam;	/* 设备参数信息 */

/******************************************************************************/
void FILE_Init(void);
ErrorStatus FILE_SaveInfo(FILE_InfoTypedef* saveInfo, uint16_t* fileStructCount);
ErrorStatus FILE_ReadInfo(FILE_InfoTypedef* readInfo);
uint16_t FILE_ReadPatchInfo(FILE_PatchPackTypedef* patch, FILE_InfoTypedef* readInfo);
void FILE_InfoFormatConvert(FILE_InfoTypedef*    saveInfo,
							RT_TimeTypedef*      realTime,
							GPS_LocateTypedef*   location,
							ANALOG_ValueTypedef* analogValue);
ErrorStatus FILE_ReadPatchPackFile(FILE_PatchPackTypedef* pBuffer);
ErrorStatus FILE_WritePatchPackFile(FILE_PatchPackTypedef* pBuffer);
ErrorStatus FILE_ParamFileInit(void);
ErrorStatus FILE_PrintDependOnTime(FILE_RealTime* startTime, FILE_RealTime* stopTime,
		PRINT_ChannelSelectTypedef* select);

#endif
