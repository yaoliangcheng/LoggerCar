#include "tftlcd.h"

#include "file.h"
#include "param.h"
#include "TFTLCDProcess.h"

/******************************************************************************/
TFTLCD_SendBufferTypedef TFTLCD_SendBuffer;
TFTLCD_RecvBufferTypedef TFTLCD_RecvBuffer;
uint8_t TFTLCD_RecvBuf[TFTLCD_UART_RX_DATA_SIZE_MAX];

TFTLCD_StatusTypedef TFTLCD_status;

/******************************************************************************/
extern ANALOG_ModeEnum ANALOG_Mode;
extern osThreadId tftlcdTaskHandle;
extern uint8_t  GPRS_signalQuality;
/******************************************************************************/
static void TFTLCD_StructInit(void);
static void TFTLCD_UartInit(void);
static void TFTLCD_Analog2ASCII(uint16_t typeID, float analog, AnalogTypedef* batch);
static void TFTLCD_SendBuf(uint8_t size);
static void TFTLCD_ScreenStart(void);
static void TFTLCD_HistoryDataCurveDisplay(uint8_t channel, uint8_t data);
static void AnalogAlarmDisplay(uint16_t ctlID, float analog, ParamAlarmTypedef* param);
static void FloatValueUpdate(uint16_t screenID, uint16_t ctlID, float data);
static void TFTLCD_DisplayInit(void);

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

	/* 更新界面一些默认参数 */
	TFTLCD_DisplayInit();
}

/*******************************************************************************
 * function:设置界面ID（跳转界面）
 */
void TFTLCD_SetScreenId(TFTLCD_ScreenIDEnum screen)
{
	/* 界面跳转,并且更新状态栏 */
	TFTLCD_status.curScreenID = screen;

	taskENTER_CRITICAL();

	/* 切换界面 */
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_SET_SCREEN;
	TFTLCD_SendBuffer.screenIdH = HALFWORD_BYTE_H(screen);
	TFTLCD_SendBuffer.screenIdL = HALFWORD_BYTE_L(screen);
	memcpy(&TFTLCD_SendBuffer.buffer.data, &TFTLCD_SendBuffer.tail, 4);
	TFTLCD_SendBuf(9);

	taskEXIT_CRITICAL();

	osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_STATUS_BAR_UPDATE);
}

/*******************************************************************************
 * function:设置文本控件的值
 */
void TFTLCD_TextValueUpdate(uint16_t screenID, uint16_t ctlID, char* str, uint8_t size)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_TEXT_UPDATE;

	TFTLCD_SendBuffer.screenIdH = HALFWORD_BYTE_H(screenID);
	TFTLCD_SendBuffer.screenIdL = HALFWORD_BYTE_L(screenID);

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HALFWORD_BYTE_H(ctlID);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HALFWORD_BYTE_L(ctlID);

	memcpy(TFTLCD_SendBuffer.buffer.update.value.date, str, size);

	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[size],
				TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(11 + size);
}

/*******************************************************************************
 * function：模拟量更新
 */
void TFTLCD_AnalogDataRefresh(ANALOG_ValueTypedef* analog)
{
	/* 批量更新命令 */
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_BATCH_UPDATE;

	/* 界面ID */
	TFTLCD_SendBuffer.screenIdH = HALFWORD_BYTE_H(SCREEN_ID_CUR_DATA_8CH);
	TFTLCD_SendBuffer.screenIdL = HALFWORD_BYTE_L(SCREEN_ID_CUR_DATA_8CH);

	/* 模拟量更新 */
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH1, analog->channel1,
			&TFTLCD_SendBuffer.buffer.analogValue[0]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH2, analog->channel2,
			&TFTLCD_SendBuffer.buffer.analogValue[1]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH3, analog->channel3,
			&TFTLCD_SendBuffer.buffer.analogValue[2]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH4, analog->channel4,
			&TFTLCD_SendBuffer.buffer.analogValue[3]);

	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH5, analog->channel5,
			&TFTLCD_SendBuffer.buffer.analogValue[4]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH6, analog->channel6,
			&TFTLCD_SendBuffer.buffer.analogValue[5]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH7, analog->channel7,
			&TFTLCD_SendBuffer.buffer.analogValue[6]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH8, analog->channel8,
			&TFTLCD_SendBuffer.buffer.analogValue[7]);

	/* 不知道为什么，这个字节总是被清零 */
	/* 强制转为FF */
	/* todo */
	TFTLCD_SendBuffer.tail[0] = 0xFF;

	memcpy(&TFTLCD_SendBuffer.buffer.data[sizeof(AnalogTypedef) * 8],
				TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(sizeof(AnalogTypedef) * 8 + 9);
}

