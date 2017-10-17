#ifndef __TFTLCD_H
#define __TFTLCD_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "analog.h"
#include "rt.h"
#include "file.h"
#include "public.h"

#define TFTLCD_UART					(huart2)

#define TFTLCD_PWR_ENABLE() \
		HAL_GPIO_WritePin(SCREEN_PWR_CTRL_GPIO_Port, SCREEN_PWR_CTRL_Pin, GPIO_PIN_RESET);
#define TFTLCD_PWR_DISABLE() \
		HAL_GPIO_WritePin(SCREEN_PWR_CTRL_GPIO_Port, SCREEN_PWR_CTRL_Pin, GPIO_PIN_SET);

#define TFTLCD_UART_DMA_FLAG_GL		(DMA_FLAG_GL6)

/*******************************************************************************
 * 指令的帧头和帧尾
 */
#define TFTLCD_CMD_HEAD								(0XEE)
#define TFTLCD_CMD_TAIL1							(0XFF)
#define TFTLCD_CMD_TAIL2							(0XFC)
#define TFTLCD_CMD_TAIL3							(0XFF)
#define TFTLCD_CMD_TAIL4							(0XFF)

#define TFTLCD_UART_RX_DATA_SIZE_MAX				(100)

/*******************************************************************************
 * 指令格式
 */
#define TFTLCD_CMD_SCREEN_ID_CHANGE			(uint16_t)(0x00B1)		/* 切换画面 */
#define TFTLCD_CMD_SET_SCREEN				(uint16_t)(0X00B1)
#define TFTLCD_CMD_SCREEN_ID_GET			(uint16_t)(0x01B1)		/* 获取画面 */
#define TFTLCD_CMD_TEXT_UPDATE				(uint16_t)(0x10B1)		/* 更新文本 */
#define TFTLCD_CMD_BUTTON					(uint16_t)(0x11B1)
#define TFTLCD_CMD_BUTTON_SELECT			(uint16_t)(0x11B1)		/* 广州大彩，选择控件和按钮控件是同一个cmd */
#define TFTLCD_CMD_BATCH_UPDATE				(uint16_t)(0x12B1)
#define TFTLCD_CMD_SELECT					(uint16_t)(0x14B1)		/* 选择控件值上传 */
#define TFTLCD_CMD_SET_FORE_COLOR			(uint16_t)(0x19B1)		/* 设置文本控件前景色 */
#define TFTLCD_CMD_ICON_DISP				(uint16_t)(0x23B1)		/* 图标控件显示 */

#define TFTLCD_CMD_CURVE_ADD_CHANNEL		(uint16_t)(0x30B1)		/* 曲线添加数据通道 */
#define TFTLCD_CMD_CURVE_DELETE_CHANNEL		(uint16_t)(0x31B1)		/* 曲线删除数据通道 */
#define TFTLCD_CMD_CURVE_SET_PARAM  		(uint16_t)(0x34B1)		/* 曲线设置垂直水平的缩放、平移 */
#define TFTLCD_CMD_CURVE_ADD_DATA_TAIL		(uint16_t)(0x32B1)		/* 指定通道末端添加新数据 */
#define TFTLCD_CMD_CURVE_ADD_DATA_FRONT		(uint16_t)(0x35B1)		/* 指定通道前端添加新数据 */
#define TFTLCD_CMD_CURVE_CLEAR_DATA			(uint16_t)(0x33B1)		/* 清空指定通道的数据 */

#define TFTLCD_ALARM_COLOR					(0xF800)				/* 报警颜色 */
#define TFTLCD_PERWARM_COLOR				(0xFF10)				/* 预警颜色 */
/****************************控件ID枚举******************************************/
/* 状态栏公共控件ID */
typedef enum
{
	CTL_ID_REALTIME,
	CTL_ID_BAT_QUANTITY,
	CTL_ID_BAT_QUANTITY_PERCENT,
	CTL_ID_SIGNAL_QUALITY,
	CTL_ID_ALARM_ICON
} TFTLCD_CommonCtlIdEnum;

/* 实时数据界面控件ID */
typedef enum
{
	CTL_ID_DATA_CH1 = 6,
	CTL_ID_DATA_CH2,
	CTL_ID_DATA_CH3,
	CTL_ID_DATA_CH4,
	CTL_ID_DATA_CH5,
	CTL_ID_DATA_CH6,
	CTL_ID_DATA_CH7,
	CTL_ID_DATA_CH8,
	CTL_ID_DATA_CH9,
	CTL_ID_DATA_CH10,
	CTL_ID_DATA_CH11,
	CTL_ID_DATA_CH12,
	CTL_ID_DATA_CH13,
	CTL_ID_DATA_CH14,
} TFTLCD_DataCtlIdEnum;

