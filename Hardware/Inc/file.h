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
#include "eeprom.h"
#include "fatfs.h"

/******************************************************************************/
#define ANALOG_VALUE_FORMAT						(FORMAT_ONE_DECIMAL)
#define PATCH_PACK_FILE_NAME					("patch.txt")

#define FILE_PRINT_TO_END						(0xFFFF)

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

#pragma pack(pop)

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
ErrorStatus FILE_PrintDependOnTime(FILE_RealTime* startTime, FILE_RealTime* stopTime,
		PRINT_ChannelSelectTypedef* select);

#endif
