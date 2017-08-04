#include "debug.h"

/******************************************************************************/
void DEBUG_Task(void)
{

	while(1)
	{

		printf("FreeRTOS正在运行......\r\n");

		osDelay(10000);

	}
}


