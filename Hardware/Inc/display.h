#ifndef __DISPLAY_H
#define __DISPLAY_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

/******************************************************************************/
#define DISPLAY_HIS_DATA_ONE_SCREEN_CNT				(4)		/* 历史数据一个界面显示4行 */

/******************************************************************************/
typedef enum
{
	TIME_SELECT_HIS_DATA_START,
	TIME_SELECT_HIS_DATA_END,
	TIME_SELECT_START_PRINT_TIME,
	TIME_SELECT_END_PRINT_TIME,
} TimeSelectEnum;

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
	uint16_t hisDataDispStructOffset;			/* 历史数据显示结构体偏移 */
	ChannelSelectTypedef printChannelStatus;	/* 打印界面通道状态 */
	TimeSelectEnum timeSelectStatus;			/* 时间选择状态，标记是由哪个界面进入选择 */
	FILE_RealTimeTypedef* selectTime;			/* 已被选择的时间 */
	FILE_RealTimeTypedef printTimeStart;		/* 开始打印时间 */
	FILE_RealTimeTypedef printTimeEnd;			/* 结束打印时间 */
} DISPLAY_StatusTypedef;


#pragma pack(pop)

/******************************************************************************/
extern DISPLAY_StatusTypedef DISPLAY_Status;

/******************************************************************************/
void DISPLAY_HistoryData(uint16_t offsetStruct);
void DISPLAY_HistoryTouch(uint16_t typeID);
void DISPLAY_PrintTouch(uint16_t typeID);




#endif
