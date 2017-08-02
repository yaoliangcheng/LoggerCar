#include "tftlcd.h"

TFTLCD_SendBufferTypedef TFTLCD_SendBuffer;
TFTLCD_BufferStatusTypedef TFTLCD_BufferStatus;
uint8_t TFTLCD_RecvBuffer[TFTLCD_UART_RX_DATA_SIZE_MAX];

/******************************************************************************/
static void TFTLCD_StructInit(void);
static void TFTLCD_UartInit(void);
static void TFTLCD_Analog2ASCII(uint16_t typeID, float analog, BatchUpdateTypedef* batch);
static void TFTLCD_SendBuf(uint8_t* pBuffer, uint8_t size);

/*******************************************************************************
 *
 */
void TFTLCD_Init(void)
{
	TFTLCD_StructInit();
	TFTLCD_UartInit();
}

/*******************************************************************************
 *
 */
void TFTLCD_AnalogDataRefresh(ANALOG_ValueTypedef* analog)
{
	/* 批量更新命令 */
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_BATCH_UPDATE;

	/* 界面ID */
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(SCREEN_ID_CUR_DATA);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(SCREEN_ID_CUR_DATA);

	/* 批量更新内容 */
	TFTLCD_Analog2ASCII(CTRL_TYPE_ID_TEMP1, analog->temp1, &TFTLCD_SendBuffer.buf.batchDate[0]);
	TFTLCD_Analog2ASCII(CTRL_TYPE_ID_TEMP2, analog->temp2, &TFTLCD_SendBuffer.buf.batchDate[1]);
	TFTLCD_Analog2ASCII(CTRL_TYPE_ID_TEMP3, analog->temp3, &TFTLCD_SendBuffer.buf.batchDate[2]);
	TFTLCD_Analog2ASCII(CTRL_TYPE_ID_TEMP4, analog->temp4, &TFTLCD_SendBuffer.buf.batchDate[3]);

	TFTLCD_Analog2ASCII(CTRL_TYPE_ID_HUMI1, analog->humi1, &TFTLCD_SendBuffer.buf.batchDate[4]);
	TFTLCD_Analog2ASCII(CTRL_TYPE_ID_HUMI2, analog->humi2, &TFTLCD_SendBuffer.buf.batchDate[5]);
	TFTLCD_Analog2ASCII(CTRL_TYPE_ID_HUMI3, analog->humi3, &TFTLCD_SendBuffer.buf.batchDate[6]);
	TFTLCD_Analog2ASCII(CTRL_TYPE_ID_HUMI4, analog->humi4, &TFTLCD_SendBuffer.buf.batchDate[7]);

	/* 不知道为什么，这个字节总是被清零 */
	/* 强制转为FF */
	/* todo */
	TFTLCD_SendBuffer.tail[0] = 0xFF;

	TFTLCD_SendBuf((uint8_t*)&TFTLCD_SendBuffer.head, sizeof(TFTLCD_SendBufferTypedef));
}

/*******************************************************************************
 *
 */
void TFTLCD_RealtimeRefresh(RT_TimeTypedef* rt)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_TIME_UPDATE;

	/* 界面ID */
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(SCREEN_ID_CUR_DATA);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(SCREEN_ID_CUR_DATA);

	TFTLCD_SendBuffer.buf.data.ctrlIdH = HalfWord_GetHighByte(CTRL_TYPE_ID_REAL_TIME);
	TFTLCD_SendBuffer.buf.data.ctrlIdL = HalfWord_GetLowByte(CTRL_TYPE_ID_REAL_TIME);

