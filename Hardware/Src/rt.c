#include "rt.h"

#include "../Inc/public.h"
#include "RealTime.h"
#include "param.h"

/******************************************************************************/
RT_TimeTypedef RT_RealTime;						/* 实时时间 */
RT_TimeTypedef RT_RecordTime;					/* 记录时间 */

/******************************************************************************/
extern PARAM_DeviceParamTypedef PARAM_DeviceParam;
extern osThreadId realtimeTaskHandle;

/******************************************************************************/
static void RT_SetRealTime(RT_TimeTypedef* time);
static void RT_BKUP_ReadDate(void);
static void RT_SetAlarmTimeInterval(uint8_t interval);

/*******************************************************************************
 * function：实时时钟初始化
 */
void RT_Init(void)
{
	RT_BKUP_ReadDate();
	HAL_RTC_GetDate(&RT_RTC, &RT_RealTime.date, RTC_FORMAT_BIN);

	RT_SetAlarmTimeInterval(PARAM_DeviceParam.recordInterval);
	/* 清除闹钟中断标志位 */
	__HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);
	/* 配置闹钟中断 */
	__HAL_RTC_ALARM_ENABLE_IT(&hrtc,RTC_IT_ALRA);
	/* 配置EXTI中断 */
	__HAL_RTC_ALARM_EXTI_ENABLE_IT();
	/* EXTI上升沿产生中断 */
	__HAL_RTC_ALARM_EXTI_ENABLE_RISING_EDGE();

	/* 使能定时器和中断，触发采样 */
	__HAL_TIM_ENABLE(&RT_TIM);
	__HAL_TIM_ENABLE_IT(&RT_TIM, TIM_IT_UPDATE);
	/* 开机触发一次采样 */
	osSignalSet(realtimeTaskHandle, REALTIME_TASK_TIME_ANALOG_UPDATE);
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
	uint8_t str[12] = {0};

	RT_TimeTypedef   eTime;

	/* 将字符转换成数字 */
	str2numb(str, (pBuffer + RT_OFFSET_CLOUD_TIME), sizeof(str));

	eTime.date.Year    = (str[0]  * 10)   + str[1];
	eTime.date.Month   = (str[2]  * 10)   + str[3];
	eTime.date.Date    = (str[4]  * 10)   + str[5];
	eTime.time.Hours   = (str[6]  * 10)   + str[7];
	eTime.time.Minutes = (str[8]  * 10)   + str[9];
	eTime.time.Seconds = (str[10]  * 10)  + str[11];

	/* 接收到平台回文，与发送时间比较，若相差年月日时分有偏差，则校准，秒钟不计，校准字节长度为5 */
	if ((0 != memcmp(&eTime.date, &RT_RealTime.date, 3)
			|| (0 != memcmp(&eTime.time, &RT_RealTime.time, 2))))
	{
		RT_SetRealTime(&eTime);
	}
}

/*******************************************************************************
 * @brief 闹钟中断回调函数，更新下次闹钟时间吗，触发记录
 */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	/* 更新下次记录时间点 */
	RT_SetAlarmTimeInterval(PARAM_DeviceParam.recordInterval);
	osSignalSet(realtimeTaskHandle, REALTIME_TASK_ALRAM_RECORD);
}

/*******************************************************************************
 * @brief 设置闹钟时间间隔
 * @param interval：下一次闹钟间隔 单位：分钟
 */
static void RT_SetAlarmTimeInterval(uint8_t interval)
{
	uint16_t high = 0U, low = 0U;
	uint32_t alarmCounter = 0U;

	high = READ_REG(hrtc.Instance->CNTH & RTC_CNTH_RTC_CNT);
	low  = READ_REG(hrtc.Instance->CNTL & RTC_CNTL_RTC_CNT);

	alarmCounter = (((uint32_t) high << 16U) | low);

	/* 分钟转换为秒 */
	alarmCounter += (interval * 60);

	/* 等待上次对RTC写操作完成 */
	while((hrtc.Instance->CRL & RTC_CRL_RTOFF) == (uint32_t)RESET);
	/* 进入配置模式 */
	__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);

	WRITE_REG(hrtc.Instance->ALRH, (alarmCounter >> 16U));
	WRITE_REG(hrtc.Instance->ALRL, (alarmCounter & RTC_ALRL_RTC_ALR));

	/* 退出配置模式 */
	__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);
	while((hrtc.Instance->CRL & RTC_CRL_RTOFF) == (uint32_t)RESET);
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
	HAL_RTC_GetDate(&hrtc, &RT_RealTime.date, RTC_FORMAT_BIN);
}