/*******************************************************************************
 * @brief 判断各个通道的值是否超标
 */
void TFTLCD_AnalogDataAlarmDisplay(ANALOG_ValueTypedef* analog)
{
	AnalogAlarmDisplay(CTL_ID_DATA_CH1, analog->channel1, &PARAM_DeviceParam.chAlarmValue[0]);
	AnalogAlarmDisplay(CTL_ID_DATA_CH5, analog->channel2, &PARAM_DeviceParam.chAlarmValue[1]);
	AnalogAlarmDisplay(CTL_ID_DATA_CH2, analog->channel3, &PARAM_DeviceParam.chAlarmValue[2]);
	AnalogAlarmDisplay(CTL_ID_DATA_CH6, analog->channel4, &PARAM_DeviceParam.chAlarmValue[3]);
	AnalogAlarmDisplay(CTL_ID_DATA_CH3, analog->channel5, &PARAM_DeviceParam.chAlarmValue[4]);
	AnalogAlarmDisplay(CTL_ID_DATA_CH7, analog->channel6, &PARAM_DeviceParam.chAlarmValue[5]);
	AnalogAlarmDisplay(CTL_ID_DATA_CH4, analog->channel7, &PARAM_DeviceParam.chAlarmValue[6]);
	AnalogAlarmDisplay(CTL_ID_DATA_CH8, analog->channel8, &PARAM_DeviceParam.chAlarmValue[7]);
}

/*******************************************************************************
 * function：状态栏更新，更新内容：实时时间、电池电量
 * screenID：界面ID
 * RT_TimeTypedef:时间指针
 * batQuantity：电池电量
 */
void TFTLCD_StatusBarTextRefresh(uint16_t screenID, RT_TimeTypedef* rt, uint8_t batQuantity)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_BATCH_UPDATE;

	/* 界面ID */
	TFTLCD_SendBuffer.screenIdH = HALFWORD_BYTE_H(screenID);
	TFTLCD_SendBuffer.screenIdL = HALFWORD_BYTE_L(screenID);

	TFTLCD_SendBuffer.buffer.statusBarText.timeCtlIdH =
			HALFWORD_BYTE_H(CTL_ID_REALTIME);
	TFTLCD_SendBuffer.buffer.statusBarText.timeCtlIdL =
			HALFWORD_BYTE_L(CTL_ID_REALTIME);

	/* 时间长度是16个字符 */
	TFTLCD_SendBuffer.buffer.statusBarText.timeSizeH = 0;
	TFTLCD_SendBuffer.buffer.statusBarText.timeSizeL = 16;

	/* 在年的前面添加上“20” */
	TFTLCD_SendBuffer.buffer.statusBarText.year[0] = '2';
	TFTLCD_SendBuffer.buffer.statusBarText.year[1] = '0';
	HEX2ASCII(&TFTLCD_SendBuffer.buffer.statusBarText.year[2],  &rt->date.Year,  1);
	TFTLCD_SendBuffer.buffer.statusBarText.str1 = '.';
	HEX2ASCII(&TFTLCD_SendBuffer.buffer.statusBarText.month[0], &rt->date.Month, 1);
	TFTLCD_SendBuffer.buffer.statusBarText.str2 = '.';
	HEX2ASCII(&TFTLCD_SendBuffer.buffer.statusBarText.day[0],   &rt->date.Date,  1);
	TFTLCD_SendBuffer.buffer.statusBarText.str3 = ' ';
	HEX2ASCII(&TFTLCD_SendBuffer.buffer.statusBarText.hour[0],  &rt->time.Hours,  1);
	TFTLCD_SendBuffer.buffer.statusBarText.str4 = ':';
	HEX2ASCII(&TFTLCD_SendBuffer.buffer.statusBarText.min[0],   &rt->time.Minutes, 1);

	TFTLCD_SendBuffer.buffer.statusBarText.batCtlIdH =
			HALFWORD_BYTE_H(CTL_ID_BAT_QUANTITY_PERCENT);
	TFTLCD_SendBuffer.buffer.statusBarText.batCtlIdL =
			HALFWORD_BYTE_L(CTL_ID_BAT_QUANTITY_PERCENT);

	TFTLCD_SendBuffer.buffer.statusBarText.batSizeH = 0;
	TFTLCD_SendBuffer.buffer.statusBarText.batSizeL = 3;

	sprintf(TFTLCD_SendBuffer.buffer.statusBarText.batCapacity,
			"%3d", batQuantity);

	memcpy(&TFTLCD_SendBuffer.buffer.data[sizeof(StatusBarTextTypedef)],
			TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(sizeof(StatusBarTextTypedef) + 11);
}

