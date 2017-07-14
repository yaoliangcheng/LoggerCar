#include "../Inc/MainProcess.h"

#include "osConfig.h"
#include "analog.h"
#include "RealTime.h"

#include "exFlash.h"

/*******************************************************************************
 *
 */
void MAINPROCESS_Task(void)
{
	osEvent signal;
	ANALOG_ValueTypedef AnalogValue;
	RT_TimeTypedef *time;
	exFLASH_InfoTypedef flashInfo;

//	exFLASH_ChipErase();

	while(1)
	{

		signal = osMessageGet(realtimeMessageQId, 100);
		time = (RT_TimeTypedef*)signal.value.v;

		/* 触发ADC采样 */
		ANALOG_ConvertEnable();

		/* 等待ADC采样完成 */
		signal = osSignalWait(MAINPROCESS_GET_SENSOR_ENABLE, 1000);
		if ((signal.value.signals & MAINPROCESS_GET_SENSOR_ENABLE)
						!= MAINPROCESS_GET_SENSOR_ENABLE)
		{
			printf("MainProcess信号等待失败,超时\r\n");
			/* 将自己挂起 */
			osThreadSuspend(NULL);
		}
		
		/* 获取传感器的值 */
		ANALOG_GetSensorValue(&AnalogValue);
		printf("温度1 = %f\r\n", AnalogValue.temp1);
		printf("温度2 = %f\r\n", AnalogValue.temp2);
		printf("温度3 = %f\r\n", AnalogValue.temp3);
		printf("温度4 = %f\r\n", AnalogValue.temp4);
		printf("湿度1 = %f\r\n", AnalogValue.humi1);
		printf("湿度2 = %f\r\n", AnalogValue.humi2);
		printf("湿度3 = %f\r\n", AnalogValue.humi3);
		printf("湿度4 = %f\r\n", AnalogValue.humi4);
		printf("电池电量 = %d\r\n", AnalogValue.batVoltage);


		/* 获取外部电源状态 */
		/* todo */

		/* 获取定位数据 */
		/* todo */

		/* 从信号队列中获取时间值 */
		/* todo */

		/* 记录数值 */
		exFLASH_SaveStructInfo(time, &AnalogValue, FORMAT_ONE_DECIMAL);

		/* 读取数值 */
		exFLASH_ReadStructInfo(&flashInfo);
		printf("当前时间是%02X.%02X.%02X %02X:%02X:%02X\r\n", time->date.Year,
				time->date.Month, time->date.Date,
				time->time.Hours,time->time.Minutes,
				time->time.Seconds);
		printf("储存时间是%02X.%02X.%02X %02X:%02X:%02X\r\n", flashInfo.realTime.year,
				flashInfo.realTime.month,flashInfo.realTime.day,
				flashInfo.realTime.hour,flashInfo.realTime.min,
				flashInfo.realTime.sec);

		/* 通过GPRS上传到平台 */
		/* todo */

		/* 任务运行完毕，一定要将自己挂起 */
		osThreadSuspend(NULL);

	}
}


