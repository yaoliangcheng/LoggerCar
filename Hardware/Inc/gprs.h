#ifndef __GPRS_H
#define __GPRS_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "param.h"
#include "exFlash.h"
#include "analog.h"
#include "file.h"

/******************************************************************************/
#define GPRS_UART 						(huart1)
#define GPRS_UART_DMA_RX_FLAG			(DMA_FLAG_GL5)

#define GPRS_PWR_CTRL_ENABLE() \
		HAL_GPIO_WritePin(O_GSM_PWR_GPIO_Port, O_GSM_PWR_Pin, GPIO_PIN_SET);
#define GPRS_PWR_CTRL_DISABLE() \
		HAL_GPIO_WritePin(O_GSM_PWR_GPIO_Port, O_GSM_PWR_Pin, GPIO_PIN_RESET);

#define GPRS_RST_CTRL_ENABLE() \
		HAL_GPIO_WritePin(O_GSM_RST_GPIO_Port, O_GSM_RST_Pin, GPIO_PIN_RESET);
#define GPRS_RST_CTRL_DISABLE() \
		HAL_GPIO_WritePin(O_GSM_RST_GPIO_Port, O_GSM_RST_Pin, GPIO_PIN_SET);

/******************************************************************************/
#define GPRS_PATCH_PACK_NUMB_MAX		  (20)				/* 最多支持补传数据组数 */
#define GPRS_PACK_HEAD					  (0X37)
#define GPRS_PACK_TAIL					  (0x38)

#define GPRS_UART_RX_DATA_SIZE_MAX		  (50)
#define GPRS_SIGNAL_QUALITY_OFFSET		  (8)

#define GPRS_MESSAGE_BYTES_MAX			  (70)

#define GPRS_PARAM_DATA_VERSION				(0x02)
#define GPRS_PARAM_DEVICE_TYPE_CODE			(0x0A0A)
#define GPRS_PARAM_FIRMWARE_VERSION			(0x01)
#define GPRS_PARAM_MESSAGE_PACK_VERSION		(0x01)
#define GPRS_PARAM_DATA_PACK_VERSION		(0x01)

/******************************************************************************/
typedef enum
{
	GPRS_PACK_TYPE_MESSAGE = 0x01,						/* 短信包 */
	GPRS_PACK_TYPE_DATA,								/* 数据包 */
} GPRS_PackTypeEnum;

typedef enum
{
	GPRS_RECORD_STATUS_CURRENT,							/* 实时数据 */
	GPRS_RECORD_STATUS_RECORD,							/* 历史数据 */
} GPRS_RecordStatusEnum;								/* 数据包数据类型 */

typedef enum
{
	GPRS_EXTERNAL_PWR_OFF,								/* 无外部电源 */
	GPRS_EXTERNAL_PWR_ON,								/* 有外部电源 */
} GPRS_ExternalPowerStatusEnum;							/* 外部电源状态 */




/******************************************************************************/
#pragma pack(push)
#pragma pack(1)											/* 按字节对齐 */

typedef struct
{
	uint8_t recvBuffer[GPRS_UART_RX_DATA_SIZE_MAX];			/* 接收缓存 */
	uint8_t bufferSize;										/* 缓存大小 */
} GPRS_RecvBufferTypedef;

typedef struct
{
	uint8_t  year;								/* 时间 */
	uint8_t  month;
	uint8_t  day;
	uint8_t  hour;
	uint8_t  min;
	uint8_t  sec;
	uint8_t  batteryLevel;						/* 电池电量 */
	uint8_t  externalPowerStatus;				/* 外部电池状态 */
	uint32_t longitude;							/* 经度 */
	uint32_t latitude;							/* 纬度 */
	uint8_t  resever;							/* 保留 */
	uint16_t analogValue[8];					/* 模拟量值 */
} GPRS_SendInfoTypedef;

typedef struct
{
	uint8_t head;											/* 数据头 */
	uint8_t dateSizeH;										/* 字节数 高位 */
	uint8_t dateSizeL;										/* 字节数 低位 */
	char seriaNumber[10];									/* SN号 */
	LocationTypdEnum locationType;							/* 定位标志 */
	uint8_t firmwareVersion;								/* 固件版本 */
	uint8_t recordInterval;									/* 记录间隔 */
	uint8_t overLimitInterval;								/* 超标间隔 */
	uint8_t resever[6];										/* 预留 */
	uint8_t exitAnalogChannelNumb;							/* 模拟量n通道数 */
	ParamTypeTypedef param[ANALOG_CHANNEL_NUMB];			/* n个通道参数 */
	uint8_t dataPackNumbH;									/* 数据条数m高位 */
	uint8_t dataPackNumbL;									/* 数据条数m低位 */
	GPRS_SendInfoTypedef dataPack[GPRS_PATCH_PACK_NUMB_MAX];		/* m条数据点 */
	uint8_t tail;											/* 数据尾 */
	uint8_t verifyData;										/* 校验数据 */
} GPRS_SendBufferTypedef;