/*******************************************************************************
 * function：状态栏更新，更新内容：电池电量图标、信号强度图标、报警图标
 * screenID：界面ID
 * RT_TimeTypedef:时间指针
 * batQuantity：电池电量
 */
void TFTLCD_StatusBarIconRefresh(uint16_t screenID)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_BATCH_UPDATE;

	/* 界面ID */
	TFTLCD_SendBuffer.screenIdH = HALFWORD_BYTE_H(screenID);
	TFTLCD_SendBuffer.screenIdL = HALFWORD_BYTE_L(screenID);

	TFTLCD_SendBuffer.buffer.statusBarIcon.batCtlIdH = HALFWORD_BYTE_H(CTL_ID_BAT_QUANTITY);
	TFTLCD_SendBuffer.buffer.statusBarIcon.batCtlIdL = HALFWORD_BYTE_L(CTL_ID_BAT_QUANTITY);
	TFTLCD_SendBuffer.buffer.statusBarIcon.batSizeH = 0;
	TFTLCD_SendBuffer.buffer.statusBarIcon.batSizeL = 2;
	TFTLCD_SendBuffer.buffer.statusBarIcon.batCapacityH = 0;
	if (INPUT_CheckPwrOnStatus())
	{
		TFTLCD_SendBuffer.buffer.statusBarIcon.batCapacityL = ICON_BAT_CHARGE;
	}
	else if (ANALOG_value.batVoltage >= 80)
	{
		TFTLCD_SendBuffer.buffer.statusBarIcon.batCapacityL = ICON_BAT_CAPACITY_80;
	}
	else if (ANALOG_value.batVoltage >= 60)
	{
		TFTLCD_SendBuffer.buffer.statusBarIcon.batCapacityL = ICON_BAT_CAPACITY_60;
	}
	else if (ANALOG_value.batVoltage >= 40)
	{
		TFTLCD_SendBuffer.buffer.statusBarIcon.batCapacityL = ICON_BAT_CAPACITY_40;
	}
	else if (ANALOG_value.batVoltage >= 20)
	{
		TFTLCD_SendBuffer.buffer.statusBarIcon.batCapacityL = ICON_BAT_CAPACITY_20;
	}
	else
	{
		TFTLCD_SendBuffer.buffer.statusBarIcon.batCapacityL = ICON_BAT_CAPACITY_0;
	}

	TFTLCD_SendBuffer.buffer.statusBarIcon.signalCtlIdH = HALFWORD_BYTE_H(CTL_ID_SIGNAL_QUALITY);
	TFTLCD_SendBuffer.buffer.statusBarIcon.signalCtlIdL = HALFWORD_BYTE_L(CTL_ID_SIGNAL_QUALITY);
	TFTLCD_SendBuffer.buffer.statusBarIcon.signalSizeH = 0;
	TFTLCD_SendBuffer.buffer.statusBarIcon.signalSizeL = 2;
	TFTLCD_SendBuffer.buffer.statusBarIcon.signalCapacityH = 0;
	if (GPRS_signalQuality >= 21)
	{
		TFTLCD_SendBuffer.buffer.statusBarIcon.signalCapacityL = ICON_SIGNAL_QUALITY_31_21;
	}
	else if (GPRS_signalQuality >= 11)
	{
		TFTLCD_SendBuffer.buffer.statusBarIcon.signalCapacityL = ICON_SIGNAL_QUALITY_21_11;
	}
	else
	{
		TFTLCD_SendBuffer.buffer.statusBarIcon.signalCapacityL = ICON_SIGNAL_QUALITY_11_0;
	}

	TFTLCD_SendBuffer.buffer.statusBarIcon.alarmCtlIdH = HALFWORD_BYTE_H(CTL_ID_ALARM_ICON);
	TFTLCD_SendBuffer.buffer.statusBarIcon.alarmCtlIdL = HALFWORD_BYTE_L(CTL_ID_ALARM_ICON);
	TFTLCD_SendBuffer.buffer.statusBarIcon.alarmSizeH = 0;
	TFTLCD_SendBuffer.buffer.statusBarIcon.alarmSizeL = 2;
	TFTLCD_SendBuffer.buffer.statusBarIcon.alarmCapacityH = 0;
	if (ANALOG_alarmStatus.status.all != 0)
	{
		TFTLCD_SendBuffer.buffer.statusBarIcon.alarmCapacityL = ICON_ALARM_ON;
	}
	else
	{
		TFTLCD_SendBuffer.buffer.statusBarIcon.alarmCapacityL = ICON_ALARM_OFF;
	}

	memcpy(&TFTLCD_SendBuffer.buffer.data[sizeof(StatusBarIconTypedef)],
			TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(sizeof(StatusBarIconTypedef) + 11);
}

