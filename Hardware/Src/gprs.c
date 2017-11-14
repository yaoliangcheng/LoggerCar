#include "gprs.h"

#include "GPRSProcess.h"

/******************************************************************************/
uint8_t  GPRS_RecvData[GPRS_UART_RX_DATA_SIZE_MAX];
uint8_t  GPRS_signalQuality;						/* GPRS信号质量 */
uint16_t GPRS_PackCount = 0;						/* GPRS包序号 */
uint16_t GPRS_SendPackSize = 0;						/* GPRS发送包大小 */
char     ICCID[20];									/* ICCID */
char	 IMSI[15];									/* IMSI */
char     IMEI[15];									/* IMEI */

GPRS_SendBufferTypedef GPRS_SendBuffer;
GPRS_RecvBufferTypedef GPRS_RecvBuffer;

GPRS_NewSendbufferTyepdef GPRS_NewSendbuffer;

extern osThreadId gprsprocessTaskHandle;

const char Message[] = {0x67, 0x6D, 0x5D, 0xDE, 0x8D, 0xEF, 0x68, 0x3C,
		0x79, 0xD1, 0x62, 0x80};

/******************************************************************************/
static void GPRS_StructInit(GPRS_SendBufferTypedef* sendBuf);
static uint8_t GPRS_VerifyCalculate(uint8_t* pBuffer, uint16_t size);
static void ProtocolFormat_LocationFloat(float value, uint8_t* pBuffer);
static void ProtocolFormat_AnalogFloat(float value, uint8_t* pBuffer, DataFormatEnum format);

/*******************************************************************************
 * function：GPRS初始化，包括发送结构体初始化、串口idle接收初始化
 */
void GPRS_Init(void)
{
	GPRS_NewSendbuffer.head = GPRS_PACK_HEAD;
	GPRS_NewSendbuffer.dataVersion = 2;
	memcpy(GPRS_NewSendbuffer.serialNumber, "1708151515", 10);
	GPRS_NewSendbuffer.deviceTypeCodeH = 10;
	GPRS_NewSendbuffer.deviceTypeCodeL = 10;
	GPRS_NewSendbuffer.firewareVersion = 1;
//	GPRS_NewSendbuffer.PackBuffer.MessageBuffer.packVersion = 1;
//	GPRS_NewSendbuffer.PackBuffer.MessageBuffer.codeCount = 1;
//	memcpy(GPRS_NewSendbuffer.PackBuffer.MessageBuffer.codeNumber[0], "18367053909", 11);

	GPRS_NewSendbuffer.tail = GPRS_PACK_TAIL;

	GPRS_StructInit(&GPRS_SendBuffer);
	UART_DMAIdleConfig(&GPRS_UART, GPRS_RecvData, GPRS_UART_RX_DATA_SIZE_MAX);
}

/*******************************************************************************
 * @brief:GPRS发送命令
 */
void GPRS_SendCmd(char* str)
{
	HAL_UART_Transmit_DMA(&GPRS_UART, (uint8_t*)str, strlen(str));
}

/*******************************************************************************
 * @brief:GPRS发送数据
 */
