#ifndef __EEPROM_H
#define __EEPROM_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "stm32f105xc.h"
#include "i2c.h"

#include "analog.h"



/******************************************************************************/
#define EEPROM_I2C					(hi2c2)
#define EEPROM_DEVICE_ADDR			(0xA0)		/* EEPROM设备地址 */

#define EE_ADDR_BASE							(0X000)
#define EE_ADDR_DEVICE_INIT						(0x000)
#define EE_ADDR_DEVICE_SN						(0x001)
#define EE_ADDR_FIRMWARE_VERSION				(0x00B)
#define EE_ADDR_RECORD_INTERVAL					(0x00C)
#define EE_ADDR_OVER_LIMIT_RECORD_INTERVAL		(0x00E)
#define EE_ADDR_EXIT_ANALOG_CHANNEL_NUMB		(0x00E)
#define EE_ADDR_PARAM_1							(0x00F)
#define EE_ADDR_PARAM_2							(0x012)
#define EE_ADDR_PARAM_3							(0x015)
#define EE_ADDR_PARAM_4							(0x018)
#define EE_ADDR_PARAM_5							(0x01B)
#define EE_ADDR_PARAM_6							(0x02E)
#define EE_ADDR_PARAM_7							(0x021)
#define EE_ADDR_PARAM_8							(0x024)
#define EE_ADDR_PARAM_9							(0x027)
#define EE_ADDR_PARAM_10						(0x02A)
#define EE_ADDR_PARAM_11						(0x02D)
#define EE_ADDR_PARAM_12						(0x030)
#define EE_ADDR_PARAM_13						(0x033)
#define EE_ADDR_PARAM_14						(0x036)

#define EE_ADDR_FLASH_INFO_SAVE_ADDR			(0x039)
#define EE_ADDR_FLASH_INFO_READ_ADDR			(0x043)


/******************************************************************************/
typedef enum
{
	LOCATION_STATION,								/* 基站定位 */
	LOCATION_GPS,									/* GPS定位 */
} EE_LocationTypeEnum;

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
} EE_ChannelTypeEnum;

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
} EE_ChannelUnitEnum;

typedef enum
{
	FORMAT_INT = 1,									/* 数据格式整数 */
	FORMAT_ONE_DECIMAL,								/* 数据格式1位小数 */
	FORMAT_TWO_DECIMAL,								/* 数据格式2位小数 */
} EE_DataFormatEnum;

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)											/* 按字节对齐 */

typedef struct
{
	EE_ChannelTypeEnum channelType;					/* 通道类型 */
	EE_ChannelUnitEnum channelUnit;					/* 通道单位 */
	EE_DataFormatEnum  dataFormat;					/* 数据形式 */
} EE_ParamTypedef;

#pragma pack(pop)
/******************************************************************************/
extern uint8_t EE_DeviceInit;
extern char 	EE_deviceSN[10];
extern uint8_t EE_firmwareVersion;
extern uint8_t EE_recordInterval;
extern uint8_t EE_overLimitRecordInterval;
extern uint8_t EE_exitAnalogChannelNumb;
extern EE_ParamTypedef EE_Param[ANALOG_CHANNEL_NUMB_MAX];
extern uint32_t EE_FlashInfoSaveAddr;
extern uint32_t EE_FlashInfoReadAddr;

/******************************************************************************/
void EEPROM_WriteBytes(uint16_t addr, void* pBuffer, uint8_t dataLength);
void EEPROM_ReadBytes(uint16_t addr, uint8_t* pBuffer, uint8_t dataLength);
void DEVICE_Init(void);
#endif
