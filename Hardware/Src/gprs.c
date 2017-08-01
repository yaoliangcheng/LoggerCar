#include "gprs.h"
#include "GPRSProcess.h"
#include "osConfig.h"

/******************************************************************************/
uint8_t GPRS_RecvBuffer[GPRS_UART_RX_DATA_SIZE_MAX];
GPRS_BufferStatusTypedef GPRS_BufferStatus;

/*******************************************************************************
 *
 */
void GPRS_StructInit(GPRS_StructTypedef* sendBuf)
{
	sendBuf->head = GPRS_PACK_HEAD;

	sendBuf->locationType = LOCATION_GPS;

	/* eeprom中读出数据 */
	memcpy(&sendBuf->seriaNumber, EE_deviceSN, sizeof(sendBuf->seriaNumber));
	sendBuf->firmwareVersion = 	  EE_firmwareVersion;
	sendBuf->recordInterval = 	  EE_recordInterval;
	sendBuf->overLimitInterval =  EE_overLimitRecordInterval;

	/* 设置通道类型 */
//	sendBuf->analogChannelNumb = ANALOG_CHANNEL_NUMB;
	sendBuf->analogChannelNumb    = 8;
	sendBuf->param[0].channelType = TYPE_TEMP;
	sendBuf->param[0].channelUnit = UNIT_TEMP;
	sendBuf->param[0].dataFormat  = FORMAT_ONE_DECIMAL;
	sendBuf->param[1].channelType = TYPE_HUMI;
	sendBuf->param[1].channelUnit = UNIT_HUMI;
	sendBuf->param[1].dataFormat  = FORMAT_ONE_DECIMAL;
	sendBuf->param[2].channelType = TYPE_TEMP;
	sendBuf->param[2].channelUnit = UNIT_TEMP;
	sendBuf->param[2].dataFormat  = FORMAT_ONE_DECIMAL;
	sendBuf->param[3].channelType = TYPE_HUMI;
	sendBuf->param[3].channelUnit = UNIT_HUMI;
	sendBuf->param[3].dataFormat  = FORMAT_ONE_DECIMAL;
	sendBuf->param[4].channelType = TYPE_TEMP;
	sendBuf->param[4].channelUnit = UNIT_TEMP;
	sendBuf->param[4].dataFormat  = FORMAT_ONE_DECIMAL;
	sendBuf->param[5].channelType = TYPE_HUMI;
	sendBuf->param[5].channelUnit = UNIT_HUMI;
	sendBuf->param[5].dataFormat  = FORMAT_ONE_DECIMAL;
	sendBuf->param[6].channelType = TYPE_TEMP;
	sendBuf->param[6].channelUnit = UNIT_TEMP;
	sendBuf->param[6].dataFormat  = FORMAT_ONE_DECIMAL;
	sendBuf->param[7].channelType = TYPE_HUMI;
	sendBuf->param[7].channelUnit = UNIT_HUMI;
	sendBuf->param[7].dataFormat  = FORMAT_ONE_DECIMAL;

	sendBuf->tail = GPRS_PACK_TAIL;
}

/*******************************************************************************
 *
 */
void GPRS_Init(void)
{
	UART_DMAIdleConfig(&GPRS_UART, GPRS_RecvBuffer, GPRS_UART_RX_DATA_SIZE_MAX);
}

/*******************************************************************************
 *
 */
void GPRS_SendData(uint8_t *pData, uint16_t Size)
{
	HAL_UART_Transmit_DMA(&GPRS_UART, pData, Size);
}

/*******************************************************************************
 *
 */
void GPRS_SendCmd(char* str)
{
	GPRS_SendData((uint8_t*)str, strlen(str));
}

/******************************************************************************/
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
 *
 */
void GPRS_RstModule(void)
{
	GPRS_RST_CTRL_ENABLE();
	osDelay(100);
	GPRS_RST_CTRL_DISABLE();
}

/*******************************************************************************
 * gprs数据校验
 */
uint8_t GPRS_VerifyCalculate(uint8_t* pBuffer, uint8_t size)
{
	uint8_t cal = 0;

	while(size--)
	{
		cal += (*pBuffer++);
	}

	return cal;
}

/*******************************************************************************
 *
 */
void GPRS_SendProtocol(GPRS_StructTypedef* sendBuf)
{
	uint16_t dataSize = 0;
	uint16_t packNum  = 0;

	/* 需要发送的数据包数 = 本次发送的包 + 增加发送的数据包数 */
//	packNum = GPRS_Status.addSendPackNumb + 1;
	packNum = 1;

	/* 数据长度计算方法：m=数据包数； n=通道数
	 * 数据长度 = m(17+2n)+3n+23 */
//	dataSize = packNum * (17 + 2 * ANALOG_CHANNEL_NUMB)
//				+ sizeof(GPRS_ParamTypedef) * ANALOG_CHANNEL_NUMB
//				+ 23;
	dataSize = 80;

	/* 获取数据长度 */
	sendBuf->dateSizeH = HalfWord_GetHighByte(dataSize);
	sendBuf->dateSizeL = HalfWord_GetLowByte(dataSize);

	/* 获取数据包数 */
	sendBuf->dataPackNumbH = HalfWord_GetHighByte(packNum);
	sendBuf->dataPackNumbL = HalfWord_GetLowByte(packNum);

	/* 在数据的结尾，添加上帧尾 */
	memcpy(&sendBuf->dataPack[packNum], &sendBuf->tail, 1);

	/* 计算校验 */
	sendBuf->verifyData =
			GPRS_VerifyCalculate(&sendBuf->head, dataSize + 4);
	/* 数据最后添加上校验 */
	memcpy((void*)
		   (&(sendBuf->dataPack[packNum].realTime.month)),
			&sendBuf->verifyData, 1);

	/* 发送数据，发送的总字节数 = 5+数据长度 */
	GPRS_SendData(&sendBuf->head, dataSize + 5);
}




