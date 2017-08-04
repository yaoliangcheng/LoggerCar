#ifndef __TFTLCD_H
#define __TFTLCD_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "common.h"
#include "analog.h"
#include "rt.h"
#include "file.h"

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
#define TFTLCD_CMD_BATCH_UPDATE				(uint16_t)(0x12B1)
#define TFTLCD_CMD_TEXT_UPDATE				(uint16_t)(0x10B1)		/* 更新文本 */
#define TFTLCD_CMD_SET_SCREEN				(uint16_t)(0X00B1)
#define TFTLCD_CMD_BUTTON					(uint16_t)(0x11B1)
#define TFTLCD_CMD_BUTTON_SELECT			(uint16_t)(0x11B1)		/* 广州大彩，选择控件和按钮控件是同一个cmd */
#define TFTLCD_CMD_SELECT					(uint16_t)(0x14B1)		/* 选择控件值上传 */



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
/* 画面编号 */
typedef enum
{
	SCREEN_ID_START,							/* 开机界面 */
	SCREEN_ID_MENU,								/* 主界面 */
	SCREEN_ID_CUR_DATA,							/* 当前数据 */
	SCREEN_ID_HIS_DATA,							/* 历史数据 */
	SCREEN_ID_DATA_EXPORT,						/* 数据导出 */
	SCREEN_ID_PRINT,							/* 打印界面 */
	SCREEN_ID_ABOUT,							/* 关于 */
	SCREEN_ID_ADD_DEVICES,						/* 添加设备 */
	SCREEN_ID_SETTING,							/* 密码 */
	SCREEN_ID_SETTING_MENU,						/* 设置菜单 */
	SCREEN_ID_PRINT_SETTING,					/* 打印设置 */
	SCREEN_ID_SETTING_ALARM,					/* 温湿度上下限报警设置 */
	SCREEN_ID_SETTING_ALARM_CODE,				/* 报警号码设置 */
 	SCREEN_ID_UPDATE,							/* 系统更新 */
 	SCREEN_ID_CURVE,							/* 曲线 */
 	SCREEN_ID_PRINT_TIME_SELECT,				/* 打印时间选择 */
} TFTLCD_ScreenIDEnum;

/******************************************************************************/
/* 打印界面控件编号 */
typedef enum
{
	PRINT_CTRL_ID_BACK = 1,
	PRINT_CTRL_ID_SET,
	PRINT_CTRL_ID_START_TIME,
	PRINT_CTRL_ID_END_TIME,
	PRINT_CTRL_ID_START_TIME_BUTTON,
	PRINT_CTRL_ID_END_TIME_BUTTON,
	PRINT_CTRL_ID_START_PRINT,
	PRINT_CTRL_ID_CHANNEL_1,
	PRINT_CTRL_ID_CHANNEL_2,
}CtrlID_PrintEnum;

/* 打印时间选择界面控件编号 */
typedef enum
{
	TIME_SELECT_CTRL_ID_YEAR,
	TIME_SELECT_CTRL_ID_MONTH,
	TIME_SELECT_CTRL_ID_DAY,
	TIME_SELECT_CTRL_ID_HOUR,
	TIME_SELECT_CTRL_ID_MIN,
	TIME_SELECT_CTRL_ID_CANCEL,
	TIME_SELECT_CTRL_ID_OK
} CtrlID_TimeSelectEnum;



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

/******************************************************************************/
/* 接收设置 */
typedef struct
{
	uint8_t head;												/* 帧头 */
	uint16_t cmd;												/* 指令 */
	uint8_t screenIdH;											/* 画面ID */
	uint8_t screenIdL;
	uint8_t ctrlIDH;
	uint8_t ctrlIDL;
	uint8_t buf[TFTLCD_UART_RX_DATA_SIZE_MAX];
} RecvDetail;

typedef struct
{
	union
	{
		RecvDetail recvBuf;
		uint8_t buf[TFTLCD_UART_RX_DATA_SIZE_MAX];
	} date;
	uint8_t bufferSize;											/* 缓存大小 */
} TFTLCD_RecvBufferTypedef;

#pragma pack(pop)

/******************************************************************************/
extern TFTLCD_RecvBufferTypedef TFTLCD_RecvBuffer;

/******************************************************************************/
void TFTLCD_Init(void);
void TFTLCD_AnalogDataRefresh(ANALOG_ValueTypedef* analog);
void TFTLCD_RealtimeRefresh(RT_TimeTypedef* rt);
void TFTLCD_UartIdleDeal(void);
ErrorStatus TFTLCD_CheckHeadTail(void);
void TFTLCD_printTimeUpdate(FILE_RealTime* rt, CtrlID_PrintEnum ctrl);




#endif