/*******************************************************************************
 * function:将读出来的数据转换格式后输出到历史界面
 * @saveInfo：读出的数据
 * @typeID：更新的控件
 */
void TFTLCD_HistoryDataFormat(FILE_SaveStructTypedef* saveInfo, TFTLCD_HisDataCtlIdEnum typeID)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_TEXT_UPDATE;

	TFTLCD_SendBuffer.screenIdH = HALFWORD_BYTE_H(SCREEN_ID_HIS_DATA);
	TFTLCD_SendBuffer.screenIdL = HALFWORD_BYTE_L(SCREEN_ID_HIS_DATA);

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HALFWORD_BYTE_H(typeID);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HALFWORD_BYTE_L(typeID);

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
}

/*******************************************************************************
 *
 */
void TFTLCD_HistoryDataCurveFormat(FILE_SaveStructTypedef* saveInfo)
{
	char  str[6];
	float value;
	uint8_t i;

	for (i = 0; i < 8; i++)
	{
		/* 将字符串转为float */
		memcpy(str, saveInfo->analogValue[i].value, 5);
		str[5] = '\0';
		value = (float)atof(str);

		/* 显示通道曲线，值相对底部偏移30 */
		TFTLCD_HistoryDataCurveDisplay(i, (uint8_t)(value + 30));
	}
}


/*******************************************************************************
 * function:打印通道选择图标显示
 * @ctrl：通道选择控件编号
 * @status：当前该通道的值，若选中则变成不选，反之亦然。
 */
