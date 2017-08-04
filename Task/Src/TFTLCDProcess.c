#include "TFTLCDProcess.h"

#include "tftlcd.h"

/******************************************************************************/
static void ScreenPrint(uint16_t cmd, CtrlID_PrintEnum ctrl, TFTTASK_StatusEnum* status);
static void ScreenTimeSelect(FILE_RealTime* pTime, uint16_t cmd,
		CtrlID_TimeSelectEnum ctrl, uint8_t value, TFTTASK_StatusEnum status);

/*******************************************************************************
 *
 */
void TFTLCD_Task(void)
{
	osEvent signal;
	uint16_t screenID;
	uint16_t ctrlID;

	FILE_RealTime printTime;		/* 打印起始时间 */

	TFTTASK_StatusEnum status;

	TFTLCD_Init();

	while(1)
	{
		signal = osSignalWait(TFTLCD_TASK_RECV_ENABLE, osWaitForever);
		if ((signal.value.signals & TFTLCD_TASK_RECV_ENABLE) != TFTLCD_TASK_RECV_ENABLE)
			break;

		/* 检测头和尾 */
		if (ERROR == TFTLCD_CheckHeadTail())
			break;

		/* 识别画面ID和控件ID */
		screenID = ((TFTLCD_RecvBuffer.date.recvBuf.screenIdH << 8)
				| (TFTLCD_RecvBuffer.date.recvBuf.screenIdL));
		ctrlID = ((TFTLCD_RecvBuffer.date.recvBuf.ctrlIDH << 8)
				| (TFTLCD_RecvBuffer.date.recvBuf.ctrlIDL));

		/* 按照界面来划分 */
		switch (screenID)
		{
		case SCREEN_ID_PRINT:
			ScreenPrint(TFTLCD_RecvBuffer.date.recvBuf.cmd, (CtrlID_PrintEnum)ctrlID, &status);
			break;

		case SCREEN_ID_PRINT_TIME_SELECT:
//			/* 固定字节，表示选择控件 */
//			if (TFTLCD_RecvBuffer.date.recvBuf.buf[0] != 0x1B)
//				break;
			ScreenTimeSelect(&printTime, TFTLCD_RecvBuffer.date.recvBuf.cmd,
					(CtrlID_TimeSelectEnum)ctrlID, TFTLCD_RecvBuffer.date.recvBuf.buf[1], status);
			break;
		default:
			break;
		}
	}

}

/*******************************************************************************
 *
 */
static void ScreenPrint(uint16_t cmd, CtrlID_PrintEnum ctrl, TFTTASK_StatusEnum* status)
{
	switch (cmd)
	{
	case TFTLCD_CMD_BUTTON:
		switch (ctrl)
		{
		case PRINT_CTRL_ID_START_TIME_BUTTON:
			*status = TFT_PRINT_START_TIME;
			break;
		case PRINT_CTRL_ID_END_TIME_BUTTON:
			*status = TFT_PRINT_END_TIME;
			break;
		case PRINT_CTRL_ID_START_PRINT:
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
}

/*******************************************************************************
 * function:时间选择控件数值上传
 * @pTime:储存时间结构体
 * @cmd：命令类型
 * @ctrl：控件枚举
 * @value：控件值
 * @timeType：时间类型（1：打印起始时间，0：打印结束时间）
 */
static void ScreenTimeSelect(FILE_RealTime* pTime, uint16_t cmd,
		CtrlID_TimeSelectEnum ctrl, uint8_t value, TFTTASK_StatusEnum status)
{
	/* 按照控件类型划分 */
	switch (cmd)
	{
	/* 选择控件类型 */
	case TFTLCD_CMD_BUTTON_SELECT:
		switch (ctrl)
		{
		case TIME_SELECT_CTRL_ID_YEAR:
			pTime->year = 17 + value;
			break;
		case TIME_SELECT_CTRL_ID_MONTH:
			pTime->month = 1 + value;
			break;
		case TIME_SELECT_CTRL_ID_DAY:
			pTime->day = 1 + value;
			break;
		case TIME_SELECT_CTRL_ID_HOUR:
			pTime->hour = value;
			break;
		case TIME_SELECT_CTRL_ID_MIN:
			pTime->min = 5 * value;
			break;
		case TIME_SELECT_CTRL_ID_CANCEL:
			/* 取消则把时间清空 */
			memset(pTime, 0, sizeof(FILE_RealTime));
			break;
		case TIME_SELECT_CTRL_ID_OK:
			switch (status)
			{
			case TFT_PRINT_START_TIME:
				TFTLCD_printTimeUpdate(pTime, PRINT_CTRL_ID_START_TIME);
				break;
			case TFT_PRINT_END_TIME:
				TFTLCD_printTimeUpdate(pTime, PRINT_CTRL_ID_END_TIME);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
}


















