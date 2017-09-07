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
	uint8_t curPatchPack;

	RT_TimeTypedef time;
	GPS_LocateTypedef* location;

	FILE_PatchPackTypedef patchPack;

	FILE_SaveInfoTypedef saveInfoStruct;		/* 储存的结构体变量 */
	FILE_SaveInfoTypedef readInfoStruct[GPRS_PATCH_PACK_NUMB_MAX];

	while(1)
	{		
		/* 获取时间 */
		signal = osMessageGet(realtimeMessageQId, 1000);
		memcpy(&time, (uint32_t*)signal.value.v, sizeof(RT_TimeTypedef));

		/* 时间转换成ASCII */
		HEX2ASCII(&time.date.Year,    (uint8_t*)&saveInfoStruct.year[0],  1);
		HEX2ASCII(&time.date.Month,   (uint8_t*)&saveInfoStruct.month[0], 1);
		HEX2ASCII(&time.date.Date,    (uint8_t*)&saveInfoStruct.day[0],   1);
		HEX2ASCII(&time.time.Hours,   (uint8_t*)&saveInfoStruct.hour[0],  1);
		HEX2ASCII(&time.time.Minutes, (uint8_t*)&saveInfoStruct.min[0],   1);
		HEX2ASCII(&time.time.Seconds, (uint8_t*)&saveInfoStruct.sec[0],   1);

		/* 获取外部电源状态 */
		saveInfoStruct.exPwrStatus = INPUT_CheckPwrOnStatus() + '0';

		/* 模拟量转换为ASCII */
		sprintf((char*)&saveInfoStruct.analogValue[0].value, "%5.1f", ANALOG_value.temp1);
		sprintf((char*)&saveInfoStruct.analogValue[1].value, "%5.1f", ANALOG_value.humi1);
		sprintf((char*)&saveInfoStruct.analogValue[2].value, "%5.1f", ANALOG_value.temp2);
		sprintf((char*)&saveInfoStruct.analogValue[3].value, "%5.1f", ANALOG_value.humi2);
		sprintf((char*)&saveInfoStruct.analogValue[4].value, "%5.1f", ANALOG_value.temp3);
		sprintf((char*)&saveInfoStruct.analogValue[5].value, "%5.1f", ANALOG_value.humi3);
		sprintf((char*)&saveInfoStruct.analogValue[6].value, "%5.1f", ANALOG_value.temp4);
		sprintf((char*)&saveInfoStruct.analogValue[7].value, "%5.1f", ANALOG_value.humi4);
		sprintf((char*)&saveInfoStruct.batQuality[0],        "%3d",   ANALOG_value.batVoltage);

		/* 获取定位数据 */
		osMessagePut(gprsTaskMessageQid, START_TASK_GPS, 1000);

		/* 等待GPS完成,因为这个过程可能要启动GSM模块，所以等待周期必须长点，20s */
		signal = osSignalWait(MAINPROCESS_GPS_CONVERT_FINISH, 20000);
		if ((signal.value.signals & MAINPROCESS_GPS_CONVERT_FINISH)
						== MAINPROCESS_GPS_CONVERT_FINISH)
		{
			/* 获取定位值 */
			signal = osMessageGet(infoMessageQId, 100);
			location = (GPS_LocateTypedef*)signal.value.v;

			/* 定位值转换成ASCII */
			sprintf((char*)&saveInfoStruct.longitude[0], "%10.5f", location->longitude);
			sprintf((char*)&saveInfoStruct.latitude[0],  "%10.5f",  location->latitude);
		}

		/* 储存信息符号初始化 */
		FILE_SaveInfoSymbolInit(&saveInfoStruct);

		/* 记录数据 */
		FILE_SaveInfo(&saveInfoStruct);

		/* 读取补传数据条数 */
		/* 读取成功，则表明曾经有补传数据记录 */
		if (SUCCESS == FILE_ReadFile(FILE_NAME_PATCH_PACK, 0,
				(uint8_t*)&patchPack, sizeof(FILE_PatchPackTypedef)))
		{
			curPatchPack = FILE_ReadInfo(readInfoStruct, &patchPack);
		}
		printf("本次上传数据条数=%d\r\n", curPatchPack);

		/* 根据协议，格式转换 */
		FILE_SendInfoFormatConvert((uint8_t*)readInfoStruct,
				(uint8_t*)&GPRS_SendBuffer.dataPack[0], curPatchPack);

		/* 传递本次发送的数据条数，注意：curPatchPack是以数据形式传递，不是传递指针 */
		osMessagePut(infoCntMessageQId, (uint16_t)curPatchPack, 1000);

		/* 使能MainProcess任务发送数据 */
		osMessagePut(gprsTaskMessageQid, START_TASK_GPRS, 1000);

		/* 等待GPRSProcess完成 */
		signal = osSignalWait(MAINPROCESS_GPRS_SEND_FINISHED, 20000);
		if ((signal.value.signals & MAINPROCESS_GPRS_SEND_FINISHED)
						!= MAINPROCESS_GPRS_SEND_FINISHED)
		{
			printf("发送数据超时，说明数据发送失败，记录数据等待补传\r\n");

			/* 如果是本次发生的补传，则记录时间，否则是正在补传的过程，保持补传文件内容不变 */
			if (curPatchPack == 1)
			{
				/* 记录当前文件前一次位置 */
				patchPack.patchStructOffset = FILE_DataSaveStructCnt - 1;

				FILE_WriteFile(FILE_NAME_PATCH_PACK, 0,
						(uint8_t*)&patchPack, sizeof(FILE_PatchPackTypedef));
			}
		}
		else
		{
			/* 数据发送成功 */
			printf("数据发送到平台成功！！\r\n");

			/* 有补传数据 */
			if (curPatchPack > 1)
				FILE_WriteFile(FILE_NAME_PATCH_PACK, 0,
						(uint8_t*)&patchPack, sizeof(FILE_PatchPackTypedef));
		}

		osThreadSuspend(NULL);
	}
}


