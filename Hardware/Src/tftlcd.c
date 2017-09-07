#include "tftlcd.h"

#include "file.h"

#include "osConfig.h"
#include "TFTLCDProcess.h"

/******************************************************************************/
TFTLCD_SendBufferTypedef TFTLCD_SendBuffer;
TFTLCD_RecvBufferTypedef TFTLCD_RecvBuffer;
uint8_t TFTLCD_RecvBuf[TFTLCD_UART_RX_DATA_SIZE_MAX];

TFTLCD_StatusTypedef TFTLCD_status;

/******************************************************************************/
static void TFTLCD_StructInit(void);
static void TFTLCD_UartInit(void);
static void TFTLCD_Analog2ASCII(uint16_t typeID, float analog, AnalogTypedef* batch);
static void TFTLCD_SendBuf(uint8_t size);
static void TFTLCD_ScreenStart(void);

/*******************************************************************************
 * function:触摸屏初始化
 */
void TFTLCD_Init(void)
{
	/* 发送结构初始化 */
	TFTLCD_StructInit();

	/* 触摸屏接收串口初始化 */
	TFTLCD_UartInit();

	/* 开机界面跳转 */
	TFTLCD_ScreenStart();
}

/*******************************************************************************
 * function:设置界面ID（跳转界面）
 */
void TFTLCD_SetScreenId(TFTLCD_ScreenIDEnum screen)
{
	/* 进入临界区 */
	taskENTER_CRITICAL();

	/* 切换界面 */
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_SET_SCREEN;
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(screen);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(screen);

	memcpy(&TFTLCD_SendBuffer.buffer.data, &TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(9);

	/* 退出临界区 */
	taskEXIT_CRITICAL();

	/* 界面跳转,并且更新状态栏 */
	TFTLCD_status.curScreenID = screen;
	osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_STATUS_BAR_UPDATE);
}

/*******************************************************************************
 * function：模拟量更新
 */
void TFTLCD_AnalogDataRefresh(ANALOG_ValueTypedef* analog)
{
	/* 进入临界区 */
	taskENTER_CRITICAL();

	/* 批量更新命令 */
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_BATCH_UPDATE;

	/* 界面ID */
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(SCREEN_ID_CUR_DATA_8CH);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(SCREEN_ID_CUR_DATA_8CH);

	/* 模拟量更新 */
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH1, analog->temp1,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[0]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH2, analog->temp2,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[1]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH3, analog->temp3,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[2]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH4, analog->temp4,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[3]);

	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH5, analog->humi1,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[4]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH6, analog->humi2,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[5]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH7, analog->humi3,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[6]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH8, analog->humi4,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[7]);

	/* 不知道为什么，这个字节总是被清零 */
	/* 强制转为FF */
	/* todo */
	TFTLCD_SendBuffer.tail[0] = 0xFF;

	memcpy(&TFTLCD_SendBuffer.buffer.data[sizeof(AnalogTypedef) * 8],
				TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(sizeof(AnalogTypedef) * 8 + 9);

	/* 退出临界区 */
	taskEXIT_CRITICAL();
}

/*******************************************************************************
 * function：状态栏更新，更新内容：实时时间、电池电量
 * screenID：界面ID
 * RT_TimeTypedef:时间指针
 * batQuantity：电池电量
 */
