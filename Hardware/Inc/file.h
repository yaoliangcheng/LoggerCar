#ifndef __FILE_H
#define __FILE_H


/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "common.h"
#include "TFTLCDProcess.h"


#include "rt.h"
#include "analog.h"
#include "gprs.h"
#include "gps.h"
#include "fatfs.h"

/******************************************************************************/
#define ANALOG_VALUE_FORMAT						(FORMAT_ONE_DECIMAL)
#define FILE_NAME_SAVE_DATA						("SaveData.csv")
#define FILE_NAME_PATCH_PACK					("patch.txt")
#define FILE_NAME_PARAM							("param.txt")

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
	char value[5];
	char str;
} SaveInfoAnalogTypedef;

typedef struct
{
	char year[2];								/* 日期：年月日 时分秒，（14） */
	char month[2];
	char day[2];
	char str1;
	char hour[2];
	char min[2];
	char sec[2];
	char str2;
	char batQuality[3];							/* 电池电量，（4） */
	char str3;
	FunctionalState exPwrStatus;				/* 电池状态，（2） */
	char str4;
	char longitude[10];							/* 经度，（11） */
	char str5;
	char latitude[10];							/* 纬度，（11） */
	char str6;
	SaveInfoAnalogTypedef analogValue[8];		/* 模拟量，（6 x 8） */
	char end[2];								/* 换行（2） */
} FILE_SaveInfoTypedef;							/* 总共（92字节） */

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
void FILE_SaveInfoSymbolInit(FILE_SaveInfoTypedef* info);
ErrorStatus FILE_SaveInfo(FILE_SaveInfoTypedef* saveInfo, uint64_t* fileStructCount);
ErrorStatus FILE_ReadInfo(FILE_SaveInfoTypedef* readInfo);

void FILE_SendInfoFormatConvert(FILE_SaveInfoTypedef* saveInfo,
								GPRS_SendInfoTypedef*      sendInfo,
								uint8_t 		      sendPackNumb);

//uint16_t FILE_ReadPatchInfo(FILE_PatchPackTypedef* patch, FILE_InfoTypedef* readInfo);
ErrorStatus FILE_ReadPatchPackFile(FILE_PatchPackTypedef* pBuffer);
ErrorStatus FILE_WritePatchPackFile(FILE_PatchPackTypedef* pBuffer);
ErrorStatus FILE_ParamFileInit(void);
ErrorStatus FILE_PrintDependOnTime(FILE_RealTime* startTime, FILE_RealTime* stopTime,
		PRINT_ChannelSelectTypedef* select);

#endif
