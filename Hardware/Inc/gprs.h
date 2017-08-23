#ifndef __GPRS_H
#define __GPRS_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "exFlash.h"
#include "file.h"

/******************************************************************************/
#define GPRS_UART 						(huart2)
#define GPRS_PWR_CTRL_ENABLE() \
		HAL_GPIO_WritePin(GPRS_PWRKEY_CTRL_GPIO_Port, GPRS_PWRKEY_CTRL_Pin, GPIO_PIN_SET);
#define GPRS_PWR_CTRL_DISABLE() \
		HAL_GPIO_WritePin(GPRS_PWRKEY_CTRL_GPIO_Port, GPRS_PWRKEY_CTRL_Pin, GPIO_PIN_RESET);

#define GPRS_RST_CTRL_ENABLE() \
		HAL_GPIO_WritePin(GPRS_RST_CTRL_GPIO_Port, GPRS_RST_CTRL_Pin, GPIO_PIN_RESET);
#define GPRS_RST_CTRL_DISABLE() \
		HAL_GPIO_WritePin(GPRS_RST_CTRL_GPIO_Port, GPRS_RST_CTRL_Pin, GPIO_PIN_SET);

/******************************************************************************/
#define GPRS_PATCH_PACK_NUMB_MAX		  (30)				/* 最多支持补传数据组数 */
#define GPRS_PACK_HEAD					  (uint8_t)(0X31)
#define GPRS_PACK_TAIL					  (uint8_t)(0x32)

#define GPRS_UART_RX_DATA_SIZE_MAX		  (50)

#define GPRS_SIGNAL_QUALITY_OFFSET			(8)

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)											/* 按字节对齐 */

typedef struct
{
	uint8_t recvBuffer[GPRS_UART_RX_DATA_SIZE_MAX];			/* 接收缓存 */
	uint8_t bufferSize;										/* 缓存大小 */
} GPRS_BufferStatusTypedef;

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
	FILE_InfoTypedef dataPack[GPRS_PATCH_PACK_NUMB_MAX];	/* m条数据点 */
	uint8_t tail;											/* 数据尾 */
	uint8_t verifyData;										/* 校验数据 */
} GPRS_StructTypedef;

#pragma pack(pop)
/******************************************************************************/
//extern uint8_t GPRS_RecvBuffer[GPRS_UART_RX_DATA_SIZE_MAX];
extern GPRS_BufferStatusTypedef GPRS_BufferStatus;

/******************************************************************************/
void GPRS_Init(void);
void GPRS_StructInit(GPRS_StructTypedef* sendBuf);
void GPRS_SendCmd(char* str);
void GPRS_UartIdleDeal(void);
void GPRS_SendProtocol(GPRS_StructTypedef* sendBuf, uint16_t patchPack);
void GPRS_RstModule(void);
uint8_t GPRS_GetSignalQuality(uint8_t* buf);
#endif
