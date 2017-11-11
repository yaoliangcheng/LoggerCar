#include "RealTime.h"

#include "iwdg.h"
#include "analog.h"
#include "tftlcd.h"
#include "rt.h"

/******************************************************************************/
extern osThreadId mainprocessTaskHandle;
extern osThreadId tftlcdTaskHandle;

/*******************************************************************************
 *
 */
void REALTIME_Task(void)
{
	osEvent signal;
	osEvent signalAnalog;

	while(1)
	{
		/* 等待时间和模拟量更新标志或者闹钟触发记录标志 */
		signal = osSignalWait(REALTIME_TASK_TIME_ANALOG_UPDATE | REALTIME_TASK_ALRAM_RECORD, 2000);
		if (((signal.value.signals & REALTIME_TASK_TIME_ANALOG_UPDATE) == REALTIME_TASK_TIME_ANALOG_UPDATE)
			|| ((signal.value.signals & REALTIME_TASK_ALRAM_RECORD) == REALTIME_TASK_ALRAM_RECORD))
		{
			/* 更新时间 */
			HAL_RTC_GetTime(&hrtc, &RT_RealTime.time, RTC_FORMAT_BIN);
			if (RT_RealTime.oldWeekDay != hrtc.DateToUpdate.WeekDay)
			{
				/* 更新日期 */
				HAL_RTC_GetDate(&hrtc, &RT_RealTime.date, RTC_FORMAT_BIN);

				/* 日期更新到备份 */
				RT_BKUP_UpdateDate(&RT_RealTime);
			}
			/* 更新状态栏 */
			osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_STATUS_BAR_UPDATE);

			/* 更新模拟量 */
			ANALOG_ConvertEnable();
			signalAnalog = osSignalWait(REALTIME_TASK_SENSOR_CONVERT_FINISH, 2000);
			if ((signalAnalog.value.signals & REALTIME_TASK_SENSOR_CONVERT_FINISH)
							== REALTIME_TASK_SENSOR_CONVERT_FINISH)
			{
				/* 获取传感器的值 */
				ANALOG_GetSensorValue();
				/* 更新液晶屏显示 */
				osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_ANALOG_UPDATE);
			}

			/* 如果是闹钟触发，则记录数据 */
			if ((signal.value.signals & REALTIME_TASK_ALRAM_RECORD) == REALTIME_TASK_ALRAM_RECORD)
			{
				/* 发送记录时间数据 */
				memcpy(&RT_RecordTime, &RT_RealTime, sizeof(RT_TimeTypedef));
				/* 激活MainProcess任务 */
				osThreadResume(mainprocessTaskHandle);
			}
		}
#if IWDG_ENABLE
		/* 看门狗监控 */
//		HAL_IWDG_Refresh(&hiwdg);
#endif

	}
}