void TFTLCD_StatusBarTextRefresh(uint16_t screenID, RT_TimeTypedef* rt, uint8_t batQuantity)
{
	/* 进入临界区 */
	taskENTER_CRITICAL();

	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_BATCH_UPDATE;

	/* 界面ID */
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(screenID);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(screenID);

	TFTLCD_SendBuffer.buffer.batch.statusBarText.timeCtlIdH =
			HalfWord_GetHighByte(CTL_ID_REALTIME);
	TFTLCD_SendBuffer.buffer.batch.statusBarText.timeCtlIdL =
			HalfWord_GetLowByte(CTL_ID_REALTIME);

	/* 时间长度是16个字符 */
	TFTLCD_SendBuffer.buffer.batch.statusBarText.timeSizeH = 0;
	TFTLCD_SendBuffer.buffer.batch.statusBarText.timeSizeL = 16;

	/* 在年的前面添加上“20” */
	TFTLCD_SendBuffer.buffer.batch.statusBarText.year[0] = '2';
	TFTLCD_SendBuffer.buffer.batch.statusBarText.year[1] = '0';
	HEX2ASCII(&rt->date.Year, (uint8_t*)&TFTLCD_SendBuffer.buffer.batch.statusBarText.year[2], 1);
	TFTLCD_SendBuffer.buffer.batch.statusBarText.str1 = '.';
	HEX2ASCII(&rt->date.Month, (uint8_t*)&TFTLCD_SendBuffer.buffer.batch.statusBarText.month[0], 1);
	TFTLCD_SendBuffer.buffer.batch.statusBarText.str2 = '.';
	HEX2ASCII(&rt->date.Date, (uint8_t*)&TFTLCD_SendBuffer.buffer.batch.statusBarText.day[0], 1);
	TFTLCD_SendBuffer.buffer.batch.statusBarText.str3 = ' ';
	HEX2ASCII(&rt->time.Hours, (uint8_t*)&TFTLCD_SendBuffer.buffer.batch.statusBarText.hour[0], 1);
	TFTLCD_SendBuffer.buffer.batch.statusBarText.str4 = ':';
	HEX2ASCII(&rt->time.Minutes, (uint8_t*)&TFTLCD_SendBuffer.buffer.batch.statusBarText.min[0], 1);

	TFTLCD_SendBuffer.buffer.batch.statusBarText.batCtlIdH =
			HalfWord_GetHighByte(CTL_ID_BAT_QUANTITY_PERCENT);
	TFTLCD_SendBuffer.buffer.batch.statusBarText.batCtlIdL =
			HalfWord_GetLowByte(CTL_ID_BAT_QUANTITY_PERCENT);

	TFTLCD_SendBuffer.buffer.batch.statusBarText.batSizeH = 0;
	TFTLCD_SendBuffer.buffer.batch.statusBarText.batSizeL = 3;

	sprintf(TFTLCD_SendBuffer.buffer.batch.statusBarText.batCapacity,
			"%3d", batQuantity);

	memcpy(&TFTLCD_SendBuffer.buffer.data[sizeof(StatusBarTextTypedef)],
			TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(sizeof(StatusBarTextTypedef) + 11);

	/* 退出临界区 */
	taskEXIT_CRITICAL();
}

/*******************************************************************************
 * function：状态栏更新，更新内容：电池电量图标、信号强度图标、报警图标
 * screenID：界面ID
 * RT_TimeTypedef:时间指针
 * batQuantity：电池电量
 */
void TFTLCD_StatusBarIconRefresh(uint16_t screenID)
{
	/* 进入临界区 */
	taskENTER_CRITICAL();

	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_BATCH_UPDATE;

	/* 界面ID */
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(screenID);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(screenID);

	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.batCtlIdH = HalfWord_GetHighByte(CTL_ID_BAT_QUANTITY);
	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.batCtlIdL = HalfWord_GetLowByte(CTL_ID_BAT_QUANTITY);
	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.batSizeH = 0;
	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.batSizeL = 2;
	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.batCapacityH = 0;
	if (INPUT_CheckPwrOnStatus())
	{
		TFTLCD_SendBuffer.buffer.batch.statusBarIcon.batCapacityL = ICON_BAT_CHARGE;
	}
	else if (ANALOG_value.batVoltage >= 80)
	{
		TFTLCD_SendBuffer.buffer.batch.statusBarIcon.batCapacityL = ICON_BAT_CAPACITY_80;
	}
	else if (ANALOG_value.batVoltage >= 60)
	{
		TFTLCD_SendBuffer.buffer.batch.statusBarIcon.batCapacityL = ICON_BAT_CAPACITY_60;
	}
	else if (ANALOG_value.batVoltage >= 40)
	{
		TFTLCD_SendBuffer.buffer.batch.statusBarIcon.batCapacityL = ICON_BAT_CAPACITY_40;
	}
	else if (ANALOG_value.batVoltage >= 20)
	{
		TFTLCD_SendBuffer.buffer.batch.statusBarIcon.batCapacityL = ICON_BAT_CAPACITY_20;
	}
	else
	{
		TFTLCD_SendBuffer.buffer.batch.statusBarIcon.batCapacityL = ICON_BAT_CAPACITY_0;
	}

	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.signalCtlIdH = HALFWORD_BYTE_H(CTL_ID_SIGNAL_QUALITY);
	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.signalCtlIdL = HALFWORD_BYTE_L(CTL_ID_SIGNAL_QUALITY);
	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.signalSizeH = 0;
	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.signalSizeL = 2;
	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.signalCapacityH = 0;
	if (GPRS_signalQuality >= 21)
	{
		TFTLCD_SendBuffer.buffer.batch.statusBarIcon.signalCapacityL = ICON_SIGNAL_QUALITY_31_21;
	}
	else if (GPRS_signalQuality >= 11)
	{
		TFTLCD_SendBuffer.buffer.batch.statusBarIcon.signalCapacityL = ICON_SIGNAL_QUALITY_21_11;
	}
	else
	{
		TFTLCD_SendBuffer.buffer.batch.statusBarIcon.signalCapacityL = ICON_SIGNAL_QUALITY_11_0;
	}

	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.alarmCtlIdH = HALFWORD_BYTE_H(CTL_ID_ALARM_ICON);
	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.alarmCtlIdL = HALFWORD_BYTE_L(CTL_ID_ALARM_ICON);
	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.alarmSizeH = 0;
	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.alarmSizeL = 2;
	TFTLCD_SendBuffer.buffer.batch.statusBarIcon.alarmCapacityH = 0;
	if (ANALOG_alarmStatus.status.all != 0)
	{
		TFTLCD_SendBuffer.buffer.batch.statusBarIcon.alarmCapacityL = ICON_ALARM_ON;
	}
	else
	{
		TFTLCD_SendBuffer.buffer.batch.statusBarIcon.alarmCapacityL = ICON_ALARM_OFF;
	}

	memcpy(&TFTLCD_SendBuffer.buffer.data[sizeof(StatusBarIconTypedef)],
			TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(sizeof(StatusBarIconTypedef) + 11);

	/* 退出临界区 */
	taskEXIT_CRITICAL();
}

/*******************************************************************************
 * function:将读出来的数据转换格式后输出到历史界面
 * @saveInfo：读出的数据
 * @typeID：更新的控件
 */
void TFTLCD_HistoryDataFormat(FILE_SaveInfoTypedef* saveInfo, TFTLCD_HisDataCtlIdEnum typeID)
{
	/* 进入临界区 */
	taskENTER_CRITICAL();

	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_TEXT_UPDATE;

	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(SCREEN_ID_HIS_DATA);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(SCREEN_ID_HIS_DATA);

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HalfWord_GetHighByte(typeID);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HalfWord_GetLowByte(typeID);

	/* 历史数据显示的格式为：
	 * [0][1][2][3][4][5][6][7][8][9][10][11][12][13][14][15][16][17][18]
	 * |     |     |     |  |     |  |       |   |                  |   |
	 *    年           月          日       空格     时         ：       分          空格                     通道1                     空格
	 *
	 *
	 * [19][20][21][22][23][24][25][26][27][28][29][30][31][32][33][34][35]
	 * |                   |   |                   |   |                  |
	 *       通道2                           空格                 通道3                           空格                   通道4
	 *
	 * [36][37][38][39][40][41][42][43][44][45][46][47][48][49][50][51][52]
	 * |                   |   |                   |   |                  |
	 *         通道5                      空格                         通道6                   空格                     通道7
	 *
	 * [53][54][55][56][57][58]
	 * |   |                  |
	 *  空格          通道8
	 */

	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[0], saveInfo->year,  9);
	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[10], saveInfo->min,  2);
	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[13], saveInfo->analogValue[0].value, 23);
	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[36], saveInfo->analogValue[4].value, 23);
	TFTLCD_SendBuffer.buffer.update.value.date[9]  = ':';
	TFTLCD_SendBuffer.buffer.update.value.date[12] = ' ';
	TFTLCD_SendBuffer.buffer.update.value.date[18] = ' ';
	TFTLCD_SendBuffer.buffer.update.value.date[24] = ' ';
	TFTLCD_SendBuffer.buffer.update.value.date[30] = ' ';
	TFTLCD_SendBuffer.buffer.update.value.date[41] = ' ';
	TFTLCD_SendBuffer.buffer.update.value.date[47] = ' ';
	TFTLCD_SendBuffer.buffer.update.value.date[53] = ' ';

	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[59],
				TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(70);

	/* 退出临界区 */
	taskEXIT_CRITICAL();
}

