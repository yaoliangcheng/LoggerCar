#include "TFTLCDProcess.h"

#include "tftlcd.h"
#include "print.h"
#include "gprs.h"
#include "display.h"
#include "ble.h"

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

	/* 需等待os开始运行再开始延时启动，其余任务也可同步进行 */
	TFTLCD_Init();

	while(1)
	{
//		BLE_LinkPrint();

		/* 获取任务信号 */
		signal = osSignalWait(TFTLCD_TASK_STATUS_BAR_UPDATE
				| TFTLCD_TASK_ANALOG_UPDATE | TFTLCD_TASK_RECV_ENABLE, 1000);

		/* 状态栏刷新 */
		if ((signal.value.signals & TFTLCD_TASK_STATUS_BAR_UPDATE) ==
				TFTLCD_TASK_STATUS_BAR_UPDATE)
		{
			/* 更新状态栏文本 */
			TFTLCD_StatusBarTextRefresh(TFTLCD_status.curScreenID, &RT_RealTime,
					ANALOG_value.batVoltage);

			/* 更新状态栏图标 */
			TFTLCD_StatusBarIconRefresh(TFTLCD_status.curScreenID);
		}

		/* 刷新实时数据 */
		if ((signal.value.signals & TFTLCD_TASK_ANALOG_UPDATE) ==
				TFTLCD_TASK_ANALOG_UPDATE)
		{
			/* 显示模拟量的值 */
			TFTLCD_AnalogDataRefresh(&ANALOG_value);
			/* 判断模拟量的值是否超标 */
			TFTLCD_AnalogDataAlarmDisplay(&ANALOG_value);
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

				/* 界面更新的指令 */
				if (TFTLCD_RecvBuffer.date.recvBuf.cmd == TFTLCD_CMD_SCREEN_ID_GET)
				{
					/* 不知道为什么，界面跳转后屏幕会发一次0界面，导致错误 */
					if (TFTLCD_RecvBuffer.date.recvBuf.screenIdL != 0)
					{
						TFTLCD_status.curScreenID =
								(TFTLCD_ScreenIDEnum)(TFTLCD_RecvBuffer.date.recvBuf.screenIdL);
						/* 界面跳转后，需要刷新一次状态栏 */
						osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_STATUS_BAR_UPDATE);
						/* 界面跳转后，默认出现的画面 */
						ScreenDefaultDisplay(screenID);
					}
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
		if (FILE_DataSaveStructCnt >= DISPLAY_HIS_DATA_ONE_SCREEN_CNT)
		{
			/* 显示最新的一组数据 */
			DISPLAY_Status.hisDataDispStructOffset =
					FILE_DataSaveStructCnt - DISPLAY_HIS_DATA_ONE_SCREEN_CNT;

			DISPLAY_HistoryData(DISPLAY_Status.hisDataDispStructOffset,
					DISPLAY_HIS_DATA_ONE_SCREEN_CNT);
		}
		break;

	/* 历史曲线界面 */
	case SCREEN_ID_HIS_DATA_CURVE:
		if (FILE_DataSaveStructCnt >= DISPLAY_HIS_DATA_CURVE_CNT)
		{
			DISPLAY_Status.hisDataDispStructOffset =
					FILE_DataSaveStructCnt - DISPLAY_HIS_DATA_CURVE_CNT;

			DISPLAY_HistoryDataCurve(DISPLAY_Status.hisDataDispStructOffset);
		}
		break;

	/* 密码界面 */
	case SCREEN_ID_SET_PASSWORD:
		/* 清空密码缓存和位指示 */
		memcpy(DISPLAY_Status.passwordBuffer, "    ", 4);
		DISPLAY_Status.passwordBufferIndex = 0;
		TFTLCD_SetPasswordUpdate(DISPLAY_Status.passwordBufferIndex);
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

	case SCREEN_ID_SET_PASSWORD:
		DISPLAY_SetPasswordTouch(typeID);
		break;

	case SCREEN_ID_SET_ALARM_LIMIT:
		DISPLAY_SetAlarmLimitTouch(typeID);
		break;

	case SCREEN_ID_SET_ALARM_LIMIT_2:
		DISPLAY_SetAlarmLimit2Touch(typeID);
		break;

	case SCREEN_ID_SET_ALARM_CODE:
		DISPLAY_SetMessageTouch(typeID);
		break;

	case SCREEN_ID_SET_CHANGE_PASSWORD:
		DISPLAY_SetPasswordChangeTouch(typeID);
		break;

	default:
		break;
	}
}

