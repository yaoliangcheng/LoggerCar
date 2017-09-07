#include "rt.h"

#include "common.h"

#include "RealTime.h"
#include "osConfig.h"

/******************************************************************************/
RT_TimeTypedef RT_RealTime;

/******************************************************************************/
static void RT_SetRealTime(RT_TimeTypedef* time);
static void RT_BKUP_ReadDate(void);

/*******************************************************************************
 * function：实时时钟初始化
 */
void RT_Init(void)
{
	if (HAL_RTCEx_BKUPRead(&RT_RTC, RTC_BKUP_REG_DATA) != RTC_BKUP_DATA)
	{
		RT_RealTime.date.Year = 17;
		RT_RealTime.date.Month = RTC_MONTH_AUGUST;
		RT_RealTime.date.Date = 17;
		RT_RealTime.date.WeekDay = RTC_WEEKDAY_THURSDAY;
		RT_RealTime.time.Hours = 10;
		RT_RealTime.time.Minutes = 55;
		RT_RealTime.time.Seconds = 0x00;
		RT_SetRealTime(&RT_RealTime);

		HAL_RTCEx_BKUPWrite(&RT_RTC, RTC_BKUP_REG_DATA, RTC_BKUP_DATA);
	}
	else
	{
		RT_BKUP_ReadDate();
		HAL_RTC_GetDate(&RT_RTC, &RT_RealTime.date, RTC_FORMAT_BIN);
	}
}

/*******************************************************************************
 * function：将当前日期更新到备份区域，并记录下当前星期值，便于更新日期
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
 * 与云平台时间校准
 * @pBuffer：接收缓存数据（平台回文）
 * @pStruct：发送结构体
 */
void RT_TimeAdjustWithCloud(uint8_t* pBuffer)
{
	uint8_t str[10] = {0};

	RT_TimeTypedef   eTime;

	/* 将字符转换成数字 */
	str2numb((pBuffer + RT_OFFSET_CLOUD_TIME), str, sizeof(str));

	eTime.date.Year    = (str[0]  * 10)  + str[1];
	eTime.date.Month   = (str[2]  * 10)  + str[3];
	eTime.date.Date    = (str[4]  * 10)  + str[5];
	eTime.time.Hours   = (str[6]  * 10)  + str[7];
	eTime.time.Minutes = (str[8]  * 10)  + str[9];

	/* 接收到平台回文，与发送时间比较，若相差年月日时分有偏差，则校准，秒钟不计，校准字节长度为5 */
	if ((0 != memcmp(&eTime.date, &RT_RealTime.date, 3)
			|| (0 != memcmp(&eTime.time, &RT_RealTime.time, 2))))
	{
		RT_SetRealTime(&eTime);
	}
}

/*******************************************************************************
 * function：秒中断回调函数
 */
void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
	osSignalSet(realtimeTaskHandle, REALTIME_TASK_SIGNAL_UPDATE);
}



/*******************************************************************************
 * function：从备份区域读出当前日期
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
 * function:设置当前时间，并将当前时间的日期更新到备份区域
 * time：当前时间值
 */
static void RT_SetRealTime(RT_TimeTypedef* time)
{
	HAL_RTC_SetDate(&RT_RTC, &time->date, RTC_FORMAT_BIN);
	HAL_RTC_SetTime(&RT_RTC, &time->time, RTC_FORMAT_BIN);

	/* 将更新的日期备份 */
	RT_BKUP_UpdateDate(time);
}