/* 历史数据界面控件ID */
typedef enum
{
	CTL_ID_DIS_DATA_1 = 6,
	CTL_ID_DIS_DATA_2,
	CTL_ID_DIS_DATA_3,
	CTL_ID_DIS_DATA_4,
	CTL_ID_PAGE_UP,
	CTL_ID_PAGE_DOWN,
	CTL_ID_DIS_CURVE,
} TFTLCD_HisDataCtlIdEnum;

/* 历史曲线界面控件ID */
typedef enum
{
	CTL_ID_HIS_DATA_CURVE = 6,
} TFTLCD_HisDataCurveCtlIdEnum;

/* 数据打印界面控件ID */
typedef enum
{
	CTL_ID_PRINT_TIME_START_TEXT = 6,
	CTL_ID_PRINT_TIME_END_TEXT,
	CTL_ID_CHANNAL_SELECT_CH1_ICON,
	CTL_ID_CHANNAL_SELECT_CH2_ICON,
	CTL_ID_CHANNAL_SELECT_CH3_ICON,
	CTL_ID_CHANNAL_SELECT_CH4_ICON,
	CTL_ID_CHANNAL_SELECT_CH5_ICON,
	CTL_ID_CHANNAL_SELECT_CH6_ICON,
	CTL_ID_CHANNAL_SELECT_CH7_ICON,
	CTL_ID_CHANNAL_SELECT_CH8_ICON,
	CTL_ID_PRINT_TIME_START_TOUCH,
	CTL_ID_PRINT_TIME_END_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH1_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH2_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH3_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH4_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH5_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH6_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH7_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH8_TOUCH,
	CTL_ID_PRINT_DEFAULT,								/* 默认打印 */
	CTL_ID_PRINT_CUSTOM,								/* 自定义打印 */
} TFTLCD_PrintDataCtlIdEnum;

/* 时间选择界面控件ID */
typedef enum
{
	CTL_ID_TIME_SELECT_YEAR,
	CTL_ID_TIME_SELECT_MONTH,
	CTL_ID_TIME_SELECT_DAY,
	CTL_ID_TIME_SELECT_HOUR,
	CTL_ID_TIME_SELECT_MIN,
	CTL_ID_TIME_SELECT_CANCEL,
	CTL_ID_TIME_SELECT_OK,
} TFTLCD_TimeSelectCtlIdEnum;

/* 密码设置界面控件ID */
typedef enum
{
	CTL_ID_SET_PASSWORD_TEXT = 6,
	CTL_ID_SET_PASSWORD_NUMB_1,
	CTL_ID_SET_PASSWORD_NUMB_2,
	CTL_ID_SET_PASSWORD_NUMB_3,
	CTL_ID_SET_PASSWORD_NUMB_4,
	CTL_ID_SET_PASSWORD_NUMB_5,
	CTL_ID_SET_PASSWORD_NUMB_6,
	CTL_ID_SET_PASSWORD_NUMB_7,
	CTL_ID_SET_PASSWORD_NUMB_8,
	CTL_ID_SET_PASSWORD_NUMB_9,
	CTL_ID_SET_PASSWORD_CLEAR,
	CTL_ID_SET_PASSWORD_NUMB_0,
	CTL_ID_SET_PASSWORD_ENTER,
} TFTLCD_SetPasswordCtlIdEnum;

/* 报警上下限界面控件ID */
typedef enum
{
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH1 = 6,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH1,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH1,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH1,
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH2,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH2,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH2,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH2,
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH3,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH3,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH3,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH3,
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH4,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH4,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH4,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH4,
	CTL_ID_SET_ALARM_LIMIT_SAVE,
} TFTLCD_SetAlarmLimitCtlIdEnum;

/* 报警上下限界面2控件ID */
typedef enum
{
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH5 = 6,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH5,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH5,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH5,
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH6,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH6,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH6,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH6,
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH7,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH7,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH7,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH7,
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH8,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH8,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH8,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH8,
	CTL_ID_SET_ALARM_LIMIT_2_SAVE
} TFTLCD_SetAlarmLimit2CtlIdEnum;

/* 报警号码界面控件ID */
typedef enum
{
	CTL_ID_SET_ALARM_CODE_1 = 6,
	CTL_ID_SET_ALARM_CODE_2,
	CTL_ID_SET_ALARM_CODE_3,
	CTL_ID_SET_ALARM_CODE_SAVE
} TFTLCD_SetAlarmCodeCtlIdEnum;

/* 密码修改界面控件ID */
typedef enum
{
	CTL_ID_SET_PASSWORD_CHANGE_OLD = 6,
	CTL_ID_SET_PASSWORD_CHANGE_NEW,
	CTL_ID_SET_PASSWORD_CHANGE_NEW_AGAIN,
	CTL_ID_SET_PASSWORD_CHANGE_SAVE,
} TFTLCD_SetPasswordChangeCtlIdEnum;

