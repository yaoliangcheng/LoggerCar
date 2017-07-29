#include "debug.h"

#include "gps.h"
#include "exFlash.h"

/******************************************************************************/
void DEBUG_Task(void)
{
//	uint8_t buf[] = "&GNRMC,063651.729,A,1234.5689,N,98745.4320,W";
//	exFLASH_InfoTypedef info;

	while(1)
	{
		printf("FreeRTOS正在运行......\r\n");
//		GPS_GetLocation(buf, &info);
		osDelay(10000);
	}
}