/*******************************************************************************
 * function:打印通道选择图标显示
 * @ctrl：通道选择控件编号
 * @status：当前该通道的值，若选中则变成不选，反之亦然。
 */
void TFTLCD_ChannelSelectICON(TFTLCD_ScreenIDEnum screen, uint16_t typeID, uint8_t status)
{
	/* 进入临界区 */
	taskENTER_CRITICAL();

	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_ICON_DISP;

	/* 界面ID */
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(screen);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(screen);

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HalfWord_GetHighByte(typeID);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HalfWord_GetLowByte(typeID);

	TFTLCD_SendBuffer.buffer.update.value.date[0] = status;

	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[1],
			TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(12);

	/* 退出临界区 */
	taskEXIT_CRITICAL();
}

/*******************************************************************************
 * function:将时间选择界面选好的数值更新到指定的时间控件
 * @time：选好的时间
 */
void TFTLCD_SelectTimeUpdate(TFTLCD_ScreenIDEnum screen, uint16_t ctlID, FILE_RealTimeTypedef* time)
{
	/* 进入临界区 */
	taskENTER_CRITICAL();

	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_TEXT_UPDATE;

	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(screen);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(screen);

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HalfWord_GetHighByte(ctlID);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HalfWord_GetLowByte(ctlID);

	HEX2ASCII(&time->year, (uint8_t*)TFTLCD_SendBuffer.buffer.update.value.date, 3);
	TFTLCD_SendBuffer.buffer.update.value.date[6] = ' ';
	HEX2ASCII(&time->hour, (uint8_t*)&TFTLCD_SendBuffer.buffer.update.value.date[7], 1);
	TFTLCD_SendBuffer.buffer.update.value.date[9] = ':';
	HEX2ASCII(&time->min, (uint8_t*)&TFTLCD_SendBuffer.buffer.update.value.date[10], 1);

	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[12],
				TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(23);

	/* 退出临界区 */
	taskEXIT_CRITICAL();
}