void GPRS_SendData(uint16_t size)
{
	HAL_UART_Transmit_DMA(&GPRS_UART, (uint8_t*)&GPRS_NewSendbuffer, size);
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
//	GPRS_SendData(&sendBuf->head, dataSize + 5);
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
													 = GPRS_PACK_TAIL;
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
 * @brief：发送实时数据包
 */
uint16_t GPRS_SendDataPackFromCurrent(GPRS_NewSendbufferTyepdef* sendBuffer,
		RT_TimeTypedef* curtime, ANALOG_ValueTypedef* analog,
		GPS_LocateTypedef* location)
{
	uint16_t size = 0;

	/* 包体版本 */
	sendBuffer->PackBuffer.DataBuffer.packVersion = GPRS_PARAM_DATA_PACK_VERSION;
	/* 记录类型为实时数据 */
	sendBuffer->PackBuffer.DataBuffer.recordStatus = GPRS_RECORD_STATUS_CURRENT;
	/* 设置通道数 */
	sendBuffer->PackBuffer.DataBuffer.channelCount = ANALOG_CHANNEL_NUMB;
	/* N个通道参数 */
	memcpy(sendBuffer->PackBuffer.DataBuffer.param, PARAM_DeviceParam.chParam,
			sizeof(ParamTypeTypedef) * ANALOG_CHANNEL_NUMB);
	/* 数据条数为1 */
	sendBuffer->PackBuffer.DataBuffer.dataPackCountH = 0;
	sendBuffer->PackBuffer.DataBuffer.dataPackCountL = 1;

	/* 时间字符串转换成BCD */
	HEX2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[0].year,  &curtime->date.Year,    1);
	HEX2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[0].month, &curtime->date.Month,   1);
	HEX2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[0].day,   &curtime->date.Date,    1);
	HEX2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[0].hour,  &curtime->time.Hours,   1);
	HEX2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[0].min,   &curtime->time.Minutes, 1);
	HEX2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[0].sec,   &curtime->time.Seconds, 1);
	/* 电池电量 */
	sendBuffer->PackBuffer.DataBuffer.SendData[0].batteryLevel = (uint8_t)analog->batVoltage;
	/* 外部电源状态 */
	sendBuffer->PackBuffer.DataBuffer.SendData[0].externalPowerStatus =
			(GPRS_ExternalPowerStatusEnum)INPUT_CheckPwrOnStatus();

	/* 定位为GPS定位 */
	sendBuffer->PackBuffer.DataBuffer.SendData[0].locationStatus = GPRS_LOCATION_TYPE_GPS;
	/* 转换经纬度值 */
	ProtocolFormat_LocationFloat(location->latitude,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].latitude);
	ProtocolFormat_LocationFloat(location->longitude,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].longitude);

	/* 转换模拟量值 */
	ProtocolFormat_AnalogFloat(analog->temp1,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[0],
			sendBuffer->PackBuffer.DataBuffer.param[0].dataFormat);
	ProtocolFormat_AnalogFloat(analog->humi1,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[1],
			sendBuffer->PackBuffer.DataBuffer.param[1].dataFormat);
	ProtocolFormat_AnalogFloat(analog->temp2,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[2],
			sendBuffer->PackBuffer.DataBuffer.param[2].dataFormat);
	ProtocolFormat_AnalogFloat(analog->humi2,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[3],
			sendBuffer->PackBuffer.DataBuffer.param[3].dataFormat);
	ProtocolFormat_AnalogFloat(analog->temp3,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[4],
			sendBuffer->PackBuffer.DataBuffer.param[4].dataFormat);
	ProtocolFormat_AnalogFloat(analog->humi3,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[5],
			sendBuffer->PackBuffer.DataBuffer.param[5].dataFormat);
	ProtocolFormat_AnalogFloat(analog->temp4,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[6],
			sendBuffer->PackBuffer.DataBuffer.param[6].dataFormat);
	ProtocolFormat_AnalogFloat(analog->humi4,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[7],
			sendBuffer->PackBuffer.DataBuffer.param[7].dataFormat);

	/* 当前数据形式固定为61字节 */
	size = 61;
	sendBuffer->PackBuffer.DataBuffer.packSizeH = 0;
	sendBuffer->PackBuffer.DataBuffer.packSizeL = 61;

	/* 包体类型 */
	sendBuffer->packType = GPRS_PACK_TYPE_DATA;
	/* 包序号 */
	GPRS_PackCount++;
	sendBuffer->packCountH = HALFWORD_BYTE_H(GPRS_PackCount);
	sendBuffer->packCountL = HALFWORD_BYTE_L(GPRS_PackCount);
	/* 上传时间 */
	/* 时间字符串转换成BCD */
	HEX2BCD(&sendBuffer->year,  &curtime->date.Year,    1);
	HEX2BCD(&sendBuffer->month, &curtime->date.Month,   1);
	HEX2BCD(&sendBuffer->day,   &curtime->date.Date,    1);
	HEX2BCD(&sendBuffer->hour,  &curtime->time.Hours,   1);
	HEX2BCD(&sendBuffer->min,   &curtime->time.Minutes, 1);
	HEX2BCD(&sendBuffer->sec,   &curtime->time.Seconds, 1);

	/* 字节数 */
//	size = (size + 3) + 23;
	size += 26;
	sendBuffer->dataSizeH = HALFWORD_BYTE_H(size);
	sendBuffer->dataSizeL = HALFWORD_BYTE_L(size);

//	sendBuffer->PackBuffer.DataBuffer.SendData[1] = GPRS_PACK_TAIL;

	*(&sendBuffer->head + size + 3) = GPRS_PACK_TAIL;
	*(&sendBuffer->head + size + 4) =
			GPRS_VerifyCalculate(&sendBuffer->head, size + 4);

	/* 返回整体包大小 */
	return (size + 5);
//	HAL_UART_Transmit_DMA(&GPRS_UART, &sendBuffer->head, size + 5);
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
//static void GPRS_SendData(uint8_t *pData, uint16_t Size)
//{
//	HAL_UART_Transmit_DMA(&GPRS_UART, pData, Size);
//}

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

/*******************************************************************************
 * @brief 定位浮点型值协议格式转换
 */
static void ProtocolFormat_LocationFloat(float value, uint8_t* pBuffer)
{
	uint32_t temp;

	/* 无效的定位数值 */
	if (value == 0)
		memset(pBuffer, 0, 4);

	/* 获取整数部分 */
	*pBuffer = abs((int)value);

	temp = (uint32_t)((value - (*pBuffer)) * 1000000);

	if (value < 0)
		temp |= 0x800000;

	*(pBuffer + 1) = (uint8_t)((temp & 0x00FF0000) >> 16);
	*(pBuffer + 2) = (uint8_t)((temp & 0x0000FF00) >> 8);
	*(pBuffer + 3) = (uint8_t)(temp & 0x000000FF);
}

/*******************************************************************************
 * @brief 模拟量浮点型值协议格式转换
 */
static void ProtocolFormat_AnalogFloat(float value, uint8_t* pBuffer, DataFormatEnum format)
{
	uint16_t temp = 0;
	/* 无效值 */
	if (value == ANALOG_CHANNLE_INVALID_VALUE)
	{
		*pBuffer       = 0xFF;
		*(pBuffer + 1) = 0xFE;
		return;
	}

	switch (format)
	{
	case FORMAT_INT:
		temp = (uint16_t)abs((int)(value));
		break;

	case FORMAT_ONE_DECIMAL:
		temp = (uint16_t)abs((int)(value * 10));
		break;

	case FORMAT_TWO_DECIMAL:
		temp = (uint16_t)abs((int)(value * 100));
		break;
	default:
		break;
	}

	*pBuffer       = HALFWORD_BYTE_H(temp);
	*(pBuffer + 1) = HALFWORD_BYTE_L(temp);

	/* 负数则最高位置一 */
	if (value < 0)
		*pBuffer |= 0x80;
}


