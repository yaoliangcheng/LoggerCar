#ifndef __DISPLAY_H
#define __DISPLAY_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "file.h"

/******************************************************************************/
#define DISPLAY_HIS_DATA_ONE_SCREEN_CNT				(4)		/* 历史数据一个界面显示4行 */
#define DISPLAY_HIS_DATA_READ_ONCE_CNT				(5)		/* 历史曲线界面，一次读取5组数据 */
#define DISPLAY_HIS_DATA_READ_CNT					(18)	/* 一个界面90个数据，读取16次可完成 */
#define DISPLAY_HIS_DATA_CURVE_CNT					(DISPLAY_HIS_DATA_READ_ONCE_CNT * DISPLAY_HIS_DATA_READ_CNT)

/******************************************************************************/
typedef enum
{
	TIME_SELECT_HIS_DATA_START,
	TIME_SELECT_HIS_DATA_END,
	TIME_SELECT_START_PRINT_TIME,
	TIME_SELECT_END_PRINT_TIME,
} TimeSelectEnum;

/******************************************************************************/
typedef enum
{
	PRINT_MODE_INTEGRATED,						/* 一体式 */
	PRINT_MODE_BLE_UNLINK,						/* 蓝牙打印机未连接 */
	PRINT_MODE_BLE_LINK,						/* 蓝牙打印机已连接 */
} PrintModeEnum;

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)

/******************************************************************************/
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
} ChannelSelectTypedef;

typedef struct
{
	char year[2];								/* 日期：年月日 时分秒，（16） */
	char month[2];
	char day[2];
	char str1;
	char hour[2];
	char str2;
	char min[2];
} DISPLAY_CompareTimeTypedef;

typedef struct
{
	uint32_t hisDataDispStructOffset;			/* 历史数据显示结构体偏移 */
	FunctionalState      displayModeBusy;		/* 显示状态忙，重复操作无效 */
	ChannelSelectTypedef printChannelStatus;	/* 打印界面通道状态 */
//	TimeSelectEnum timeSelectStatus;			/* 时间选择状态，标记是由哪个界面进入选择 */
	DISPLAY_CompareTimeTypedef* selectTime;				/* 已被选择的时间 */
	DISPLAY_CompareTimeTypedef  printTimeStart;			/* 开始打印时间 */
	DISPLAY_CompareTimeTypedef  printTimeEnd;			/* 结束打印时间 */

	char    passwordBuffer[4];					/* 密码缓存 */
	char    passwordBufferNew[4];
	char    passwordBufferNewAgain[4];
	uint8_t passwordBufferIndex;				/* 密码位指示 */

	PrintModeEnum printMode;					/* 打印模式 */
} DISPLAY_StatusTypedef;


#pragma pack(pop)

/******************************************************************************/
extern DISPLAY_StatusTypedef DISPLAY_Status;

/******************************************************************************/
void DISPLAY_Init(void);
void DISPLAY_HistoryData(uint32_t startStructOffset, uint8_t structCnt);
void DISPLAY_HistoryTouch(uint16_t typeID);
void DISPLAY_HistoryDataCurve(uint32_t startStructOffset);
void DISPLAY_TimeSelect(RT_TimeTypedef* time);
void DISPLAY_PrintTouch(uint16_t typeID);
void DISPLAY_TimeSelectTouch(uint16_t typeID, uint8_t value);
void DISPLAY_SetPasswordTouch(uint16_t typeID);
void DISPLAY_SetAlarmLimitTouch(uint16_t typeID);
void DISPLAY_SetAlarmLimit2Touch(uint16_t typeID);
void DISPLAY_SetMessageTouch(uint16_t typeID);
void DISPLAY_SetPasswordChangeTouch(uint16_t typeID);


#endif
