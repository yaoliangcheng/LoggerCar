#include "gprs.h"


#include "GPRSProcess.h"
#include "osConfig.h"

/******************************************************************************/
uint8_t GPRS_RecvBuffer[GPRS_UART_RX_DATA_SIZE_MAX];
GPRS_BufferStatusTypedef GPRS_BufferStatus;
uint8_t GPRS_signalQuality;			/* GPRS信号质量 */

/******************************************************************************/
static void GPRS_StructInit(GPRS_StructTypedef* sendBuf);
static void GPRS_SendData(uint8_t *pData, uint16_t Size);
static uint8_t GPRS_VerifyCalculate(uint8_t* pBuffer, uint16_t size);

/*******************************************************************************
 * function：GPRS初始化，包括发送结构体初始化、串口idle接收初始化
 */
void GPRS_Init(GPRS_StructTypedef* sendBuf)
{
	GPRS_StructInit(sendBuf);
	UART_DMAIdleConfig(&GPRS_UART, GPRS_RecvBuffer, GPRS_UART_RX_DATA_SIZE_MAX);
}

/*******************************************************************************
 * function:GPRS发送命令
 */
void GPRS_SendCmd(char* str)
{
	GPRS_SendData((uint8_t*)str, strlen(str));
}

/*******************************************************************************
 * function：GPRS模块复位
 */
void GPRS_RstModule(void)
{
	GPRS_RST_CTRL_ENABLE();
	osDelay(100);
	GPRS_RST_CTRL_DISABLE();
}

/*******************************************************************************
 * function：GPRS发送协议到平台
 * sendBuf：发送数据指针
 * patch：该缓存中包含几个补传的数据
 */
void GPRS_SendProtocol(GPRS_StructTypedef* sendBuf, uint16_t patchPack)
{
	uint16_t dataSize = 0;

	/* 数据长度计算方法：m=数据包数； n=通道数
	 * 数据长度 = m(17+2n)+3n+23 */
//	dataSize = patchPack * (17 + 2 * ANALOG_CHANNEL_NUMB)
//				+ sizeof(GPRS_ParamTypedef) * ANALOG_CHANNEL_NUMB
//				+ 23;
	dataSize = patchPack * 33 + 47;

	/* 获取数据长度 */
	sendBuf->dateSizeH = HalfWord_GetHighByte(dataSize);
	sendBuf->dateSizeL = HalfWord_GetLowByte(dataSize);

	/* 获取数据包数 */
	sendBuf->dataPackNumbH = HalfWord_GetHighByte(patchPack);
	sendBuf->dataPackNumbL = HalfWord_GetLowByte(patchPack);

	if (patchPack < GPRS_PATCH_PACK_NUMB_MAX)
	{
		/* 在数据的结尾，添加上帧尾 */
		memcpy(&sendBuf->dataPack[patchPack], &sendBuf->tail, 1);
	}

	/* 计算校验 */
	sendBuf->verifyData =
			GPRS_VerifyCalculate(&sendBuf->head, dataSize + 4);
	if (patchPack < GPRS_PATCH_PACK_NUMB_MAX)
	{
		/* 数据最后添加上校验 */
		memcpy((void*)(&(sendBuf->dataPack[patchPack].realTime.month)),
					&sendBuf->verifyData, 1);
	}

	/* 发送数据，发送的总字节数 = 5+数据长度 */
	GPRS_SendData(&sendBuf->head, dataSize + 5);
}

/*******************************************************************************
 * function：获取信号质量
 */
uint8_t GPRS_GetSignalQuality(uint8_t* buf)
{
	uint8_t temp[2];

	str2numb(&buf[GPRS_SIGNAL_QUALITY_OFFSET], temp, 2);
	return (uint8_t)(temp[0] * 10 + temp[1]);
}

/*******************************************************************************
 * function:uart——idle接收处理
 */
void GPRS_UartIdleDeal(void)
{
	uint32_t tmp_flag = 0, tmp_it_source = 0;

	tmp_flag = __HAL_UART_GET_FLAG(&GPRS_UART, UART_FLAG_IDLE);
	tmp_it_source = __HAL_UART_GET_IT_SOURCE(&GPRS_UART, UART_IT_IDLE);
	if((tmp_flag != RESET) && (tmp_it_source != RESET))
	{
		__HAL_DMA_DISABLE(GPRS_UART.hdmarx);
		__HAL_DMA_CLEAR_FLAG(GPRS_UART.hdmarx, DMA_FLAG_GL6);

		/* Clear Uart IDLE Flag */
		__HAL_UART_CLEAR_IDLEFLAG(&GPRS_UART);

		GPRS_BufferStatus.bufferSize = GPRS_UART_RX_DATA_SIZE_MAX
						- __HAL_DMA_GET_COUNTER(GPRS_UART.hdmarx);

		memcpy(GPRS_BufferStatus.recvBuffer, GPRS_RecvBuffer, GPRS_BufferStatus.bufferSize);
		memset(GPRS_RecvBuffer, 0, GPRS_BufferStatus.bufferSize);

		osSignalSet(gprsprocessTaskHandle, GPRS_PROCESS_TASK_RECV_ENABLE);

		GPRS_UART.hdmarx->Instance->CNDTR = GPRS_UART.RxXferSize;
		__HAL_DMA_ENABLE(GPRS_UART.hdmarx);
	}
}

/*******************************************************************************
 * function:发送到平台数据结构初始化
 */
static void GPRS_StructInit(GPRS_StructTypedef* sendBuf)
{
	sendBuf->head = GPRS_PACK_HEAD;

	sendBuf->locationType = FILE_DeviceParam.locationType;

	/* eeprom中读出数据 */
	memcpy(&sendBuf->seriaNumber, FILE_DeviceParam.deviceSN, sizeof(sendBuf->seriaNumber));
	sendBuf->firmwareVersion = 	  FILE_DeviceParam.firmwareVersion;
	sendBuf->recordInterval = 	  FILE_DeviceParam.recordInterval;
	sendBuf->overLimitInterval =  FILE_DeviceParam.overLimitRecordInterval;

	/* 设置通道类型 */
	sendBuf->exitAnalogChannelNumb = FILE_DeviceParam.exitAnalogChannelNumb;
	memcpy(&sendBuf->param[0], &FILE_DeviceParam.param[0], sizeof(sendBuf->param));

	sendBuf->tail = GPRS_PACK_TAIL;
}

/*******************************************************************************
 * function:串口dma发送数据
 */
static void GPRS_SendData(uint8_t *pData, uint16_t Size)
{
	HAL_UART_Transmit_DMA(&GPRS_UART, pData, Size);
}

/*******************************************************************************
 * function：gprs发送数据校验
 */
static uint8_t GPRS_VerifyCalculate(uint8_t* pBuffer, uint16_t size)
{
	uint8_t cal = 0;

	while(size--)
	{
		cal += (*pBuffer++);
	}

	return cal;
}