void TFTLCD_SetIconValue(TFTLCD_ScreenIDEnum screen, uint16_t typeID, uint8_t status)
{
	/* 进入临界区 */
//	taskENTER_CRITICAL();

	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_ICON_DISP;

	/* 界面ID */
	TFTLCD_SendBuffer.screenIdH = HALFWORD_BYTE_H(screen);
	TFTLCD_SendBuffer.screenIdL = HALFWORD_BYTE_L(screen);

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HALFWORD_BYTE_H(typeID);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HALFWORD_BYTE_L(typeID);

	TFTLCD_SendBuffer.buffer.update.value.date[0] = status;

	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[1],
			TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(12);

	/* 退出临界区 */
//	taskEXIT_CRITICAL();
}

/*******************************************************************************
 * @brief 设置时间选择控件的值
 * @param
 */
void TFTLCD_SetTimeSelect(TFTLCD_ScreenIDEnum screen, TFTLCD_TimeSelectCtlIdEnum ctlID, uint8_t value)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_SELECT;

	TFTLCD_SendBuffer.screenIdH = HALFWORD_BYTE_H(screen);
	TFTLCD_SendBuffer.screenIdL = HALFWORD_BYTE_L(screen);

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HALFWORD_BYTE_H(ctlID);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HALFWORD_BYTE_L(ctlID);

//	TFTLCD_SendBuffer.buffer.update.value.date[0] = 0x1B;
	TFTLCD_SendBuffer.buffer.update.value.date[0] = value;
	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[1],
				TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(12);
}

/*******************************************************************************
 * function:将时间选择界面选好的数值更新到指定的时间控件
 * @time：选好的时间
 */
void TFTLCD_SelectTimeUpdate(TFTLCD_ScreenIDEnum screen, uint16_t ctlID, FILE_RealTimeTypedef* time)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_TEXT_UPDATE;

	TFTLCD_SendBuffer.screenIdH = HALFWORD_BYTE_H(screen);
	TFTLCD_SendBuffer.screenIdL = HALFWORD_BYTE_L(screen);

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HALFWORD_BYTE_H(ctlID);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HALFWORD_BYTE_L(ctlID);

	HEX2ASCII(TFTLCD_SendBuffer.buffer.update.value.date, &time->year,  3);
	TFTLCD_SendBuffer.buffer.update.value.date[6] = ' ';
	HEX2ASCII(&TFTLCD_SendBuffer.buffer.update.value.date[7], &time->hour, 1);
	TFTLCD_SendBuffer.buffer.update.value.date[9] = ':';
	HEX2ASCII(&TFTLCD_SendBuffer.buffer.update.value.date[10], &time->min, 1);

	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[12],
				TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(23);
}

/*******************************************************************************
 * function:密码设置界面文本框更新，显示当前输入的密码位数
 */
void TFTLCD_SetPasswordUpdate(uint8_t numb)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_TEXT_UPDATE;

	TFTLCD_SendBuffer.screenIdH = HALFWORD_BYTE_H(SCREEN_ID_SET_PASSWORD);
	TFTLCD_SendBuffer.screenIdL = HALFWORD_BYTE_L(SCREEN_ID_SET_PASSWORD);

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HALFWORD_BYTE_H(CTL_ID_SET_PASSWORD_TEXT);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HALFWORD_BYTE_L(CTL_ID_SET_PASSWORD_TEXT);

	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[0], "****", numb);
	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[numb],
				TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(numb + 11);
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
 * @brief 界面默认显示的一些参数
 */
