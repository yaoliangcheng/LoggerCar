#ifndef __RT_H
#define __RT_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "public.h"

#include "rtc.h"

/******************************************************************************/
#define RT_RTC						(hrtc)

/******************************************************************************/
#define RTC_BKUP_DATA				(0xA0A0)

#define RTC_BKUP_REG_DATA			(RTC_BKP_DR2)
#define RTC_BKUP_REG_YEAR			(RTC_BKP_DR3)
#define RTC_BKUP_REG_MONTH			(RTC_BKP_DR4)
#define RTC_BKUP_REG_DAY			(RTC_BKP_DR5)
#define RTC_BKUP_REG_WEEK			(RTC_BKP_DR6)

/******************************************************************************/
#define RT_OFFSET_CLOUD_TIME		(8)				/* 平台回文时间偏移 */

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)											/* 按字节对齐 */

typedef struct
{
	RTC_DateTypeDef date;
	RTC_TimeTypeDef time;
	uint8_t         oldWeekDay;				/* 上次记录星期,用于更新日期 */
} RT_TimeTypedef;

#pragma pack(pop)

/******************************************************************************/
extern RT_TimeTypedef RT_RealTime;
extern RT_TimeTypedef RT_RecordTime;					/* 记录时间 */
extern BOOL RT_recordFlag;

/******************************************************************************/
void RT_Init(void);
void RT_BKUP_UpdateDate(RT_TimeTypedef* time);
void RT_TimeAdjustWithCloud(uint8_t* pBuffer);

#endif

