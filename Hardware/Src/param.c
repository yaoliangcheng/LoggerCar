#include "param.h"
#include "file.h"
#include "tftlcd.h"

/******************************************************************************/
PARAM_DeviceParamTypedef PARAM_DeviceParam;

/******************************************************************************/
void PARAM_ReadStruct(PARAM_DeviceParamTypedef* param);
void PARAM_AlarmLimitStructUpdate(uint16_t screenID, uint16_t ctlID, float data);

/*******************************************************************************
 * function:设备参数文件初始化
 */
void PARAM_ParamFileInit(void)
{
	PARAM_ReadStruct(&PARAM_DeviceParam);

	/* 判断SN号是否正常 */
	/* todo */

//	memcpy(PARAM_DeviceParam.deviceSN, "1708151515", sizeof(PARAM_DeviceParam.deviceSN));
//	PARAM_DeviceParam.locationType    = LOCATION_GPS;
//	PARAM_DeviceParam.firmwareVersion = 10;
//	PARAM_DeviceParam.recordInterval  = 2;
//	PARAM_DeviceParam.overLimitRecordInterval = 2;
//	PARAM_DeviceParam.exitAnalogChannelNumb = ANALOG_CHANNEL_NUMB;
//	PARAM_DeviceParam.param[0].channelType = TYPE_TEMP;
//	PARAM_DeviceParam.param[0].channelUnit = UNIT_TEMP;
//	PARAM_DeviceParam.param[0].dataFormat  = FORMAT_ONE_DECIMAL;
//	PARAM_DeviceParam.param[1].channelType = TYPE_HUMI;
//	PARAM_DeviceParam.param[1].channelUnit = UNIT_HUMI;
//	PARAM_DeviceParam.param[1].dataFormat  = FORMAT_ONE_DECIMAL;
//	PARAM_DeviceParam.param[2].channelType = TYPE_TEMP;
//	PARAM_DeviceParam.param[2].channelUnit = UNIT_TEMP;
//	PARAM_DeviceParam.param[2].dataFormat  = FORMAT_ONE_DECIMAL;
//	PARAM_DeviceParam.param[3].channelType = TYPE_HUMI;
//	PARAM_DeviceParam.param[3].channelUnit = UNIT_HUMI;
//	PARAM_DeviceParam.param[3].dataFormat  = FORMAT_ONE_DECIMAL;
//	PARAM_DeviceParam.param[4].channelType = TYPE_TEMP;
//	PARAM_DeviceParam.param[4].channelUnit = UNIT_TEMP;
//	PARAM_DeviceParam.param[4].dataFormat  = FORMAT_ONE_DECIMAL;
//	PARAM_DeviceParam.param[5].channelType = TYPE_HUMI;
//	PARAM_DeviceParam.param[5].channelUnit = UNIT_HUMI;
//	PARAM_DeviceParam.param[5].dataFormat  = FORMAT_ONE_DECIMAL;
//	PARAM_DeviceParam.param[6].channelType = TYPE_TEMP;
//	PARAM_DeviceParam.param[6].channelUnit = UNIT_TEMP;
//	PARAM_DeviceParam.param[6].dataFormat  = FORMAT_ONE_DECIMAL;
//	PARAM_DeviceParam.param[7].channelType = TYPE_HUMI;
//	PARAM_DeviceParam.param[7].channelUnit = UNIT_HUMI;
//	PARAM_DeviceParam.param[7].dataFormat  = FORMAT_ONE_DECIMAL;
//	memcpy(PARAM_DeviceParam.password, "0000", 4);

	/* 通道1参数更新 */
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH1,
			PARAM_DeviceParam.channel[0].alarmValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH1,
			PARAM_DeviceParam.channel[0].alarmValueLow);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH1,
			PARAM_DeviceParam.channel[0].perwarningValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH1,
			PARAM_DeviceParam.channel[0].perwarningValueLow);

	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH2,
			PARAM_DeviceParam.channel[1].alarmValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH2,
			PARAM_DeviceParam.channel[1].alarmValueLow);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH2,
			PARAM_DeviceParam.channel[1].perwarningValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH2,
			PARAM_DeviceParam.channel[1].perwarningValueLow);

	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH3,
			PARAM_DeviceParam.channel[2].alarmValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH3,
			PARAM_DeviceParam.channel[2].alarmValueLow);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH3,
			PARAM_DeviceParam.channel[2].perwarningValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH3,
			PARAM_DeviceParam.channel[2].perwarningValueLow);

	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH4,
			PARAM_DeviceParam.channel[3].alarmValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH4,
			PARAM_DeviceParam.channel[3].alarmValueLow);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH4,
			PARAM_DeviceParam.channel[3].perwarningValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH4,
			PARAM_DeviceParam.channel[3].perwarningValueLow);

	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH5,
			PARAM_DeviceParam.channel[4].alarmValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH5,
			PARAM_DeviceParam.channel[4].alarmValueLow);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH5,
			PARAM_DeviceParam.channel[4].perwarningValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH5,
			PARAM_DeviceParam.channel[4].perwarningValueLow);

	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH6,
			PARAM_DeviceParam.channel[5].alarmValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH6,
			PARAM_DeviceParam.channel[5].alarmValueLow);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH6,
			PARAM_DeviceParam.channel[5].perwarningValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH6,
			PARAM_DeviceParam.channel[5].perwarningValueLow);

	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH7,
			PARAM_DeviceParam.channel[6].alarmValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH7,
			PARAM_DeviceParam.channel[6].alarmValueLow);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH7,
			PARAM_DeviceParam.channel[6].perwarningValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH7,
			PARAM_DeviceParam.channel[6].perwarningValueLow);

	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH8,
			PARAM_DeviceParam.channel[7].alarmValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH8,
			PARAM_DeviceParam.channel[7].alarmValueLow);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH8,
			PARAM_DeviceParam.channel[7].perwarningValueUp);
	PARAM_AlarmLimitStructUpdate(SCREEN_ID_SET_ALARM_LIMIT_2,
			CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH8,
			PARAM_DeviceParam.channel[7].perwarningValueLow);

	TFTLCD_TextValueUpdate(SCREEN_ID_SET_ALARM_CODE, CTL_ID_SET_ALARM_CODE_1,
			(char*)&PARAM_DeviceParam.alarmCode[0], 11);
	TFTLCD_TextValueUpdate(SCREEN_ID_SET_ALARM_CODE, CTL_ID_SET_ALARM_CODE_2,
			(char*)&PARAM_DeviceParam.alarmCode[1], 11);
	TFTLCD_TextValueUpdate(SCREEN_ID_SET_ALARM_CODE, CTL_ID_SET_ALARM_CODE_3,
			(char*)&PARAM_DeviceParam.alarmCode[2], 11);
}

/*******************************************************************************
 * function：保存参数结构体
 */
void PARAM_SaveStruct(PARAM_DeviceParamTypedef* param)
{
	FILE_WriteFile(FILE_NAME_PARAM, 0, (BYTE*)param, sizeof(PARAM_DeviceParamTypedef));
}

/*******************************************************************************
 * function：读取参数结构体
 */
void PARAM_ReadStruct(PARAM_DeviceParamTypedef* param)
{
	FILE_ReadFile(FILE_NAME_PARAM, 0, (BYTE*)param, sizeof(PARAM_DeviceParamTypedef));
}

/*******************************************************************************
 *
 */
void PARAM_AlarmLimitStructUpdate(uint16_t screenID, uint16_t ctlID, float data)
{
	char str[5];

	sprintf(str, "%5.1f", data);

	TFTLCD_TextValueUpdate(screenID, ctlID, str, 5);
}