static void TFTLCD_DisplayInit(void)
{
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH1,
			PARAM_DeviceParam.chAlarmValue[0].alarmValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH1,
			PARAM_DeviceParam.chAlarmValue[0].alarmValueLow);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH1,
			PARAM_DeviceParam.chAlarmValue[0].perwarningValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH1,
			PARAM_DeviceParam.chAlarmValue[0].perwarningValueLow);

	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH2,
			PARAM_DeviceParam.chAlarmValue[1].alarmValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH2,
			PARAM_DeviceParam.chAlarmValue[1].alarmValueLow);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH2,
			PARAM_DeviceParam.chAlarmValue[1].perwarningValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH2,
			PARAM_DeviceParam.chAlarmValue[1].perwarningValueLow);

	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH3,
			PARAM_DeviceParam.chAlarmValue[2].alarmValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH3,
			PARAM_DeviceParam.chAlarmValue[2].alarmValueLow);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH3,
			PARAM_DeviceParam.chAlarmValue[2].perwarningValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH3,
			PARAM_DeviceParam.chAlarmValue[2].perwarningValueLow);

	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH4,
			PARAM_DeviceParam.chAlarmValue[3].alarmValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH4,
			PARAM_DeviceParam.chAlarmValue[3].alarmValueLow);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH4,
			PARAM_DeviceParam.chAlarmValue[3].perwarningValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT, CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH4,
			PARAM_DeviceParam.chAlarmValue[3].perwarningValueLow);

	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH5,
			PARAM_DeviceParam.chAlarmValue[4].alarmValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH5,
			PARAM_DeviceParam.chAlarmValue[4].alarmValueLow);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH5,
			PARAM_DeviceParam.chAlarmValue[4].perwarningValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH5,
			PARAM_DeviceParam.chAlarmValue[4].perwarningValueLow);

	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH6,
			PARAM_DeviceParam.chAlarmValue[5].alarmValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH6,
			PARAM_DeviceParam.chAlarmValue[5].alarmValueLow);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH6,
			PARAM_DeviceParam.chAlarmValue[5].perwarningValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH6,
			PARAM_DeviceParam.chAlarmValue[5].perwarningValueLow);

	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH7,
			PARAM_DeviceParam.chAlarmValue[6].alarmValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH7,
			PARAM_DeviceParam.chAlarmValue[6].alarmValueLow);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH7,
			PARAM_DeviceParam.chAlarmValue[6].perwarningValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH7,
			PARAM_DeviceParam.chAlarmValue[6].perwarningValueLow);

	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH8,
			PARAM_DeviceParam.chAlarmValue[7].alarmValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH8,
			PARAM_DeviceParam.chAlarmValue[7].alarmValueLow);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH8,
			PARAM_DeviceParam.chAlarmValue[7].perwarningValueUp);
	FloatValueUpdate(SCREEN_ID_SET_ALARM_LIMIT_2, CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH8,
			PARAM_DeviceParam.chAlarmValue[7].perwarningValueLow);

	TFTLCD_TextValueUpdate(SCREEN_ID_SET_ALARM_CODE, CTL_ID_SET_ALARM_CODE_1,
			(char*)&PARAM_DeviceParam.alarmCode[0], 11);
	TFTLCD_TextValueUpdate(SCREEN_ID_SET_ALARM_CODE, CTL_ID_SET_ALARM_CODE_2,
			(char*)&PARAM_DeviceParam.alarmCode[1], 11);
	TFTLCD_TextValueUpdate(SCREEN_ID_SET_ALARM_CODE, CTL_ID_SET_ALARM_CODE_3,
			(char*)&PARAM_DeviceParam.alarmCode[2], 11);

	TFTLCD_TextValueUpdate(SCREEN_ID_ABOUT_DEVICE, CTL_ID_ABOUT_DEVICE_SN,
			"1708151515", 10);
	TFTLCD_TextValueUpdate(SCREEN_ID_ABOUT_DEVICE, CTL_ID_ABOUT_DEVICE_TYPE,
				"G95-****", 8);
	TFTLCD_TextValueUpdate(SCREEN_ID_ABOUT_DEVICE, CTL_ID_ABOUT_CHANNEL_NUMB,
				"8", 1);
	TFTLCD_TextValueUpdate(SCREEN_ID_ABOUT_DEVICE, CTL_ID_ABOUT_FIRM_VERSION,
				"V1.0.10", 7);
	TFTLCD_TextValueUpdate(SCREEN_ID_ABOUT_DEVICE, CTL_ID_ABOUT_OS_VERSION,
				"FreeRTOS 9.0.0", 14);
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
	batch->ctrlIdH = HALFWORD_BYTE_H(typeID);
	batch->ctrlIdL = HALFWORD_BYTE_L(typeID);

	if (analog == ANALOG_CHANNLE_INVALID_VALUE)
		memcpy(batch->value, ANALOG_INVALID_VALUE, 5);
	else
	{
		/* %5.1表示有效数据长度为5，小数1位 */
		sprintf(batch->value, "%5.1f", analog);
	}

	/* 长度固定为5 */
	batch->sizeH = 0x00;
	batch->sizeL = 0x05;
}

