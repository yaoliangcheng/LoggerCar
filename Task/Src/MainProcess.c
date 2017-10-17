#include "../Inc/MainProcess.h"
#include "public.h"
#include "osConfig.h"

#include "GPRSProcess.h"

#include "rt.h"
#include "gps.h"
#include "file.h"
#include "input.h"
#include "analog.h"

/******************************************************************************/
extern FILE_SaveStructTypedef FILE_SaveStruct;
extern FILE_SaveStructTypedef FILE_ReadStruct[GPRS_PATCH_PACK_NUMB_MAX];
extern FILE_PatchPackTypedef FILE_PatchPack;		/* 补传文件信息 */

/*******************************************************************************
 *
 */
void MAINPROCESS_Task(void)
{
	osEvent signal;
	uint8_t curPatchPack;

	RT_TimeTypedef time;
	GPS_LocateTypedef* location;

	while(1)
	{		
		/* 获取时间 */
		signal = osMessageGet(realtimeMessageQId, 1000);
		memcpy(&time, (uint32_t*)signal.value.v, sizeof(RT_TimeTypedef));

		printf("时间：%d.%d.%d %d:%d:%d\r\n", RT_RecordTime.date.Year, RT_RecordTime.date.Month,
				RT_RecordTime.date.Date, RT_RecordTime.time.Hours, RT_RecordTime.time.Minutes, RT_RecordTime.time.Seconds);

		/* 激活 */
//		osThreadResume(gprsprocessTaskHandle);
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
		}

		/* 时间转换成ASCII */
		HEX2ASCII((uint8_t*)&FILE_SaveStruct.year[0],  &RT_RecordTime.date.Year,    1);
		HEX2ASCII((uint8_t*)&FILE_SaveStruct.month[0], &RT_RecordTime.date.Month,   1);
		HEX2ASCII((uint8_t*)&FILE_SaveStruct.day[0],   &RT_RecordTime.date.Date,    1);
		HEX2ASCII((uint8_t*)&FILE_SaveStruct.hour[0],  &RT_RecordTime.time.Hours,   1);
		HEX2ASCII((uint8_t*)&FILE_SaveStruct.min[0],   &RT_RecordTime.time.Minutes, 1);
		HEX2ASCII((uint8_t*)&FILE_SaveStruct.sec[0],   &RT_RecordTime.time.Seconds, 1);
		/* 获取外部电源状态 */
		FILE_SaveStruct.exPwrStatus = INPUT_CheckPwrOnStatus() + '0';

		/* 模拟量转换为ASCII */
		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[0], ANALOG_value.temp1);
		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[1], ANALOG_value.humi1);
		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[2], ANALOG_value.temp2);
		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[3], ANALOG_value.humi2);
		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[4], ANALOG_value.temp3);
		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[5], ANALOG_value.humi3);
		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[6], ANALOG_value.temp4);
		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[7], ANALOG_value.humi4);

		sprintf((char*)&FILE_SaveStruct.batQuality[0], "%3d", ANALOG_value.batVoltage);
		/* 定位值转换成ASCII */
		sprintf((char*)&FILE_SaveStruct.longitude[0], "%10.5f", location->longitude);
		sprintf((char*)&FILE_SaveStruct.latitude[0],  "%10.5f",  location->latitude);
		/* CVS文件格式 */
		FILE_SaveStruct.batQuality[3]	   = '%';		/* 电池电量百分号 */
		FILE_SaveStruct.str7   			   = ',';
		FILE_SaveStruct.str8   			   = ',';

		/* 储存数据 */
		FILE_SaveInfo();

		/* 读取补传数据条数 */
		/* 读取成功，则表明曾经有补传数据记录 */
		FILE_ReadFile(FILE_NAME_PATCH_PACK, 0,
				(uint8_t*)&FILE_PatchPack, sizeof(FILE_PatchPackTypedef));
		curPatchPack = FILE_ReadInfo(&FILE_PatchPack);

		FILE_SendInfoFormatConvert((uint8_t*)GPRS_SendBuffer.dataPack, 
								   (uint8_t*)FILE_ReadStruct, curPatchPack);
		GPRS_SendBuffer.dataPackNumbL = curPatchPack;

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
				FILE_PatchPack.patchStructOffset = FILE_DataSaveStructCnt - 1;

				FILE_WriteFile(FILE_NAME_PATCH_PACK, 0,
						(uint8_t*)&FILE_PatchPack, sizeof(FILE_PatchPackTypedef));
			}
		}
		else
		{
			/* 数据发送成功 */
			printf("数据发送到平台成功！！\r\n");

			/* 有补传数据 */
			if (curPatchPack > 1)
				FILE_WriteFile(FILE_NAME_PATCH_PACK, 0,
						(uint8_t*)&FILE_PatchPack, sizeof(FILE_PatchPackTypedef));
		}
		osThreadSuspend(NULL);
	}
}


