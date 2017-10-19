#include "gprs.h"

#include "GPRSProcess.h"
#include "osConfig.h"

/******************************************************************************/
uint8_t GPRS_RecvData[GPRS_UART_RX_DATA_SIZE_MAX];
uint8_t GPRS_signalQuality;			/* GPRS信号质量 */

GPRS_SendBufferTypedef GPRS_SendBuffer;
GPRS_RecvBufferTypedef GPRS_RecvBuffer;

/******************************************************************************/
static void GPRS_StructInit(GPRS_SendBufferTypedef* sendBuf);
static void GPRS_SendData(uint8_t *pData, uint16_t Size);
static uint8_t GPRS_VerifyCalculate(uint8_t* pBuffer, uint16_t size);

/*******************************************************************************
 * function：GPRS初始化，包括发送结构体初始化、串口idle接收初始化
 */
void GPRS_Init(void)
{
	GPRS_StructInit(&GPRS_SendBuffer);
	UART_DMAIdleConfig(&GPRS_UART, GPRS_RecvData, GPRS_UART_RX_DATA_SIZE_MAX);
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
void GPRS_SendProtocol(GPRS_SendBufferTypedef* sendBuf)
{
	uint16_t dataSize = 0;

	/* 数据长度计算方法：m=数据包数； n=通道数
	 * 数据长度 = m(17+2n)+3n+23 */
//	dataSize = patchPack * (17 + 2 * ANALOG_CHANNEL_NUMB)
//				+ sizeof(GPRS_ParamTypedef) * ANALOG_CHANNEL_NUMB
//				+ 23;
	dataSize = GPRS_SendBuffer.dataPackNumbL * sizeof(GPRS_SendInfoTypedef) + 47;

	/* 获取数据长度 */
	sendBuf->dateSizeH = HALFWORD_BYTE_H(dataSize);
	sendBuf->dateSizeL = HALFWORD_BYTE_L(dataSize);

	/* 如果缓存未被装在满，则在下一数据中加入帧尾 */
	if (GPRS_SendBuffer.dataPackNumbL < GPRS_PATCH_PACK_NUMB_MAX)
	{
		/* 在数据的结尾，添加上帧尾 */
		memcpy(&sendBuf->dataPack[GPRS_SendBuffer.dataPackNumbL], &sendBuf->tail, 1);
	}

	/* 计算校验 */
	sendBuf->verifyData =
			GPRS_VerifyCalculate(&sendBuf->head, dataSize + 4);
	if (GPRS_SendBuffer.dataPackNumbL < GPRS_PATCH_PACK_NUMB_MAX)
	{
		/* 数据最后添加上校验 */
		memcpy((void*)(&(sendBuf->dataPack[GPRS_SendBuffer.dataPackNumbL].month)),
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

		GPRS_RecvBuffer.bufferSize = GPRS_UART_RX_DATA_SIZE_MAX
						- __HAL_DMA_GET_COUNTER(GPRS_UART.hdmarx);

		memcpy(GPRS_RecvBuffer.recvBuffer, GPRS_RecvData, GPRS_RecvBuffer.bufferSize);
		memset(GPRS_RecvData, 0, GPRS_RecvBuffer.bufferSize);

		osSignalSet(gprsprocessTaskHandle, GPRS_PROCESS_TASK_RECV_ENABLE);

		GPRS_UART.hdmarx->Instance->CNDTR = GPRS_UART.RxXferSize;
		__HAL_DMA_ENABLE(GPRS_UART.hdmarx);
	}
}

/*******************************************************************************
 * function:发送到平台数据结构初始化
 */
static void GPRS_StructInit(GPRS_SendBufferTypedef* sendBuf)
{
	sendBuf->head = GPRS_PACK_HEAD;

	sendBuf->locationType = PARAM_DeviceParam.locationType;

	/* eeprom中读出数据 */
	memcpy(&sendBuf->seriaNumber, PARAM_DeviceParam.deviceSN, sizeof(sendBuf->seriaNumber));
	sendBuf->firmwareVersion = 	  PARAM_DeviceParam.firmwareVersion;
	sendBuf->recordInterval = 	  PARAM_DeviceParam.recordInterval;
	sendBuf->overLimitInterval =  PARAM_DeviceParam.overLimitRecordInterval;

	/* 设置通道类型 */
	sendBuf->exitAnalogChannelNumb = PARAM_DeviceParam.exAnalogChannelNumb;
	memcpy(&sendBuf->param[0], &PARAM_DeviceParam.chParam[0], sizeof(ParamTypeTypedef) * 8);

	/* 数据包长度最大不可能超过255，包个数H只能为0 */
	sendBuf->dataPackNumbH = 0x00;

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
