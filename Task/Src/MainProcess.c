#include "../Inc/MainProcess.h"
#include "public.h"

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
extern osMessageQId gprsTaskMessageQid;
extern GPS_LocateTypedef  GPS_Locate;

extern FunctionalState sendPackRecordEnable;			/* 发送数据包记录使能 */

extern uint16_t GPRS_SendPackSize;		/* GPRS发送包大小 */
extern GPRS_NewSendbufferTyepdef GPRS_NewSendbuffer;

/*******************************************************************************
 *
 */
void MAINPROCESS_Task(void)
{
	osEvent signal;
	uint8_t curPatchPack;

	while(1)
	{
		/* 获取定位数据 */
		osMessagePut(gprsTaskMessageQid, START_TASK_GPS, 1000);
		/* 等待GPS完成,因为这个过程可能要启动GSM模块，所以等待周期必须长点，20s */
		signal = osSignalWait(MAINPROCESS_GPS_CONVERT_FINISH, 20000);
		if ((signal.value.signals & MAINPROCESS_GPS_CONVERT_FINISH)
						== MAINPROCESS_GPS_CONVERT_FINISH)
		{
			/* 获取定位值 */
			/* 定位值转换成ASCII */
			sprintf((char*)&FILE_SaveStruct.longitude[0], "%10.5f", GPS_Locate.longitude);
			sprintf((char*)&FILE_SaveStruct.latitude[0],  "%10.5f", GPS_Locate.latitude);
		}
		else
		{
			memset((char*)&FILE_SaveStruct.longitude[0], 0, 10);
			memset((char*)&FILE_SaveStruct.latitude[0],  0, 10);
		}

		/* 如果是需要保存的，先保存数据 */
		if (sendPackRecordEnable == ENABLE)
		{
			FILE_SaveSendInfo(&FILE_SaveStruct, &RT_RecordTime, &GPS_Locate, &ANALOG_value);
		}

		/* 读取补传文件 */
		FILE_ReadFile(FILE_NAME_PATCH_PACK, 0,
				(uint8_t*)&FILE_PatchPack, sizeof(FILE_PatchPackTypedef));
		/* 如果有断点数据 */
//		if (FILE_PatchPack.patchStructOffset != 0)
//		{
//			/* 读取从断点开始的数据，返回当前读出的包数 */
//			curPatchPack =
//					FILE_ReadSaveInfo(FILE_ReadStruct, FILE_PatchPack.patchStructOffset);
//			GPRS_SendPackSize = GPRS_SendDataPackFromRecord(&GPRS_NewSendbuffer,
//					FILE_ReadStruct, curPatchPack, &RT_RealTime);
//			FILE_PatchPack.patchStructOffset += curPatchPack;
//		}
//		else /* 没有断点数据 */
		{
			/* 记录数据 */
			if (sendPackRecordEnable == ENABLE)
			{
				curPatchPack = FILE_ReadSaveInfo(FILE_ReadStruct, 0);
				GPRS_SendPackSize = GPRS_SendDataPackFromRecord(&GPRS_NewSendbuffer,
						FILE_ReadStruct, 1, &RT_RealTime);
			}
			else	/* 实时数据 */
			{
				GPRS_SendPackSize = GPRS_SendDataPackFromCurrent(&GPRS_NewSendbuffer,
						&RT_RealTime, &ANALOG_value, &GPS_Locate);
			}
		}

		/* 不管什么情况，均取消记录 */
		sendPackRecordEnable = DISABLE;

		/* 使能MainProcess任务发送数据 */
		osMessagePut(gprsTaskMessageQid, START_TASK_DATA, 1000);
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


