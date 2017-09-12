#ifndef __PARAM_H
#define __PARAM_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "common.h"

/******************************************************************************/
#define ANALOG_CHANNEL_NUMB_MAX					(14)	/* ���֧��14ͨ����ģ���� */
#define ANALOG_CHANNEL_NUMB_TOTLE      			(9)		/* ģ����ͨ����(������ʪ�Ⱥ�﮵�ص�ѹ�ɼ�) */
#define ANALOG_CHANNEL_NUMB      				(8)		/* ģ����ͨ���� */

/******************************************************************************/
typedef enum
{
	LOCATION_BASE_SATTION,
	LOCATION_GPS
} LocationTypdEnum;

typedef enum
{
	TYPE_TEMP = 1,									/* �¶� */
	TYPE_HUMI,										/* ʪ�� */
	TYPE_RAINFALL,									/* ���� */
	TYPE_SOIL_MOISTURE,								/* ����ˮ�� */
	TYPE_VOLTAGE,									/* ��ѹ */
	TYPE_ELECTRIC_CURRENT,							/* ���� */
	TYPE_POWER,										/* ���� */
	TYPE_ILLUMINATION_INTENSITY,					/* ����ǿ�� */
	TYPE_WIND_SPEED,								/* ���� */
	TYPE_WIND_DIR,									/* ���� */
	TYPE_CO2,										/* CO2 */
	TYPE_BAROMETRIC_PRESSURE,						/* ����ѹ */
	TYPE_O2,										/* O2 */
	TYPE_NUMB_OF_TIMES,								/* ���� */
	TYPE_WATER_LEVEL,								/* ˮλ */
	TYPE_PRESSURE,									/* ѹ�� */
	TYPE_PH_VALUE,									/* PHֵ */
} ChannelTypeEnum;

typedef enum
{
	UNIT_TEMP = 1,									/* �¶� */
	UNIT_HUMI,										/* ʪ�� */
	UNIT_RAINFALL,									/* ���� */
	UNIT_SOIL_MOISTURE,								/* ����ˮ�� */
	UNIT_VOLTAGE,									/* ��ѹ */
	UNIT_ELECTRIC_CURRENT,							/* ���� */
	UNIT_POWER,										/* ���� */
	UNIT_ILLUMINATION_INTENSITY,					/* ����ǿ�� */
	UNIT_WIND_SPEED,								/* ���� */
	UNIT_WIND_DIR,									/* ���� */
	UNIT_CO2,										/* CO2 */
	UNIT_BAROMETRIC_PRESSURE,						/* ����ѹ */
	UNIT_O2,										/* O2 */
	UNIT_NUMB_OF_TIMES,								/* ���� */
	UNIT_WATER_LEVEL,								/* ˮλ */
	UNIT_PRESSURE,									/* ѹ�� */
	UNIT_PH_VALUE,									/* PHֵ */
} ChannelUnitEnum;

typedef enum
{
	FORMAT_INT = 1,									/* ���ݸ�ʽ���� */
	FORMAT_ONE_DECIMAL,								/* ���ݸ�ʽ1λС�� */
	FORMAT_TWO_DECIMAL,								/* ���ݸ�ʽ2λС�� */
} DataFormatEnum;

/**************************������������*******************************************/
typedef struct
{
	ChannelTypeEnum channelType;					/* ͨ������ */
	ChannelUnitEnum channelUnit;					/* ͨ����λ */
	DataFormatEnum  dataFormat;						/* ������ʽ */
} ParamTypeTypedef;

typedef struct
{
	float alarmValueUp;								/* �������� */
	float alarmValueLow;							/* �������� */
	float perwarningValueUp;						/* Ԥ������ */
	float perwarningValueLow;						/* Ԥ������ */
} ParamAlarmTypedef;

typedef struct
{
	char    deviceSN[10];									/* �豸SN�� */
	LocationTypdEnum locationType;							/* ��λ��� */
	uint8_t firmwareVersion;								/* �̼��汾�� */
	uint8_t recordInterval;									/* ��¼��� */
	uint8_t overLimitRecordInterval;						/* �����¼��� */
	uint8_t exitAnalogChannelNumb;							/* �ⲿģ����ͨ���� */
	ParamTypeTypedef param[ANALOG_CHANNEL_NUMB];			/* ģ�������� */
	ParamAlarmTypedef temp1;								/* temp1����ֵ */
	ParamAlarmTypedef temp2;								/* temp2����ֵ */
	ParamAlarmTypedef temp3;								/* temp3����ֵ */
	ParamAlarmTypedef temp4;								/* temp4����ֵ */
} PARAM_DeviceParamTypedef;

/******************************************************************************/
extern PARAM_DeviceParamTypedef PARAM_DeviceParam;

/******************************************************************************/
ErrorStatus PARAM_ParamFileInit(void);

#endif