/******************************************************************************/
typedef struct
{
	uint8_t  packVersion;								/* 包体版本 */
	uint8_t  packSizeH;									/* 包体长度H */
	uint8_t  packSizeL;									/* 包体长度L */
	char     ICCID[20];									/* ICCID */
	char	 IMSI[15];									/* IMSI */
	char     IMEI[15];									/* IMEI */
	uint8_t  codeCount;									/* 号码数 */
	char     codeNumber[1][11];							/* 号码 */
	uint8_t  contentCountH;								/* 短信内容字节数H */
	uint8_t  contentCountL;								/* 短信内容字节数L */
	char     content[GPRS_MESSAGE_BYTES_MAX];			/* 短信内容 */
} MessageBufferTypedef;									/* 短信包 */

typedef struct
{
	uint8_t year;										/* 时间 */
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t batteryLevel;								/* 电池电量 */
	GPRS_ExternalPowerStatusEnum externalPowerStatus;	/* 外部电池状态 */
	GPS_LocationTypeEnum locationStatus;				/* 定位标志 */
	uint32_t longitude;									/* 经度 */
	uint32_t latitude;									/* 纬度 */
	uint16_t analogValue[ANALOG_CHANNEL_NUMB];			/* 模拟量值 */
} SendDataTypedef;

typedef struct
{
	uint8_t packVersion;								/* 包体版本 */
	uint8_t packSizeH;									/* 包体长度 */
	uint8_t packSizeL;
	GPRS_RecordStatusEnum recordStatus;					/* 记录标记 */
	uint8_t channelCount;								/* 通道数 */
	ParamTypeTypedef param[ANALOG_CHANNEL_NUMB];		/* n个通道参数 */
	uint8_t dataPackCountH;								/* 数据条数 */
	uint8_t dataPackCountL;
	SendDataTypedef SendData[GPRS_PATCH_PACK_NUMB_MAX];	/* 数据点 */
} DataBufferTypedef;

typedef struct
{
	uint8_t  head;										/* 数据头 */
	uint8_t  dataSizeH;									/* 字节数H */
	uint8_t  dataSizeL;									/* 字节数L */
	uint8_t  dataVersion;								/* 数据包版本 */
	char     serialNumber[10];							/* SN号 */
	uint8_t  deviceTypeCodeH;							/* 型号编码H */
	uint8_t  deviceTypeCodeL;							/* 型号编码L */
	uint8_t  firewareVersion;							/* 固件版本 */
	uint8_t  year;										/* 上传时间 */
	uint8_t  month;
	uint8_t  day;
	uint8_t  hour;
	uint8_t  min;
	uint8_t  sec;
	uint8_t packCountH;									/* 数据包序号H */
	uint8_t packCountL;									/* 数据包序号L */
	GPRS_PackTypeEnum packType;							/* 包体类型 */
	union
	{
		MessageBufferTypedef MessageBuffer;				/* 短信包 */
		DataBufferTypedef    DataBuffer;				/* 数据包 */
	} PackBuffer;										/* 包体 */
	uint8_t tail;										/* 数据尾 */
	uint8_t verify;										/* 校验和 */
} GPRS_NewSendbufferTyepdef;							/* 新协议 */

#pragma pack(pop)

/******************************************************************************/
extern GPRS_RecvBufferTypedef GPRS_RecvBuffer;
extern uint8_t GPRS_signalQuality;			/* GPRS信号质量 */
extern GPRS_SendBufferTypedef GPRS_SendBuffer;

/******************************************************************************/
void GPRS_Init(void);
void GPRS_SendCmd(char* str);
void GPRS_SendData(uint16_t size);
void GPRS_RstModule(void);
void GPRS_SendProtocol(GPRS_SendBufferTypedef* sendBuf);
uint8_t GPRS_GetSignalQuality(uint8_t* buf);
void GPRS_UartIdleDeal(void);
void GPRS_SendMessagePack(GPRS_NewSendbufferTyepdef* sendBuffer,
		RT_TimeTypedef curtime,	char* messageContent, uint16_t messageCount);

uint16_t GPRS_SendDataPackFromCurrent(GPRS_NewSendbufferTyepdef* sendBuffer,
		RT_TimeTypedef* curtime, ANALOG_ValueTypedef* analog,
		GPS_LocateTypedef* location);
uint16_t GPRS_SendDataPackFromRecord(GPRS_NewSendbufferTyepdef* sendBuffer,
		FILE_SaveStructTypedef* saveInfo, uint16_t sendPackCount, RT_TimeTypedef* curtime);
#endif
