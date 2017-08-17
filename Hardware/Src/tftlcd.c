#include "tftlcd.h"

#include "file.h"

#include "osConfig.h"
#include "TFTLCDProcess.h"

TFTLCD_SendBufferTypedef TFTLCD_SendBuffer;
TFTLCD_RecvBufferTypedef TFTLCD_RecvBuffer;
uint8_t TFTLCD_RecvBuf[TFTLCD_UART_RX_DATA_SIZE_MAX];

/******************************************************************************/
static void TFTLCD_StructInit(void);
static void TFTLCD_UartInit(void);
static void TFTLCD_Analog2ASCII(uint16_t typeID, float analog, BatchUpdateTypedef* batch);
static void TFTLCD_SendBuf(uint8_t* pBuffer, uint8_t size);
static void TFTLCD_ScreenStart(void);

/*******************************************************************************
 *
 */
void TFTLCD_Init(void)
{
	TFTLCD_StructInit();
	TFTLCD_UartInit();
	TFTLCD_ScreenStart();
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
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_TEXT_UPDATE;

	/* 界面ID */
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(SCREEN_ID_CUR_DATA);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(SCREEN_ID_CUR_DATA);

	TFTLCD_SendBuffer.buf.data.ctrlIdH = HalfWord_GetHighByte(CTRL_TYPE_ID_REAL_TIME);
	TFTLCD_SendBuffer.buf.data.ctrlIdL = HalfWord_GetLowByte(CTRL_TYPE_ID_REAL_TIME);

//	sprintf(&(TFTLCD_SendBuffer.buf.data.value.time.year), "%4d", rt->date.Year + 2000);

	/* 在年的前面添加上“20” */
	TFTLCD_SendBuffer.buf.data.value.time.year[0] = '2';
	TFTLCD_SendBuffer.buf.data.value.time.year[1] = '0';
	HEX2ASCII(&rt->date.Year, (uint8_t*)&TFTLCD_SendBuffer.buf.data.value.time.year[2], 1);
	TFTLCD_SendBuffer.buf.data.value.time.str1 = '.';
	HEX2ASCII(&rt->date.Month, (uint8_t*)&TFTLCD_SendBuffer.buf.data.value.time.month[0], 1);
	TFTLCD_SendBuffer.buf.data.value.time.str2 = '.';
	HEX2ASCII(&rt->date.Date, (uint8_t*)&TFTLCD_SendBuffer.buf.data.value.time.day[0], 1);
	TFTLCD_SendBuffer.buf.data.value.time.str3 = ' ';
	HEX2ASCII(&rt->time.Hours, (uint8_t*)&TFTLCD_SendBuffer.buf.data.value.time.hour[0], 1);
	TFTLCD_SendBuffer.buf.data.value.time.str4 = ':';
	HEX2ASCII(&rt->time.Minutes, (uint8_t*)&TFTLCD_SendBuffer.buf.data.value.time.min[0], 1);
	TFTLCD_SendBuffer.buf.data.value.time.str5 = ':';
	HEX2ASCII(&rt->time.Seconds, (uint8_t*)&TFTLCD_SendBuffer.buf.data.value.time.sec[0], 1);

	memcpy(&TFTLCD_SendBuffer.buf.data.value.date[sizeof(TFTLCD_TimeUpdateTypedef)],
			TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf((uint8_t*)&TFTLCD_SendBuffer.head, sizeof(TFTLCD_TimeUpdateTypedef) + 11);
}

/*******************************************************************************
 * function:打印时间更新
 */
void TFTLCD_printTimeUpdate(FILE_RealTime* rt, CtrlID_PrintEnum ctrl)
{
	if ((ctrl != PRINT_CTRL_ID_START_TIME) && (ctrl != PRINT_CTRL_ID_END_TIME))
		return;

	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_TEXT_UPDATE;

	/* 界面ID */
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(SCREEN_ID_PRINT);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(SCREEN_ID_PRINT);

	TFTLCD_SendBuffer.buf.data.ctrlIdH = HalfWord_GetHighByte(ctrl);
	TFTLCD_SendBuffer.buf.data.ctrlIdL = HalfWord_GetLowByte(ctrl);

	/* 在年的前面添加上“20” */
	TFTLCD_SendBuffer.buf.data.value.time.year[0] = '2';
	TFTLCD_SendBuffer.buf.data.value.time.year[1] = '0';
	sprintf(&TFTLCD_SendBuffer.buf.data.value.time.year[2], "%2d", rt->year);
	TFTLCD_SendBuffer.buf.data.value.time.str1 = '.';
	sprintf(&TFTLCD_SendBuffer.buf.data.value.time.month[0], "%2d", rt->month);
	TFTLCD_SendBuffer.buf.data.value.time.str2 = '.';
	sprintf(&TFTLCD_SendBuffer.buf.data.value.time.day[0], "%2d", rt->day);
	TFTLCD_SendBuffer.buf.data.value.time.str3 = ' ';
	sprintf(&TFTLCD_SendBuffer.buf.data.value.time.hour[0], "%2d", rt->hour);
	TFTLCD_SendBuffer.buf.data.value.time.str4 = ':';
	sprintf(&TFTLCD_SendBuffer.buf.data.value.time.min[0], "%2d", rt->min);

	/* 长度少了“：05”，秒位 */
	memcpy(&TFTLCD_SendBuffer.buf.data.value.date[sizeof(TFTLCD_TimeUpdateTypedef) - 3],
			TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf((uint8_t*)&TFTLCD_SendBuffer.head, sizeof(TFTLCD_TimeUpdateTypedef) + 11);
}

/*******************************************************************************
 * function:打印通道选择图标显示
 * @ctrl：通道选择控件编号
 * @status：当前该通道的值，若选中则变成不选，反之亦然。
 */
void TFTLCD_printChannelSelectICON(CtrlID_PrintEnum ctrl, uint8_t status)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_ICON_DISP;

	/* 界面ID */
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(SCREEN_ID_PRINT);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(SCREEN_ID_PRINT);

	TFTLCD_SendBuffer.buf.data.ctrlIdH = HalfWord_GetHighByte(ctrl);
	TFTLCD_SendBuffer.buf.data.ctrlIdL = HalfWord_GetLowByte(ctrl);

	if (status)
		TFTLCD_SendBuffer.buf.data.value.date[0] = 0;
	else
		TFTLCD_SendBuffer.buf.data.value.date[0] = 1;

	memcpy(&TFTLCD_SendBuffer.buf.data.value.date[1],
			TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf((uint8_t*)&TFTLCD_SendBuffer.head, 12);
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

		TFTLCD_RecvBuffer.bufferSize = TFTLCD_UART_RX_DATA_SIZE_MAX
						- __HAL_DMA_GET_COUNTER(TFTLCD_UART.hdmarx);

		memcpy(&TFTLCD_RecvBuffer.date.buf[0], TFTLCD_RecvBuf, TFTLCD_RecvBuffer.bufferSize);
		memset(TFTLCD_RecvBuf, 0, TFTLCD_RecvBuffer.bufferSize);

		osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_RECV_ENABLE);

		TFTLCD_UART.hdmarx->Instance->CNDTR = TFTLCD_UART.RxXferSize;
		__HAL_DMA_ENABLE(TFTLCD_UART.hdmarx);
	}
}