#if 0
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

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HalfWord_GetHighByte(ctrl);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HalfWord_GetLowByte(ctrl);

	/* 在年的前面添加上“20” */
	TFTLCD_SendBuffer.buffer.update.value.time.year[0] = '2';
	TFTLCD_SendBuffer.buffer.update.value.time.year[1] = '0';
	sprintf(&TFTLCD_SendBuffer.buffer.update.value.time.year[2], "%2d", rt->year);
	TFTLCD_SendBuffer.buffer.update.value.time.str1 = '.';
	sprintf(&TFTLCD_SendBuffer.buffer.update.value.time.month[0], "%2d", rt->month);
	TFTLCD_SendBuffer.buffer.update.value.time.str2 = '.';
	sprintf(&TFTLCD_SendBuffer.buffer.update.value.time.day[0], "%2d", rt->day);
	TFTLCD_SendBuffer.buffer.update.value.time.str3 = ' ';
	sprintf(&TFTLCD_SendBuffer.buffer.update.value.time.hour[0], "%2d", rt->hour);
	TFTLCD_SendBuffer.buffer.update.value.time.str4 = ':';
	sprintf(&TFTLCD_SendBuffer.buffer.update.value.time.min[0], "%2d", rt->min);

	/* 长度少了“：05”，秒位 */
	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[sizeof(TFTLCD_TimeUpdateTypedef) - 3],
			TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(sizeof(TFTLCD_TimeUpdateTypedef) + 11);
}
#endif



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
static void TFTLCD_SendBuf(uint8_t size)
{
//	HAL_UART_Transmit_DMA(&TFTLCD_UART, (uint8_t*)&TFTLCD_SendBuffer.head, size);
	HAL_UART_Transmit(&TFTLCD_UART, (uint8_t*)&TFTLCD_SendBuffer.head, size, 1000);
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
static void TFTLCD_Analog2ASCII(uint16_t typeID, float analog, AnalogTypedef* batch)
{
	uint16_t size;

	batch->ctrlIdH = HalfWord_GetHighByte(typeID);
	batch->ctrlIdL = HalfWord_GetLowByte(typeID);
	/* %5.1表示有效数据长度为5，小数1位 */
	size = sprintf((char*)&batch->value[0], "%5.1f", analog);
	batch->sizeH = HalfWord_GetHighByte(size);
	batch->sizeL = HalfWord_GetLowByte(size);
}

/*******************************************************************************
 *
 */
static void TFTLCD_ScreenStart(void)
{
	osDelay(3000);

	TFTLCD_SetScreenId(SCREEN_ID_CUR_DATA_8CH);
}









