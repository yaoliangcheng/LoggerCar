#include "rt.h"
#include "RealTime.h"
#include "osConfig.h"

/*******************************************************************************
 *
 */
void RT_BKUP_UpdateDate(RT_TimeTypedef* time)
{
	/* 更新备份日期 */
	HAL_RTCEx_BKUPWrite(&RT_RTC, RTC_BKUP_REG_YEAR,  RT_RTC.DateToUpdate.Year);
	HAL_RTCEx_BKUPWrite(&RT_RTC, RTC_BKUP_REG_MONTH, RT_RTC.DateToUpdate.Month);
	HAL_RTCEx_BKUPWrite(&RT_RTC, RTC_BKUP_REG_DAY,   RT_RTC.DateToUpdate.Date);
	HAL_RTCEx_BKUPWrite(&RT_RTC, RTC_BKUP_REG_WEEK,  RT_RTC.DateToUpdate.WeekDay);

	/* 更新上次记录星期 */
	time->oldWeekDay = RT_RTC.DateToUpdate.WeekDay;
}

/*******************************************************************************
 *
 */
static void RT_BKUP_ReadDate(void)
{
	/* 读取备份日期 */
	RT_RTC.DateToUpdate.Year =    HAL_RTCEx_BKUPRead(&RT_RTC, RTC_BKUP_REG_YEAR);
	RT_RTC.DateToUpdate.Month =   HAL_RTCEx_BKUPRead(&RT_RTC, RTC_BKUP_REG_MONTH);
	RT_RTC.DateToUpdate.Date =    HAL_RTCEx_BKUPRead(&RT_RTC, RTC_BKUP_REG_DAY);
	RT_RTC.DateToUpdate.WeekDay = HAL_RTCEx_BKUPRead(&RT_RTC, RTC_BKUP_REG_WEEK);
}

/*******************************************************************************
 *
 */
void RT_SetRealTime(RT_TimeTypedef* time)
{
	HAL_RTC_SetDate(&RT_RTC, &time->date, RTC_FORMAT_BCD);
	HAL_RTC_SetTime(&RT_RTC, &time->time, RTC_FORMAT_BCD);

	/* 将更新的日期备份 */
	RT_BKUP_UpdateDate(time);
}

/*******************************************************************************
 *
 */
void RT_Init(RT_TimeTypedef* time)
{
	if (HAL_RTCEx_BKUPRead(&RT_RTC, RTC_BKUP_REG_DATA) != RTC_BKUP_DATA)
	{
		time->date.Year = 0X17;
		time->date.Month = RTC_MONTH_JULY;
		time->date.Date = 0x11;
		time->date.WeekDay = RTC_WEEKDAY_TUESDAY;
		time->time.Hours = 0x13;
		time->time.Minutes = 0x43;
		time->time.Seconds = 0x00;
		RT_SetRealTime(time);

		HAL_RTCEx_BKUPWrite(&RT_RTC, RTC_BKUP_REG_DATA, RTC_BKUP_DATA);
	}
	else
	{
		RT_BKUP_ReadDate();
		HAL_RTC_GetDate(&RT_RTC, &time->date, RTC_FORMAT_BCD);
	}
}

/******************************************************************************/
void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
	osSignalSet(realtimeTaskHandle, REALTIME_TASK_SIGNAL_UPDATE);
}
