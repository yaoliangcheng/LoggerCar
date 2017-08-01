#include "../Inc/MainProcess.h"

#include "exFlash.h"
#include "analog.h"
#include "input.h"
#include "gps.h"
#include "file.h"

#include "osConfig.h"
#include "RealTime.h"
#include "GPRSProcess.h"

/*******************************************************************************
 *
 */
void MAINPROCESS_Task(void)
{
	osEvent signal;

	RT_TimeTypedef *time;
	GPS_LocateTypedef* location;
	ANALOG_ValueTypedef AnalogValue;

	FILE_InfoTypedef saveInfo;
	FILE_InfoTypedef readInfo[5];

	/* 文件名格式初始化 */
	FILE_Init();

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

		/* 获取定位数据 */
		/* 激活GPRSProcess任务，启动GPS转换 */
		osThreadResume(gprsprocessTaskHandle);
 		osSignalSet(gprsprocessTaskHandle, GPRSPROCESS_GPS_ENABLE);
		
		/* 等待GPS完成,因为这个过程可能要启动GSM模块，所以等待周期必须长点，100s */
		signal = osSignalWait(MAINPROCESS_GPS_CONVERT_FINISH, 100000);
		if ((signal.value.signals & MAINPROCESS_GPS_CONVERT_FINISH)
						!= MAINPROCESS_GPS_CONVERT_FINISH)
		{
			printf("GPS定位失败\r\n");
		}
		else
		{
			/* 获取定位值 */
			signal = osMessageGet(infoMessageQId, 100);
			location = (GPS_LocateTypedef*)signal.value.v;
		}

		/* 将数据格式转换成协议格式 */
		FILE_InfoFormatConvert(&saveInfo, time, location, &AnalogValue);

		/* 读取补传数据条数 */
		/* todo */

		/* 储存并读取数据 */
		FILE_SaveReadInfo(&saveInfo, readInfo, 1);

		/* 通过GPRS上传到平台 */
		/* 传递发送结构体 */
		osMessagePut(infoMessageQId, (uint32_t)&readInfo, 100);

		/* 把当前时间传递到GPRS进程，根据回文校准时间 */
		osMessagePut(realtimeMessageQId, (uint32_t)time, 100);

		/* 使能MainProcess任务发送数据 */
		osSignalSet(gprsprocessTaskHandle, GPRSPROCESS_SEND_DATA_ENABLE);

		/* 等待GPRSProcess完成 */
		signal = osSignalWait(MAINPROCESS_GPRS_SEND_FINISHED, 60000);
		if ((signal.value.signals & MAINPROCESS_GPRS_SEND_FINISHED)
						!= MAINPROCESS_GPRS_SEND_FINISHED)
		{
			printf("发送数据超时，说明数据发送失败，记录数据等待补传\r\n");
			/* 记录补传数据条数 */
			/* todo */
		}

		/* 任务运行完毕，一定要将自己挂起 */
		osThreadSuspend(NULL);
	}
}


