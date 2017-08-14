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

	RT_TimeTypedef time;
	GPS_LocateTypedef* location;
	ANALOG_ValueTypedef* AnalogValue;

	FILE_InfoTypedef saveInfo;
	FILE_InfoTypedef readInfo[GPRS_PATCH_PACK_NUMB_MAX];

	FILE_PatchPackTypedef patchPack;
	uint16_t curPatchPack;				/* 本次补传数据 */
	uint16_t curFileStructCount;		/* 当前文件结构体总数 */



	while(1)
	{		
		/* 获取时间 */
		signal = osMessageGet(realtimeMessageQId, 5000);
		memcpy(&time, (uint32_t*)signal.value.v, sizeof(RT_TimeTypedef));

		/* 获取模拟量数据 */
		signal = osMessageGet(analogMessageQId, 2000);
		AnalogValue = (ANALOG_ValueTypedef*)signal.value.v;

		/* 获取定位数据 */
		/* 激活GPRSProcess任务，启动GPS转换 */
		osThreadResume(gprsprocessTaskHandle);
		osSignalSet(gprsprocessTaskHandle, GPRSPROCESS_GPS_ENABLE);

		/* 等待GPS完成,因为这个过程可能要启动GSM模块，所以等待周期必须长点，30s */
		/* 刚启动模块的第一次定位耗时长，也不能得到定位数据，可以放弃 */
		signal = osSignalWait(MAINPROCESS_GPS_CONVERT_FINISH, 30000);
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
		FILE_InfoFormatConvert(&saveInfo, &time, location, AnalogValue);

		printf("当前记录时间是%02x.%02x.%02x %02x:%02x:%02x \r\n",
				time.date.Year,  time.date.Month,   time.date.Date,
				time.time.Hours, time.time.Minutes, time.time.Seconds);

		printf("模拟量数据是：%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\r\n,",
				AnalogValue->temp1, AnalogValue->temp2, AnalogValue->temp3, AnalogValue->temp4,
				AnalogValue->humi1, AnalogValue->humi2, AnalogValue->humi3, AnalogValue->humi4);

		/* 记录数据 */
		FILE_SaveInfo(&saveInfo, &curFileStructCount);

		/* 读取补传数据条数 */
		/* 读取成功，则表明曾经有补传数据记录 */
		if (SUCCESS == FILE_ReadPatchPackFile(&patchPack))
		{
			/* 补传数据全部上传完毕 */
			if (memcmp(patchPack.patchFileName, "\0\0\0\0\0\0", 6) != 0)
			{
				printf("读取补传文件名是%11s,补传开始结构体偏移=%d \r\n",
						patchPack.patchFileName, patchPack.patchStructOffset);
				curPatchPack = FILE_ReadPatchInfo(&patchPack, readInfo);
			}
			else
			{
				FILE_ReadInfo(readInfo);
				curPatchPack = 1;
			}
		}
		/* 读取文件不成功，则说明该文件还未创建 */
		else
		{
			FILE_ReadInfo(readInfo);
			curPatchPack = 1;
		}
		printf("本次上传数据条数=%d\r\n", curPatchPack);

		/* 通过GPRS上传到平台 */
		/* 传递发送结构体 */
		osMessagePut(infoMessageQId, (uint32_t)&readInfo, 1000);

		/* 传递本次发送的数据条数，注意：curPatchPack是以数据形式传递，不是传递指针 */
		osMessagePut(infoCntMessageQId, (uint16_t)curPatchPack, 1000);

		/* 把时间传递到GPRS进程，便于根据平台回文校准时间 */
		osMessagePut(adjustTimeMessageQId, (uint32_t)&time, 1000);

		/* 使能MainProcess任务发送数据 */
		osSignalSet(gprsprocessTaskHandle, GPRSPROCESS_SEND_DATA_ENABLE);

		/* 等待GPRSProcess完成 */
		signal = osSignalWait(MAINPROCESS_GPRS_SEND_FINISHED, 30000);
		if ((signal.value.signals & MAINPROCESS_GPRS_SEND_FINISHED)
						!= MAINPROCESS_GPRS_SEND_FINISHED)
		{
			printf("发送数据超时，说明数据发送失败，记录数据等待补传\r\n");

			/* 如果是本次发生的补传，则记录时间，否则是正在补传的过程，保持补传文件内容不变 */
			if (curPatchPack == 1)
			{
				/* 根据时间生成文件名 */
				BCD2ASCII(&patchPack.patchFileName[0], &time.date.Year,  1);
				BCD2ASCII(&patchPack.patchFileName[2], &time.date.Month, 1);
				BCD2ASCII(&patchPack.patchFileName[4], &time.date.Date,  1);
				/* 补传信息文件名后缀 */
				memcpy(&patchPack.patchFileName[6], ".txt\0", 5);

				/* 记录当前文件前一次位置 */
				patchPack.patchStructOffset = curFileStructCount - 1;

				FILE_WritePatchPackFile(&patchPack);
			}
		}
		else
		{
			/* 数据发送成功 */
			printf("数据发送到平台成功！！\r\n");

			/* 有补传数据 */
			if (curPatchPack > 1)
				FILE_WritePatchPackFile(&patchPack);
		}

		osThreadSuspend(NULL);
	}
}


