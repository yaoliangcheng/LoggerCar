#include "rt.h"

#include "common.h"

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
	HAL_RTC_SetDate(&RT_RTC, &time->date, RTC_FORMAT_BIN);
	HAL_RTC_SetTime(&RT_RTC, &time->time, RTC_FORMAT_BIN);

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
		time->date.Year = 17;
		time->date.Month = RTC_MONTH_AUGUST;
		time->date.Date = 17;
		time->date.WeekDay = RTC_WEEKDAY_THURSDAY;
		time->time.Hours = 10;
		time->time.Minutes = 55;
		time->time.Seconds = 0x00;
		RT_SetRealTime(time);

		HAL_RTCEx_BKUPWrite(&RT_RTC, RTC_BKUP_REG_DATA, RTC_BKUP_DATA);
	}
	else
	{
		RT_BKUP_ReadDate();
		HAL_RTC_GetDate(&RT_RTC, &time->date, RTC_FORMAT_BIN);
	}
}

/******************************************************************************/
void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
	osSignalSet(realtimeTaskHandle, REALTIME_TASK_SIGNAL_UPDATE);
}

/*******************************************************************************
 * 与云平台时间校准
 * @pBuffer：接收缓存数据（平台回文）
 * @pStruct：发送结构体
 */
void RT_TimeAdjustWithCloud(uint8_t* pBuffer, RT_TimeTypedef* time)
{
	uint8_t str[12] = {0};

	RT_TimeTypedef   eTime;

	/* 将字符转换成数字 */
	str2numb((pBuffer + RT_OFFSET_CLOUD_TIME), str, sizeof(str));

	eTime.date.Year    = (str[0]  * 10)  + str[1];
	eTime.date.Month   = (str[2]  * 10)  + str[3];
	eTime.date.Date    = (str[4]  * 10)  + str[5];
	eTime.time.Hours   = (str[6]  * 10)  + str[7];
	eTime.time.Minutes = (str[8]  * 10)  + str[9];
	eTime.time.Seconds = (str[10] * 10)  + str[11];

	/* 接收到平台回文，与发送时间比较，若相差年月日时分有偏差，则校准，秒钟不计，校准字节长度为5 */
	if ((0 != memcmp(&eTime.date, &time->date, 3)
			|| (0 != memcmp(&eTime.time, &time->time, 2))))
	{
		RT_SetRealTime(&eTime);
	}
}
