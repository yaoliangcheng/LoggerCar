#ifndef __TFTLCD_H
#define __TFTLCD_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "common.h"
#include "analog.h"
#include "rt.h"

#define TFTLCD_UART					(huart4)

#define TFTLCD_PWR_ENABLE() \
		HAL_GPIO_WritePin(SCREEN_PWR_CTRL_GPIO_Port, SCREEN_PWR_CTRL_Pin, GPIO_PIN_RESET);
#define TFTLCD_PWR_DISABLE() \
		HAL_GPIO_WritePin(SCREEN_PWR_CTRL_GPIO_Port, SCREEN_PWR_CTRL_Pin, GPIO_PIN_SET);

#define TFTLCD_UART_DMA_FLAG_GL		(DMA_FLAG_GL3)

/*******************************************************************************
 * 指令的帧头和帧尾
 */
#define TFTLCD_CMD_HEAD								(0XEE)
#define TFTLCD_CMD_TAIL1							(0XFF)
#define TFTLCD_CMD_TAIL2							(0XFC)
#define TFTLCD_CMD_TAIL3							(0XFF)
#define TFTLCD_CMD_TAIL4							(0XFF)

#define TFTLCD_UART_RX_DATA_SIZE_MAX				(50)

/*******************************************************************************
 * 指令格式
 */
#define TFTLCD_CMD_BATCH_UPDATE				(0x12B1)
#define TFTLCD_CMD_TIME_UPDATE				(0x10B1)

/* 屏幕ID */
#define SCREEN_ID_START						(uint16_t)(0)		/* 开机界面 */
#define SCREEN_ID_MENU						(uint16_t)(1)		/* 主界面 */
#define SCREEN_ID_CUR_DATA					(uint16_t)(2)		/* 当前数据 */
#define SCREEN_ID_HIS_DATA					(uint16_t)(3)		/* 历史数据 */
#define SCREEN_ID_DATA_EXPORT				(uint16_t)(4)		/* 数据导出 */
#define SCREEN_ID_PRINT						(uint16_t)(5)		/* 打印界面 */
#define SCREEN_ID_ABOUT						(uint16_t)(6)		/* 关于 */
#define SCREEN_ID_ADD_DEVICES				(uint16_t)(7)		/* 添加设备 */
#define SCREEN_ID_SETTING					(uint16_t)(8)		/* 密码 */
#define SCREEN_ID_SETTING_MENU				(uint16_t)(9)		/* 设置菜单 */
#define SCREEN_ID_PRINT_SETTING				(uint16_t)(10)	/* 打印设置 */
#define SCREEN_ID_SETTING_ALARM				(uint16_t)(11)	/* 温湿度上下限报警设置 */
#define SCREEN_ID_SETTING_ALARM_CODE		(uint16_t)(12)	/* 报警号码设置 */
#define SCREEN_ID_UPDATE					(uint16_t)(13)	/* 系统更新 */
#define SCREEN_ID_CURVE						(uint16_t)(14)	/* 曲线 */
#define SCREEN_ID_PRINT_TIME_SELECT			(uint16_t)(15)	/* 打印时间选择 */

/* 控件ID */
#define CTRL_TYPE_ID_TEMP1					(uint16_t)(1)		/* 温度1 */
#define CTRL_TYPE_ID_TEMP2					(uint16_t)(2)		/* 温度2 */
#define CTRL_TYPE_ID_TEMP3					(uint16_t)(3)		/* 温度3 */
#define CTRL_TYPE_ID_TEMP4					(uint16_t)(4)		/* 温度4 */
#define CTRL_TYPE_ID_HUMI1					(uint16_t)(5)		/* 湿度1 */
#define CTRL_TYPE_ID_HUMI2					(uint16_t)(6)		/* 湿度2 */
#define CTRL_TYPE_ID_HUMI3					(uint16_t)(7)		/* 湿度3 */
#define CTRL_TYPE_ID_HUMI4					(uint16_t)(8)		/* 湿度4 */
#define CTRL_TYPE_ID_REAL_TIME				(uint16_t)(9)		/* 实时时钟 */


/******************************************************************************/
#pragma pack(push)
#pragma pack(1)											/* 按字节对齐 */

typedef struct
{
	uint8_t ctrlIdH;											/* 画面ID */
	uint8_t ctrlIdL;
	uint8_t sizeH;
	uint8_t sizeL;
	char value[5];				/* 批量更新最大传4个字节 */
} BatchUpdateTypedef;

typedef struct
{
	char year[4];
	char str1;
	char month[2];
	char str2;
	char day[2];
	char str3;
	char hour[2];
	char str4;
	char min[2];
	char str5;
	char sec[2];
} TFTLCD_TimeUpdateTypedef;

typedef struct
{
	uint8_t ctrlIdH;											/* 画面ID */
	uint8_t ctrlIdL;
	union
	{
		char date[TFTLCD_UART_RX_DATA_SIZE_MAX];
		TFTLCD_TimeUpdateTypedef time;
	}value;

} UpdateTypedef;

typedef struct
{
	uint8_t head;												/* 帧头 */
	uint16_t cmd;												/* 指令 */
	uint8_t screenIdH;											/* 画面ID */
	uint8_t screenIdL;
	union
	{
		BatchUpdateTypedef batchDate[8];		/* 批量更新内容 */
		UpdateTypedef data;						/* buffer */
	}buf;
	uint8_t tail[4];											/* 帧尾 */
} TFTLCD_SendBufferTypedef;

typedef struct
{
	uint8_t recvBuffer[TFTLCD_UART_RX_DATA_SIZE_MAX];			/* 接收缓存 */
	uint8_t bufferSize;											/* 缓存大小 */
} TFTLCD_BufferStatusTypedef;

/***********************批量更新控件数值********************************************/
#define BATCH_UPDATE_CONTROL_MAX		(20)	/* 最大支持批量更改控件数 */
#define BATCH_UPDATE_DATA_MAX			(5)		/* 单个控件数据最大字节数 */
typedef struct
{
	uint8_t controlIdH;								/* 控件ID */
	uint8_t controlIdL;
	uint8_t sizeH;									/* 更新控件数值长度 */
	uint8_t sizeL;
	char strData[BATCH_UPDATE_DATA_MAX];		/* 更新数值 */
} BatchUpdataData;

typedef struct
{
	uint8_t cmdH;									/* 指令高 */
	uint8_t cmdL;									/* 指令低 */
	uint8_t screenIdH;								/* 画面ID */
	uint8_t screenIdL;
	BatchUpdataData updateData[BATCH_UPDATE_CONTROL_MAX];
												/* 控件更新的数值 */
} TFTLCD_BatchUpdateStructTypedef;

/***********************RealTime Struct****************************************/
typedef struct
{
	uint8_t cmdH;									/* 指令高 */
	uint8_t cmdL;									/* 指令低 */
	uint8_t screenIdH;								/* 画面ID */
	uint8_t screenIdL;
	uint8_t controlIdH;								/* 控件ID */
	uint8_t controlIdL;

	uint32_t year;
	char symbol1;
	uint16_t month;
	char symbol2;
	uint16_t day;
	char symbol3;
	uint16_t hour;
	char symbol4;
	uint16_t min;
	char symbol5;
	uint16_t sec;
} TFTLCD_RealTimeUpdateTypedef;

#pragma pack(pop)

/******************************************************************************/
void TFTLCD_Init(void);
void TFTLCD_AnalogDataRefresh(ANALOG_ValueTypedef* analog);
void TFTLCD_RealtimeRefresh(RT_TimeTypedef* rt);
void TFTLCD_UartIdleDeal(void);






#endif
