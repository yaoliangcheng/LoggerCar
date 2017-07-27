#include "../Inc/MainProcess.h"

#include "exFlash.h"
#include "analog.h"
#include "input.h"
#include "gps.h"

#include "osConfig.h"
#include "RealTime.h"



/*******************************************************************************
 *
 */
void MAINPROCESS_Task(void)
{
	osEvent signal;
	ANALOG_ValueTypedef AnalogValue;
	RT_TimeTypedef *time;
	exFLASH_InfoTypedef sendInfo;
	GPS_LocationTypedef* location;

	while(1)
	{		
		signal = osMessageGet(realtimeMessageQId, 100);
		time = (RT_TimeTypedef*)signal.value.v;

		/* 触发ADC采样 */
		ANALOG_ConvertEnable();

		/* 等待ADC采样完成 */
		signal = osSignalWait(MAINPROCESS_SENSOR_CONVERT_FINISH, MAINPROCESS_TICKS_TO_TIMEOUT);
		if ((signal.value.signals & MAINPROCESS_SENSOR_CONVERT_FINISH)
						!= MAINPROCESS_SENSOR_CONVERT_FINISH)
		{
			printf("MainProcess等待ADC采样完成信号等待失败,超时\r\n");
			/* 将自己挂起 */
			osThreadSuspend(NULL);
		}
		
		printf("当前时间是%02X.%02X.%02X %02X:%02X:%02X\r\n", time->date.Year,
				time->date.Month, time->date.Date,
				time->time.Hours,time->time.Minutes,
				time->time.Seconds);

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
		/* 激活GPRSProcess任务，启动GPS转换 */
		osThreadResume(gprsprocessTaskHandle);
		/* 等待GPS完成 */
		signal = osSignalWait(MAINPROCESS_GPS_CONVERT_FINISH, osWaitForever);
		if ((signal.value.signals & MAINPROCESS_GPS_CONVERT_FINISH)
						!= MAINPROCESS_GPS_CONVERT_FINISH)
		{
			printf("GPS定位失败\r\n");
			/* 将自己挂起 */
			osThreadSuspend(NULL);
		}

		/* 获取定位值 */
		signal = osMessageGet(infoMessageQId, 100);
		location = (GPS_LocationTypedef*)signal.value.v;

		/* 记录数值 */
		exFLASH_SaveStructInfo(&sendInfo, time, &AnalogValue, FORMAT_ONE_DECIMAL, location);

		/* 读取数值 */
//		exFLASH_ReadStructInfo(&flashInfo);

		/* 通过GPRS上传到平台 */
		/* 传递发送结构体 */
		osMessagePut(infoMessageQId, (uint32_t)&sendInfo, 100);
		/* 激活MainProcess任务 */
		osThreadResume(gprsprocessTaskHandle);

		/* 等待GPRSProcess完成 */
		signal = osSignalWait(MAINPROCESS_GPRS_SEND_FINISHED, osWaitForever);
		if ((signal.value.signals & MAINPROCESS_GPRS_SEND_FINISHED)
						!= MAINPROCESS_GPRS_SEND_FINISHED)
		{
			printf("MainProcess等待GPRSProcess完成 信号等待失败,超时\r\n");
			/* 将自己挂起 */
			osThreadSuspend(NULL);
		}

		/* 任务运行完毕，一定要将自己挂起 */
		osThreadSuspend(NULL);
	}
}