/*******************************************************************************
 *
 */
static void TFTLCD_ScreenStart(void)
{
	osDelay(3000);

	TFTLCD_SetScreenId(SCREEN_ID_CUR_DATA_8CH);
}

/*******************************************************************************
 * function:历史数据曲线显示
 * @channel：曲线通道
 * @data：该通道的值
 */
static void TFTLCD_HistoryDataCurveDisplay(uint8_t channel, uint8_t data)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_CURVE_ADD_DATA_TAIL;

	TFTLCD_SendBuffer.screenIdH = HALFWORD_BYTE_H(SCREEN_ID_HIS_DATA_CURVE);
	TFTLCD_SendBuffer.screenIdL = HALFWORD_BYTE_L(SCREEN_ID_HIS_DATA_CURVE);

	TFTLCD_SendBuffer.buffer.curve.ctlIdH = HALFWORD_BYTE_H(CTL_ID_HIS_DATA_CURVE);
	TFTLCD_SendBuffer.buffer.curve.ctlIdL = HALFWORD_BYTE_L(CTL_ID_HIS_DATA_CURVE);

	TFTLCD_SendBuffer.buffer.curve.channel     = channel;
	TFTLCD_SendBuffer.buffer.curve.dataLengthH = 0;
	TFTLCD_SendBuffer.buffer.curve.dataLengthL = 1;
	TFTLCD_SendBuffer.buffer.curve.data        = data;

	memcpy(&TFTLCD_SendBuffer.buffer.data[sizeof(CurveTypedef)],
				TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(sizeof(CurveTypedef) + 11);
}

/*******************************************************************************
 * @brief 设置文本控件前景色
 */
static void SetTextForeColor(uint16_t ctlID, uint16_t foreColor)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_SET_FORE_COLOR;

	TFTLCD_SendBuffer.screenIdH = HALFWORD_BYTE_H(SCREEN_ID_CUR_DATA_8CH);
	TFTLCD_SendBuffer.screenIdL = HALFWORD_BYTE_L(SCREEN_ID_CUR_DATA_8CH);

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HALFWORD_BYTE_H(ctlID);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HALFWORD_BYTE_L(ctlID);

	TFTLCD_SendBuffer.buffer.update.value.date[0] = HALFWORD_BYTE_H(foreColor);
	TFTLCD_SendBuffer.buffer.update.value.date[1] = HALFWORD_BYTE_L(foreColor);

	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[2], TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(13);
}

/*******************************************************************************
 * @brief 判断模拟量的值是否超标，超标则改变指定控件的颜色
 */
static void AnalogAlarmDisplay(uint16_t ctlID, float analog, ParamAlarmTypedef* param)
{
	/* 上下限比较 */
	if ((analog > param->alarmValueUp) || (analog < param->alarmValueLow))
	{
		SetTextForeColor(ctlID, TFTLCD_ALARM_COLOR);
		ANALOG_Mode = ANALOG_MODE_ALARM;
	}
	else if ((analog > param->perwarningValueUp) || (analog < param->perwarningValueLow))
	{
		SetTextForeColor(ctlID, TFTLCD_PERWARM_COLOR);
		ANALOG_Mode = ANALOG_MODE_PERWARM;
	}
	else
	{
		ANALOG_Mode = ANALOG_MODE_NORMAL;
	}
}

/*******************************************************************************
 * 更新界面的浮点型值
 */
static void FloatValueUpdate(uint16_t screenID, uint16_t ctlID, float data)
{
	char str[6];
	sprintf(str, "%5.1f", data);
	TFTLCD_TextValueUpdate(screenID, ctlID, str, 5);
}

