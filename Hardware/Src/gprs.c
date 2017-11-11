#include "gprs.h"

#include "GPRSProcess.h"

/******************************************************************************/
uint8_t GPRS_RecvData[GPRS_UART_RX_DATA_SIZE_MAX];
uint8_t GPRS_signalQuality;			/* GPRS信号质量 */

GPRS_SendBufferTypedef GPRS_SendBuffer;
GPRS_RecvBufferTypedef GPRS_RecvBuffer;

GPRS_NewSendbufferTyepdef GPRS_NewSendbuffer;

extern osThreadId gprsprocessTaskHandle;

const char Message[] = {0x67, 0x6D, 0x5D, 0xDE, 0x8D, 0xEF, 0x68, 0x3C,
		0x79, 0xD1, 0x62, 0x80};

/******************************************************************************/
static void GPRS_StructInit(GPRS_SendBufferTypedef* sendBuf);
static void GPRS_SendData(uint8_t *pData, uint16_t Size);
static uint8_t GPRS_VerifyCalculate(uint8_t* pBuffer, uint16_t size);

/*******************************************************************************
 * function：GPRS初始化，包括发送结构体初始化、串口idle接收初始化
 */
void GPRS_Init(void)
{
	GPRS_NewSendbuffer.head = GPRS_PACK_HEAD_NEW;
	GPRS_NewSendbuffer.dataVersion = 2;
	memcpy(GPRS_NewSendbuffer.serialNumber, "1708151515", 10);
	GPRS_NewSendbuffer.deviceTypeCodeH = 10;
	GPRS_NewSendbuffer.deviceTypeCodeL = 10;
	GPRS_NewSendbuffer.firewareVersion = 1;
	GPRS_NewSendbuffer.PackBuffer.MessageBuffer.packVersion = 1;
	GPRS_NewSendbuffer.PackBuffer.MessageBuffer.codeCount = 1;
	memcpy(GPRS_NewSendbuffer.PackBuffer.MessageBuffer.codeNumber[0], "18367053909", 11);

	GPRS_NewSendbuffer.tail = GPRS_PACK_TAIL_NEW;

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

	str2numb(temp, &buf[GPRS_SIGNAL_QUALITY_OFFSET], 2);
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

uint8_t MessageModule[] = {0x37, 0x00, 0x92, 0x02, 0x38, 0x33, 0x31, 0x31, 0x32,
		0x31, 0x30, 0x30, 0x31, 0x32, 0x0B, 0x7E, 0x8C, 0x17, 0x10, 0x16, 0x19,
		0x37, 0x28, 0x00, 0x00, 0x01, 0x02, 0x00, 0x78, 0x38, 0x39, 0x38, 0x36,
		0x30, 0x32, 0x62, 0x34,	0x31, 0x31, 0x31, 0x36, 0x63, 0x30, 0x35, 0x38,
		0x39, 0x39, 0x32, 0x35, 0x34, 0x36, 0x30, 0x30, 0x34, 0x30, 0x34, 0x30,
		0x38, 0x30, 0x30, 0x32, 0x39, 0x32, 0x35, 0x38, 0x36, 0x36, 0x30, 0x35,
		0x30, 0x30, 0x33, 0x35, 0x35, 0x30, 0x32, 0x31, 0x39, 0x32, 0x01, 0x31,
		0x35, 0x39, 0x38, 0x38, 0x34, 0x35, 0x38, 0x30, 0x35, 0x31, 0x00, 0x38,
		0x00, 0x0A, 0x00, 0x31, 0x00, 0x37, 0x00, 0x31, 0x00, 0x30, 0x00, 0x31,
		0x00, 0x36, 0x00, 0x20, 0x00, 0x31, 0x00, 0x39, 0x00, 0x3A, 0x00, 0x33,
		0x00, 0x34, 0x00, 0x0A, 0x00, 0x53, 0x00, 0x48, 0x00, 0x54, 0x00, 0x33,
		0x00, 0x30, 0x00, 0x0A, 0x00, 0x31, 0x6E, 0x29, 0x00, 0x32, 0x00, 0x33,
		0x00, 0x2E, 0x00, 0x30, 0x8D, 0x85, 0x00, 0x0A, 0x38, 0xA5};

/*******************************************************************************
 * @brief 发送短信包
 */
void GPRS_SendMessagePack(GPRS_NewSendbufferTyepdef* sendBuffer,
		RT_TimeTypedef curtime,	char* messageContent, uint16_t messageCount)
{
	uint16_t size = 0;
	static uint16_t GPRS_PackCount = 0;

	/* 短信内容字节数 */
	sendBuffer->PackBuffer.MessageBuffer.contentCountH = HALFWORD_BYTE_H(messageCount);
	sendBuffer->PackBuffer.MessageBuffer.contentCountL = HALFWORD_BYTE_L(messageCount);

	/* 短信内容 */
	memcpy(GPRS_NewSendbuffer.PackBuffer.MessageBuffer.content, messageContent,
			messageCount);
	/* 包体长度 = 53 + 11 * 号码个数 + 短信内容字节数 */
	size = messageCount + sendBuffer->PackBuffer.MessageBuffer.codeCount * 11 + 53;
	sendBuffer->PackBuffer.MessageBuffer.packSizeH = HALFWORD_BYTE_H(size);
	sendBuffer->PackBuffer.MessageBuffer.packSizeL = HALFWORD_BYTE_L(size);

	/* 包体类型 */
	sendBuffer->packType = GPRS_PACK_TYPE_MESSAGE;
	/* 包序号 */
	GPRS_PackCount++;
	sendBuffer->packCountH = HALFWORD_BYTE_H(GPRS_PackCount);
	sendBuffer->packCountL = HALFWORD_BYTE_L(GPRS_PackCount);
	/* 上传时间 */
	/* 时间字符串转换成BCD */
	HEX2BCD(&sendBuffer->year,  &curtime.date.Year,    1);
	HEX2BCD(&sendBuffer->month, &curtime.date.Month,   1);
	HEX2BCD(&sendBuffer->day,   &curtime.date.Date,    1);
	HEX2BCD(&sendBuffer->hour,  &curtime.time.Hours,   1);
	HEX2BCD(&sendBuffer->min,   &curtime.time.Minutes, 1);
	HEX2BCD(&sendBuffer->sec,   &curtime.time.Seconds, 1);


	/* 字节数 */
	size = (size + 3) + 23;
	sendBuffer->dataSizeH = HALFWORD_BYTE_H(size);
	sendBuffer->dataSizeL = HALFWORD_BYTE_L(size);

	if (messageCount < GPRS_MESSAGE_BYTES_MAX)
	{
		sendBuffer->PackBuffer.MessageBuffer.content[messageCount]
													 = GPRS_PACK_TAIL_NEW;
	}

	sendBuffer->verify =
			GPRS_VerifyCalculate(&sendBuffer->head, size + 4);

	if (messageCount < GPRS_MESSAGE_BYTES_MAX)
	{
		sendBuffer->PackBuffer.MessageBuffer.content[messageCount + 1]
													 = sendBuffer->verify;
	}



//	memcpy(&sendBuffer->head, MessageModule, sizeof(MessageModule));

	HAL_UART_Transmit_DMA(&GPRS_UART, &sendBuffer->head, size + 5);
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
