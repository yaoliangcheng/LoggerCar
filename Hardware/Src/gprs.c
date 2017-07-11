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
	sendBuf->firmwareVersion = EE_firmwareVersion;
	sendBuf->recordInterval = EE_recordInterval;
	sendBuf->overLimitInterval = EE_overLimitRecordInterval;

	/* 设置通道类型 */
	sendBuf->analogChannelNumb = ANALOG_CHANNEL_NUMB;
	sendBuf->param[0].channelType = TYPE_TEMP;
	sendBuf->param[0].channelUnit = UNIT_TEMP;
	sendBuf->param[0].dataFormat = FORMAT_ONE_DECIMAL;
	sendBuf->param[1].channelType = TYPE_HUMI;
	sendBuf->param[1].channelUnit = UNIT_HUMI;
	sendBuf->param[1].dataFormat = FORMAT_ONE_DECIMAL;
	sendBuf->param[2].channelType = TYPE_TEMP;
	sendBuf->param[2].channelUnit = UNIT_TEMP;
	sendBuf->param[2].dataFormat = FORMAT_ONE_DECIMAL;
	sendBuf->param[3].channelType = TYPE_HUMI;
	sendBuf->param[3].channelUnit = UNIT_HUMI;
	sendBuf->param[3].dataFormat = FORMAT_ONE_DECIMAL;
	sendBuf->param[4].channelType = TYPE_TEMP;
	sendBuf->param[4].channelUnit = UNIT_TEMP;
	sendBuf->param[4].dataFormat = FORMAT_ONE_DECIMAL;
	sendBuf->param[5].channelType = TYPE_HUMI;
	sendBuf->param[5].channelUnit = UNIT_HUMI;
	sendBuf->param[5].dataFormat = FORMAT_ONE_DECIMAL;
	sendBuf->param[6].channelType = TYPE_TEMP;
	sendBuf->param[6].channelUnit = UNIT_TEMP;
	sendBuf->param[6].dataFormat = FORMAT_ONE_DECIMAL;
	sendBuf->param[7].channelType = TYPE_HUMI;
	sendBuf->param[7].channelUnit = UNIT_HUMI;
	sendBuf->param[7].dataFormat = FORMAT_ONE_DECIMAL;

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
void GPRS_SendDate(uint8_t *pData, uint16_t Size)
{
	HAL_UART_Transmit_DMA(&GPRS_UART, pData, Size);
}

/*******************************************************************************
 *
 */
void GPRS_SendCmd(char* str)
{
	GPRS_SendDate((uint8_t*)str, strlen(str));
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