//	sprintf(&(TFTLCD_SendBuffer.buf.data.value.time.year), "%4d", rt->date.Year + 2000);

	/* 在年的前面添加上“20” */
	TFTLCD_SendBuffer.buf.data.value.time.year[0] = '2';
	TFTLCD_SendBuffer.buf.data.value.time.year[1] = '0';
	BCD2ASCII(&TFTLCD_SendBuffer.buf.data.value.time.year[2], &rt->date.Year, 1);
	TFTLCD_SendBuffer.buf.data.value.time.str1 = '.';
	BCD2ASCII(&TFTLCD_SendBuffer.buf.data.value.time.month[0], &rt->date.Month, 1);
	TFTLCD_SendBuffer.buf.data.value.time.str2 = '.';
	BCD2ASCII(&TFTLCD_SendBuffer.buf.data.value.time.day[0], &rt->date.Date, 1);
	TFTLCD_SendBuffer.buf.data.value.time.str3 = ' ';
	BCD2ASCII(&TFTLCD_SendBuffer.buf.data.value.time.hour[0], &rt->time.Hours, 1);
	TFTLCD_SendBuffer.buf.data.value.time.str4 = ':';
	BCD2ASCII(&TFTLCD_SendBuffer.buf.data.value.time.min[0], &rt->time.Minutes, 1);
	TFTLCD_SendBuffer.buf.data.value.time.str5 = ':';
	BCD2ASCII(&TFTLCD_SendBuffer.buf.data.value.time.sec[0], &rt->time.Seconds, 1);

	memcpy(&TFTLCD_SendBuffer.buf.data.value.date[sizeof(TFTLCD_TimeUpdateTypedef)],
			TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf((uint8_t*)&TFTLCD_SendBuffer.head, sizeof(TFTLCD_TimeUpdateTypedef) + 11);
}

/*******************************************************************************
 * Uart接收中断函数
 */
void TFTLCD_UartIdleDeal(void)
{
	uint32_t tmp_flag = 0, tmp_it_source = 0;

	tmp_flag = __HAL_UART_GET_FLAG(&TFTLCD_UART, UART_FLAG_IDLE);
	tmp_it_source = __HAL_UART_GET_IT_SOURCE(&TFTLCD_UART, UART_IT_IDLE);
	if((tmp_flag != RESET) && (tmp_it_source != RESET))
	{
		__HAL_DMA_DISABLE(TFTLCD_UART.hdmarx);
		__HAL_DMA_CLEAR_FLAG(TFTLCD_UART.hdmarx, TFTLCD_UART_DMA_FLAG_GL);

		/* Clear Uart IDLE Flag */
		__HAL_UART_CLEAR_IDLEFLAG(&TFTLCD_UART);

		TFTLCD_BufferStatus.bufferSize = TFTLCD_UART_RX_DATA_SIZE_MAX
						- __HAL_DMA_GET_COUNTER(TFTLCD_UART.hdmarx);

		memcpy(TFTLCD_BufferStatus.recvBuffer, TFTLCD_RecvBuffer, TFTLCD_BufferStatus.bufferSize);
		memset(TFTLCD_RecvBuffer, 0, TFTLCD_BufferStatus.bufferSize);

//		osSignalSet(gprsprocessTaskHandle, GPRS_PROCESS_TASK_RECV_ENABLE);

		TFTLCD_UART.hdmarx->Instance->CNDTR = TFTLCD_UART.RxXferSize;
		__HAL_DMA_ENABLE(TFTLCD_UART.hdmarx);
	}
}

/*******************************************************************************
 *
 */
static void TFTLCD_StructInit(void)
{
	TFTLCD_SendBuffer.head = TFTLCD_CMD_HEAD;

	TFTLCD_SendBuffer.tail[0] = TFTLCD_CMD_TAIL1;
	TFTLCD_SendBuffer.tail[1] = TFTLCD_CMD_TAIL2;
	TFTLCD_SendBuffer.tail[2] = TFTLCD_CMD_TAIL3;
	TFTLCD_SendBuffer.tail[3] = TFTLCD_CMD_TAIL4;
}

/*******************************************************************************
 *
 */
static void TFTLCD_UartInit(void)
{
	UART_DMAIdleConfig(&TFTLCD_UART, TFTLCD_RecvBuffer, TFTLCD_UART_RX_DATA_SIZE_MAX);
}

/*******************************************************************************
 *
 */
static void TFTLCD_Analog2ASCII(uint16_t typeID, float analog, BatchUpdateTypedef* batch)
{
	uint16_t size;

	batch->ctrlIdH = HalfWord_GetHighByte(typeID);
	batch->ctrlIdL = HalfWord_GetLowByte(typeID);
	/* %4.1表示有效数据长度为4，小数1位 */
	size = sprintf((char*)&batch->value[0], "%5.1f", analog);
	batch->sizeH = HalfWord_GetHighByte(size);
	batch->sizeL = HalfWord_GetLowByte(size);
}

/*******************************************************************************
 *
 */
static void TFTLCD_SendBuf(uint8_t* pBuffer, uint8_t size)
{
	HAL_UART_Transmit_DMA(&TFTLCD_UART, pBuffer, size);
}















