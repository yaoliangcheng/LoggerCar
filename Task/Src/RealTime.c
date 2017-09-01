#include "RealTime.h"

#include "osConfig.h"
#include "Mainprocess.h"

#include "analog.h"
#include "file.h"
#include "analog.h"
#include "tftlcd.h"

#include "rt.h"

/******************************************************************************/
void REALTIME_Task(void)
{
	osEvent signal;
//	RT_TimeTypedef sTime;

	while(1)
	{
		signal = osSignalWait(REALTIME_TASK_SIGNAL_UPDATE, 2000);

		/* 收到使能事件组标志位 */
		if ((signal.value.signals & REALTIME_TASK_SIGNAL_UPDATE)
				== REALTIME_TASK_SIGNAL_UPDATE)
		{
			HAL_RTC_GetTime(&hrtc, &RT_RealTime.time, RTC_FORMAT_BIN);
			if (RT_RealTime.oldWeekDay != hrtc.DateToUpdate.WeekDay)
			{
				/* 更新日期 */
				HAL_RTC_GetDate(&hrtc, &RT_RealTime.date, RTC_FORMAT_BIN);

				/* 日期更新到备份 */
				RT_BKUP_UpdateDate(&RT_RealTime);
			}

			/* 每分钟温湿度采样一次 */
			if (RT_RealTime.time.Seconds % 30 == 0)
			{
				/* 触发ADC采样 */
				ANALOG_ConvertEnable();

				/* 等待ADC采样完成 */
				signal = osSignalWait(REALTIME_SENSOR_CONVERT_FINISH, 2000);
				if ((signal.value.signals & REALTIME_SENSOR_CONVERT_FINISH)
								== REALTIME_SENSOR_CONVERT_FINISH)
				{
					/* 获取传感器的值 */
					ANALOG_GetSensorValue();
					/* 更新液晶屏显示 */
					if (TFTLCD_status.curScreenID == SCREEN_ID_CUR_DATA_8CH)
					{
						osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_ANALOG_UPDATE);
					}

					/* 如果记录间隔时间到，则触发记录 */
					if (RT_RealTime.time.Seconds % 60 == 0)
					{
						/* 把时间传递到GPRS进程，便于根据平台回文校准时间 */
//						osMessagePut(adjustTimeMessageQId, (uint32_t)&RT_RealTime, 1000);

						/* 发送记录时间数据,先把时间转换成BCD模式 */
//						HEX2BCD((uint8_t*)&RT_RealTime, (uint8_t*)&sTime, sizeof(RT_TimeTypedef));
						osMessagePut(realtimeMessageQId, (uint32_t)&RT_RealTime, 100);

						/* 发送模拟量数据 */
//						osMessagePut(analogMessageQId, (uint32_t)&ANALOG_value, 100);

						/* 激活MainProcess任务 */
						osThreadResume(mainprocessTaskHandle);

						/* 更新状态栏 */
						osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_STATUS_BAR_UPDATE);
					}
				}
			}
		}
	}
}