/* 本机信息界面控件ID */
typedef enum
{
	CTL_ID_ABOUT_DEVICE_SN,
	CTL_ID_ABOUT_DEVICE_TYPE,
	CTL_ID_ABOUT_CHANNEL_NUMB,
	CTL_ID_ABOUT_FIRM_VERSION,
	CTL_ID_ABOUT_OS_VERSION,
	CTL_ID_ABOUT_DEVICE_CAPACITY,
} TFTLCD_AboutCtlIdEnum;

/****************************画面ID枚举******************************************/
/* 画面编号 */
typedef enum
{
	SCREEN_ID_START,							/* 开机界面 */
	SCREEN_ID_CUR_DATA_2CH,						/* 实时数据 2通道 */
	SCREEN_ID_CUR_DATA_4CH,						/* 实时数据 4通道 */
	SCREEN_ID_CUR_DATA_8CH,						/* 实时数据 8通道 */
	SCREEN_ID_CUR_DATA_14CH,					/* 实时数据 14通道 */
	SCREEN_ID_HIS_DATA,							/* 历史数据 */
	SCREEN_ID_HIS_DATA_CURVE,					/* 历史数据 曲线 */
	SCREEN_ID_PRINT,							/* 数据打印 */
	SCREEN_ID_PRINT_DETAIL,						/* 数据打印——详细 */
	SCREEN_ID_DATA_EXPORT,						/* 数据导出 */
	SCREEN_ID_SET_PASSWORD,						/* 密码 */
	SCREEN_ID_SET_ALARM_LIMIT,					/* 报警上下限 */
	SCREEN_ID_SET_ALARM_LIMIT_2,				/* 报警上下限2 */
	SCREEN_ID_SET_ALARM_CODE,					/* 报警号码 */
	SCREEN_ID_SET_MESSAGE,						/* 短信签名 */
	SCREEN_ID_SET_UPDATE,						/* 固件升级 */
	SCREEN_ID_SET_CHANGE_PASSWORD,				/* 修改密码 */
	SCREEN_ID_ABOUT_DEVICE,						/* 关于设备 */
	SCREEN_ID_ABOUT_LUGE,						/* 关于路格 */
	SCREEN_ID_TIME_SELECT,
} TFTLCD_ScreenIDEnum;

/**************************小图标控件*********************************************/
typedef enum
{
	ICON_BAT_CHARGE,							/* 正在充电 */
	ICON_BAT_CAPACITY_80,						/* 电池电量>80% */
	ICON_BAT_CAPACITY_60,						/* 电池电量>60% */
	ICON_BAT_CAPACITY_40,						/* 电池电量>40% */
	ICON_BAT_CAPACITY_20,						/* 电池电量>20% */
	ICON_BAT_CAPACITY_0,						/* 电池电量>0% */
} IconBatCapacityEnum;

typedef enum
{
	ICON_SIGNAL_QUALITY_31_21,					/* 31>信号强度>21 */
	ICON_SIGNAL_QUALITY_21_11,					/* 21>信号强度>11 */
	ICON_SIGNAL_QUALITY_11_0,					/* 11>信号强度>0 */
} IconSignalQualityEnum;

typedef enum
{
	ICON_ALARM_ON,								/* 有报警信号 */
	ICON_ALARM_OFF,								/* 无报警信号 */
} IconAlarmStatusEnum;

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)											/* 按字节对齐 */

typedef struct
{
	uint8_t timeCtlIdH;
	uint8_t timeCtlIdL;
	uint8_t timeSizeH;
	uint8_t timeSizeL;
	char    year[4];
	char    str1;
	char    month[2];
	char    str2;
	char    day[2];
	char    str3;
	char    hour[2];
	char    str4;
	char    min[2];

	uint8_t batCtlIdH;
	uint8_t batCtlIdL;
	uint8_t batSizeH;
	uint8_t batSizeL;
	char    batCapacity[3];
} StatusBarTextTypedef;					/* 状态栏文本更新 */

typedef struct
{
	uint8_t batCtlIdH;					/* 电池电量图标 */
	uint8_t batCtlIdL;
	uint8_t batSizeH;
	uint8_t batSizeL;
	uint8_t batCapacityH;
	uint8_t batCapacityL;

	uint8_t signalCtlIdH;				/* 信号强度图标 */
	uint8_t signalCtlIdL;
	uint8_t signalSizeH;
	uint8_t signalSizeL;
	uint8_t signalCapacityH;
	uint8_t signalCapacityL;

	uint8_t alarmCtlIdH;				/* 报警图标 */
	uint8_t alarmCtlIdL;
	uint8_t alarmSizeH;
	uint8_t alarmSizeL;
	uint8_t alarmCapacityH;
	uint8_t alarmCapacityL;
} StatusBarIconTypedef;					/* 状态栏图标更新 */

