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
void PARAM_Init(void)
{
	PARAM_ReadStruct(&PARAM_DeviceParam);

	/* 判断SN号是否正常 */
	/* todo */

//	memcpy(PARAM_DeviceParam.deviceSN, "1708151515", sizeof(PARAM_DeviceParam.deviceSN));
//	PARAM_DeviceParam.locationType    = LOCATION_GPS;
//	PARAM_DeviceParam.firmwareVersion = 10;
//	PARAM_DeviceParam.recordInterval  = 1;
//	PARAM_DeviceParam.overLimitRecordInterval = 1;
//	PARAM_DeviceParam.exAnalogChannelNumb  = ANALOG_CHANNEL_NUMB;
//	PARAM_DeviceParam.chParam[0].channelType = TYPE_TEMP;
//	PARAM_DeviceParam.chParam[0].channelUnit = UNIT_TEMP;
//	PARAM_DeviceParam.chParam[0].dataFormat  = FORMAT_ONE_DECIMAL;
//	PARAM_DeviceParam.chParam[1].channelType = TYPE_HUMI;
//	PARAM_DeviceParam.chParam[1].channelUnit = UNIT_HUMI;
//	PARAM_DeviceParam.chParam[1].dataFormat  = FORMAT_ONE_DECIMAL;
//	PARAM_DeviceParam.chParam[2].channelType = TYPE_TEMP;
//	PARAM_DeviceParam.chParam[2].channelUnit = UNIT_TEMP;
//	PARAM_DeviceParam.chParam[2].dataFormat  = FORMAT_ONE_DECIMAL;
//	PARAM_DeviceParam.chParam[3].channelType = TYPE_HUMI;
//	PARAM_DeviceParam.chParam[3].channelUnit = UNIT_HUMI;
//	PARAM_DeviceParam.chParam[3].dataFormat  = FORMAT_ONE_DECIMAL;
//	PARAM_DeviceParam.chParam[4].channelType = TYPE_TEMP;
//	PARAM_DeviceParam.chParam[4].channelUnit = UNIT_TEMP;
//	PARAM_DeviceParam.chParam[4].dataFormat  = FORMAT_ONE_DECIMAL;
//	PARAM_DeviceParam.chParam[5].channelType = TYPE_HUMI;
//	PARAM_DeviceParam.chParam[5].channelUnit = UNIT_HUMI;
//	PARAM_DeviceParam.chParam[5].dataFormat  = FORMAT_ONE_DECIMAL;
//	PARAM_DeviceParam.chParam[6].channelType = TYPE_TEMP;
//	PARAM_DeviceParam.chParam[6].channelUnit = UNIT_TEMP;
//	PARAM_DeviceParam.chParam[6].dataFormat  = FORMAT_ONE_DECIMAL;
//	PARAM_DeviceParam.chParam[7].channelType = TYPE_HUMI;
//	PARAM_DeviceParam.chParam[7].channelUnit = UNIT_HUMI;
//	PARAM_DeviceParam.chParam[7].dataFormat  = FORMAT_ONE_DECIMAL;
//
//	PARAM_DeviceParam.chAlarmValue[0].alarmValueUp       = 24.0;
//	PARAM_DeviceParam.chAlarmValue[0].alarmValueLow      = 20.0;
//	PARAM_DeviceParam.chAlarmValue[0].perwarningValueUp  = 23.0;
//	PARAM_DeviceParam.chAlarmValue[0].perwarningValueLow = 21.0;
//
//	PARAM_DeviceParam.chAlarmValue[1].alarmValueUp       = 74.0;
//	PARAM_DeviceParam.chAlarmValue[1].alarmValueLow      = 70.0;
//	PARAM_DeviceParam.chAlarmValue[1].perwarningValueUp  = 73.0;
//	PARAM_DeviceParam.chAlarmValue[1].perwarningValueLow = 71.0;
//
//	PARAM_DeviceParam.chAlarmValue[2].alarmValueUp       = 24.0;
//	PARAM_DeviceParam.chAlarmValue[2].alarmValueLow      = 20.0;
//	PARAM_DeviceParam.chAlarmValue[2].perwarningValueUp  = 23.0;
//	PARAM_DeviceParam.chAlarmValue[2].perwarningValueLow = 21.0;
//
//	PARAM_DeviceParam.chAlarmValue[3].alarmValueUp       = 74.0;
//	PARAM_DeviceParam.chAlarmValue[3].alarmValueLow      = 70.0;
//	PARAM_DeviceParam.chAlarmValue[3].perwarningValueUp  = 73.0;
//	PARAM_DeviceParam.chAlarmValue[3].perwarningValueLow = 71.0;
//
//	PARAM_DeviceParam.chAlarmValue[4].alarmValueUp       = 24.0;
//	PARAM_DeviceParam.chAlarmValue[4].alarmValueLow      = 20.0;
//	PARAM_DeviceParam.chAlarmValue[4].perwarningValueUp  = 23.0;
//	PARAM_DeviceParam.chAlarmValue[4].perwarningValueLow = 21.0;
//
//	PARAM_DeviceParam.chAlarmValue[5].alarmValueUp       = 74.0;
//	PARAM_DeviceParam.chAlarmValue[5].alarmValueLow      = 70.0;
//	PARAM_DeviceParam.chAlarmValue[5].perwarningValueUp  = 73.0;
//	PARAM_DeviceParam.chAlarmValue[5].perwarningValueLow = 71.0;
//
//	PARAM_DeviceParam.chAlarmValue[6].alarmValueUp       = 24.0;
//	PARAM_DeviceParam.chAlarmValue[6].alarmValueLow      = 20.0;
//	PARAM_DeviceParam.chAlarmValue[6].perwarningValueUp  = 23.0;
//	PARAM_DeviceParam.chAlarmValue[6].perwarningValueLow = 21.0;
//
//	PARAM_DeviceParam.chAlarmValue[7].alarmValueUp       = 74.0;
//	PARAM_DeviceParam.chAlarmValue[7].alarmValueLow      = 70.0;
//	PARAM_DeviceParam.chAlarmValue[7].perwarningValueUp  = 73.0;
//	PARAM_DeviceParam.chAlarmValue[7].perwarningValueLow = 71.0;
//
//	memcpy(PARAM_DeviceParam.password, "0000", 4);
//
//	PARAM_SaveStruct(&PARAM_DeviceParam);

	/* 通道1参数更新 */

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
