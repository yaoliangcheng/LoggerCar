#ifndef __GPS_H
#define __GPS_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

/******************************************************************************/
#define GPS_FLAG_VALID					('A')			/* 数据有效 */
#define GPS_FLAG_INVALID				('V')			/* 数据无效 */

#define GPS_OFFSET_FLAG					(18)			/* 数据有效标志位偏移 */
#define GPS_OFFSET_LATITUDE				(20)			/* 纬度值偏移 */
#define GPS_OFFSET_LATITUDE_FLAG		(30)			/* 纬度值标志位N、S */
#define GPS_OFFSET_LONGITUDE			(32)			/* 经度值偏移 */
#define GPS_OFFSET_LONGITUDE_FLAG		(43)			/* 经度值标志位E、W */

/*******************************************************************************
 *
 */
typedef enum
{
	GPS_LATITUDE,
	GPS_LONGITUDE
} GPS_LocationEnum;

typedef enum
{
	GPS_LOCATION_TYPE_BASE_STATION,					/* 基站定位 */
	GPS_LOCATION_TYPE_GPS,							/* GPS定位 */
} GPS_LocationTypeEnum;								/* 定位类型 */

typedef struct
{
	double latitude;
	double longitude;
} GPS_LocateTypedef;

/******************************************************************************/
void GPS_GetLocation(uint8_t* buf, GPS_LocateTypedef* info);

#endif