typedef struct
{
	uint8_t ctrlIdH;
	uint8_t ctrlIdL;
	uint8_t sizeH;
	uint8_t sizeL;
	char value[5];							/* 模拟量数值5位数 */
} AnalogTypedef;							/* 模拟量值更新 */

typedef struct
{
	uint8_t ctrlIdH;
	uint8_t ctrlIdL;
	uint8_t sizeH;
	uint8_t sizeL;
	char value[5];
} HistoryDateTypedef;						/* 历史数据更新 */

typedef struct
{
	uint8_t ctrlIdH;											/* 画面ID */
	uint8_t ctrlIdL;
	union
	{
		char date[TFTLCD_UART_RX_DATA_SIZE_MAX];
//		TFTLCD_TimeUpdateTypedef time;
	}value;
} UpdateTypedef;

typedef struct
{
	uint8_t dataLengthH;					/* 数据长度 */
	uint8_t dataLengthL;
	uint8_t data;							/* 数据 */
} CurveDataTypedef;

typedef struct
{
	uint8_t ctlIdH;
	uint8_t ctlIdL;
	uint8_t channel;						/* 数据通道，最多支持8个 */
//	union
//	{
//		CurveDataTypedef curveData;
//		uint16_t         curveColor;
//
//	};
	uint8_t dataLengthH;					/* 数据长度 */
	uint8_t dataLengthL;
	uint8_t data;							/* 数据 */
} CurveTypedef;

typedef struct
{
	uint8_t head;									/* 帧头 */
	uint16_t cmd;									/* 指令 */
	uint8_t screenIdH;								/* 画面ID */
	uint8_t screenIdL;
	union
	{
		uint8_t data[TFTLCD_UART_RX_DATA_SIZE_MAX];
		StatusBarTextTypedef statusBarText;			/* 状态栏文本更新 */
		StatusBarIconTypedef statusBarIcon;			/* 状态栏图标更新 */
		AnalogTypedef        analogValue[8];		/* 模拟量批量更新 */
		HistoryDateTypedef   HistoryDate;			/* 历史数据批量更新 */
		UpdateTypedef 		 update;				/* 数据更新 */
		CurveTypedef  		 curve;					/* 曲线 */
	}buffer;
	uint8_t tail[4];								/* 帧尾 */
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

typedef struct
{
	TFTLCD_ScreenIDEnum curScreenID;							/* 当前界面ID */
} TFTLCD_StatusTypedef;

#pragma pack(pop)

/******************************************************************************/
extern TFTLCD_RecvBufferTypedef TFTLCD_RecvBuffer;
extern TFTLCD_SendBufferTypedef TFTLCD_SendBuffer;
extern TFTLCD_StatusTypedef TFTLCD_status;

/******************************************************************************/
void TFTLCD_Init(void);
void TFTLCD_SetScreenId(TFTLCD_ScreenIDEnum screen);
void TFTLCD_TextValueUpdate(uint16_t screenID, uint16_t ctlID, char* str, uint8_t size);
void TFTLCD_AnalogDataRefresh(ANALOG_ValueTypedef* analog);
void TFTLCD_AnalogDataAlarmDisplay(ANALOG_ValueTypedef* analog);
void TFTLCD_StatusBarTextRefresh(uint16_t screenID, RT_TimeTypedef* rt, uint8_t batQuantity);
void TFTLCD_StatusBarIconRefresh(uint16_t screenID);
void TFTLCD_HistoryDataFormat(FILE_SaveStructTypedef* saveInfo, TFTLCD_HisDataCtlIdEnum typeID);
void TFTLCD_ChannelSelectICON(TFTLCD_ScreenIDEnum screen, uint16_t typeID, uint8_t status);
void TFTLCD_SelectTimeUpdate(TFTLCD_ScreenIDEnum screen, uint16_t ctlID, FILE_RealTimeTypedef* time);

void TFTLCD_UartIdleDeal(void);
ErrorStatus TFTLCD_CheckHeadTail(void);
//void TFTLCD_printTimeUpdate(FILE_RealTime* rt, CtrlID_PrintEnum ctrl);
//void TFTLCD_printChannelSelectICON(CtrlID_PrintEnum ctrl, uint8_t status);

void TFTLCD_HistoryDataCurveFormat(FILE_SaveStructTypedef* saveInfo);
void TFTLCD_SetPasswordUpdate(uint8_t numb);
#endif
