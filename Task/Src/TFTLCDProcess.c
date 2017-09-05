#include "TFTLCDProcess.h"

#include "tftlcd.h"
#include "print.h"
#include "gprs.h"
#include "display.h"

/******************************************************************************/
void ScreenDefaultDisplay(uint16_t screen);
void ScreenTouchDisplay(uint16_t screenID, uint16_t typeID);

/*******************************************************************************
 *
 */
void TFTLCD_Task(void)
{
	osEvent signal;
	uint16_t screenID;
	uint16_t ctrlID;

	TFTLCD_Init();

	while(1)
	{
		/* 获取任务信号 */
		signal = osSignalWait(0xFFFFFFFF, 1);

		/* 状态栏刷新 */
		if ((signal.value.signals & TFTLCD_TASK_STATUS_BAR_UPDATE) ==
				TFTLCD_TASK_STATUS_BAR_UPDATE)
		{
			TFTLCD_StatusBarTextRefresh(TFTLCD_status.curScreenID, &RT_RealTime,
					ANALOG_value.batVoltage);
		}

		/* 刷新实时数据 */
		if ((signal.value.signals & TFTLCD_TASK_ANALOG_UPDATE) ==
				TFTLCD_TASK_ANALOG_UPDATE)
		{
			TFTLCD_AnalogDataRefresh(&ANALOG_value);
		}

		/* 串口接收处理 */
		if ((signal.value.signals & TFTLCD_TASK_RECV_ENABLE) == TFTLCD_TASK_RECV_ENABLE)
		{
			/* 检测头和尾 */
			if (ERROR != TFTLCD_CheckHeadTail())
			{
				/* 识别画面ID */
				screenID = ((TFTLCD_RecvBuffer.date.recvBuf.screenIdH << 8)
						| (TFTLCD_RecvBuffer.date.recvBuf.screenIdL));

				if (TFTLCD_RecvBuffer.date.recvBuf.cmd == TFTLCD_CMD_SCREEN_ID_GET)
				{
					TFTLCD_status.curScreenID = (TFTLCD_ScreenIDEnum)((TFTLCD_RecvBuffer.date.recvBuf.screenIdH << 8)
							| (TFTLCD_RecvBuffer.date.recvBuf.screenIdL));

					/* 界面跳转后，需要刷新一次状态栏 */
					osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_STATUS_BAR_UPDATE);

					/* 界面跳转后，默认出现的画面 */
					ScreenDefaultDisplay(screenID);
				}
				else
				{
					/* 控件ID */
					ctrlID = ((TFTLCD_RecvBuffer.date.recvBuf.ctrlIDH << 8)
							| (TFTLCD_RecvBuffer.date.recvBuf.ctrlIDL));

					/* 接收用户对触摸屏的操作信息 */
					ScreenTouchDisplay(screenID, ctrlID);
				}
			}
		}
	}
}

/*******************************************************************************
 * function:界面跳转后，默认出现的信息
 */
void ScreenDefaultDisplay(uint16_t screen)
{
	switch(screen)
	{
	case SCREEN_ID_HIS_DATA:
		/* 显示最新的一组数据 */
		DISPLAY_Status.hisDataDispStructOffset =
				dataFileStructCnt - DISPLAY_HIS_DATA_ONE_SCREEN_CNT;
		DISPLAY_HistoryData(DISPLAY_Status.hisDataDispStructOffset);
		break;
	default:
		break;
	}
}

/*******************************************************************************
 * funtion:接收到用户操作时的显示
 */
void ScreenTouchDisplay(uint16_t screenID, uint16_t typeID)
{
	switch (screenID)
	{
	case SCREEN_ID_HIS_DATA:
		DISPLAY_HistoryTouch(typeID);
		break;

	case SCREEN_ID_PRINT:
		DISPLAY_PrintTouch(typeID);
		break;

	case SCREEN_ID_TIME_SELECT:
		DISPLAY_TimeSelectTouch(typeID, TFTLCD_RecvBuffer.date.recvBuf.buf[1]);
		break;

	default:
		break;
	}
}