/*******************************************************************************
 *
 */
ErrorStatus TFTLCD_CheckHeadTail(void)
{
	if (TFTLCD_RecvBuffer.date.recvBuf.head == TFTLCD_CMD_HEAD)
	{
		if ((TFTLCD_RecvBuffer.date.buf[TFTLCD_RecvBuffer.bufferSize - 4] == TFTLCD_CMD_TAIL1)
			&& (TFTLCD_RecvBuffer.date.buf[TFTLCD_RecvBuffer.bufferSize - 3] == TFTLCD_CMD_TAIL2)
			&& (TFTLCD_RecvBuffer.date.buf[TFTLCD_RecvBuffer.bufferSize - 2] == TFTLCD_CMD_TAIL3)
			&& (TFTLCD_RecvBuffer.date.buf[TFTLCD_RecvBuffer.bufferSize - 1] == TFTLCD_CMD_TAIL4))
		{
			return SUCCESS;
		}
		else
			return ERROR;
	}
	else
		return ERROR;
}



/*******************************************************************************
 *
 */
static void TFTLCD_SendBuf(uint8_t* pBuffer, uint8_t size)
{
	HAL_UART_Transmit_DMA(&TFTLCD_UART, pBuffer, size);
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
	UART_DMAIdleConfig(&TFTLCD_UART, TFTLCD_RecvBuf, TFTLCD_UART_RX_DATA_SIZE_MAX);
}

/*******************************************************************************
 *
 */
static void TFTLCD_Analog2ASCII(uint16_t typeID, float analog, BatchUpdateTypedef* batch)
{
	uint16_t size;

	batch->ctrlIdH = HalfWord_GetHighByte(typeID);
	batch->ctrlIdL = HalfWord_GetLowByte(typeID);
	/* %4.1表示有效数据长度为5，小数1位 */
	size = sprintf((char*)&batch->value[0], "%5.1f", analog);
	batch->sizeH = HalfWord_GetHighByte(size);
	batch->sizeL = HalfWord_GetLowByte(size);
}

/*******************************************************************************
 *
 */
static void TFTLCD_SetScreenID(uint16_t id)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_SET_SCREEN;
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(id);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(id);

	memcpy(&TFTLCD_SendBuffer.buf.data.value.date[0], &TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf((uint8_t*)&TFTLCD_SendBuffer.head, 9);
}

/*******************************************************************************
 *
 */
static void TFTLCD_ScreenStart(void)
{
	osDelay(3000);

	TFTLCD_SetScreenID(SCREEN_ID_CUR_DATA);
}









