#include "print.h"
#include "ble.h"
#include "iwdg.h"

/******************************************************************************/
uint8_t PRINT_SendBuffer[PRINT_SEND_BYTES_MAX];
FILE_SaveStructTypedef PRINT_DataFileReadStruct;

/******************************************************************************/
static void PRINT_SendData(uint16_t size);
//static BOOL PRINT_GetAnalogAndAdjust(uint16_t analog, uint8_t* buf, float alarmUp, float alarmLow);
static PRINT_DataStatusEnum PRINT_DataPrint(uint64_t offset,
		DISPLAY_CompareTimeTypedef* endTimePoint,
		DISPLAY_CompareTimeTypedef* printDate,
		ChannelSelectTypedef* select);

static void PRINT_PrintTitle(void);
static void PRINT_PrintTail(void);
static void PRINT_SetMode(void);

/*******************************************************************************
 * @brief 打印数据
 * @param startTime：开始打印时间
 * @param endTime：结束打印时间
 * @select：打印通道选择
 */
void PRINT_PrintProcess(DISPLAY_CompareTimeTypedef* startTime,
						DISPLAY_CompareTimeTypedef* endTime,
						ChannelSelectTypedef* select)
{
	uint32_t offsetStruct;
	PRINT_DataStatusEnum status;
	DISPLAY_CompareTimeTypedef printDate;	/* 当前打印日期 */

	/* 清空当前打印日期 */
	memset((uint8_t*)&printDate, 0, sizeof(DISPLAY_CompareTimeTypedef));

	/* 根据开始打印时间寻找起始数据偏移量 */
	offsetStruct = PRINT_SearchStartTime(startTime);

	/* 设置打印模式 */
	PRINT_SetMode();
	/* 先打印标题 */
	PRINT_PrintTitle();

	/* 打印输出 */
	while (1)
	{
		status = PRINT_DataPrint(offsetStruct, endTime, &printDate, select);

		if (PRINT_DATA_END == status)
			break;
		else if (PRINT_DATA_NORMAL == status)
			offsetStruct += 5;
		else if (PRINT_DATA_OVERlIMITED == status)
			offsetStruct += 2;

		osDelay(100);

#if IWDG_ENABLE
		/* 看门狗监控 */
		HAL_IWDG_Refresh(&hiwdg);
#endif
	}

	/* 打印签名 */
	PRINT_PrintTail();
}

/*******************************************************************************
 * @brief 根据开始时间查找结构体偏移，返回结构体偏移
 * @param destTime：目标时间点
 */
uint32_t PRINT_SearchStartTime(DISPLAY_CompareTimeTypedef* destTime)
{
	uint32_t fileStructStart, fileStructEnd, searchPoint;
	int compareStatus;

	fileStructStart = 0;
	fileStructEnd   = FILE_DataSaveStructCnt;

	while(1)
	{
		/* 取中间值 */
		searchPoint = (fileStructStart + fileStructEnd) / 2;

		/* 如果介于两个结构体之间的，以本次读到的数据为准 */
		if (searchPoint == fileStructStart)
			break;

		FILE_ReadFile(FILE_NAME_SAVE_DATA, searchPoint * sizeof(FILE_SaveStructTypedef),
				(BYTE*)&PRINT_DataFileReadStruct, sizeof(FILE_SaveStructTypedef));
		/* 比较数据的年月日时分 */
		/* buf1<buf2 return -1
		 * buf1>buf2 return 1
		 * buf1=buf2 return 0 */
		compareStatus = memcmp(destTime, &PRINT_DataFileReadStruct,
				sizeof(DISPLAY_CompareTimeTypedef));

		/* 找到目标时间 */
		if (compareStatus == 0)
		{
			break;
		}
		else if (compareStatus < 0) /* 目标时间 < 读取时间 */
		{
			fileStructEnd = searchPoint;
		}
		else /* 目标时间 > 读取时间 */
		{
			fileStructStart = searchPoint;
		}
	}

	return searchPoint;
}

/*******************************************************************************
 * function:根据结构体偏移，读出数据，并根据打印通道选择，打印数据，并返回该数据是否超标
 */
static PRINT_DataStatusEnum PRINT_DataPrint(uint64_t offset,
		DISPLAY_CompareTimeTypedef* endTimePoint,
		DISPLAY_CompareTimeTypedef* printDate,
		ChannelSelectTypedef* select)
{
	uint8_t index = 0;
	PRINT_DataStatusEnum status = PRINT_DATA_OVERlIMITED;

	/* 读取数据 */
	FILE_ReadFile(FILE_NAME_SAVE_DATA, offset * sizeof(FILE_SaveStructTypedef),
			(uint8_t*)&PRINT_DataFileReadStruct, sizeof(FILE_SaveStructTypedef));

	/* 读出数据的时间 >= 结束时间点 */
	/* 为了防止用户选择的结束时间是当前的时间，但是当前时间并未记录数据，导致数据无限打印的现象 */
	if (memcmp(PRINT_DataFileReadStruct.year, endTimePoint, sizeof(DISPLAY_CompareTimeTypedef)) >= 0)
	{
		return PRINT_DATA_END;
	}

	/* 打印某天数据，在开始打印日期，后面打印只需打印时间即可 */
	if (memcmp(printDate, PRINT_DataFileReadStruct.year, 6) != 0)
	{
		/* 记录当前打印时间 */
		memcpy(printDate, PRINT_DataFileReadStruct.year, 6);
		PRINT_Date(PRINT_DataFileReadStruct.year);
	}

	/* 时分秒 */
	memcpy((char*)&PRINT_SendBuffer[0],  &PRINT_DataFileReadStruct.hour, 9);
	index += 9;

	/* 根据打印选择，输出数据 */
	if (select->status.bit.ch1)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[0], 6);
		/* 判断是否超标 */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[0], &PARAM_DeviceParam.chAlarmValue[0]);
		index += 6;
	}
	if (select->status.bit.ch2)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[1], 6);
		/* 判断是否超标 */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[1], &PARAM_DeviceParam.chAlarmValue[1]);
		index += 6;
	}
	if (select->status.bit.ch3)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[2], 6);
		/* 判断是否超标 */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[2], &PARAM_DeviceParam.chAlarmValue[2]);
		index += 6;
	}
	if (select->status.bit.ch4)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[3], 6);
		/* 判断是否超标 */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[3], &PARAM_DeviceParam.chAlarmValue[3]);
		index += 6;
	}
	if (select->status.bit.ch5)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[4], 6);
		/* 判断是否超标 */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[4], &PARAM_DeviceParam.chAlarmValue[4]);
		index += 6;
	}
	if (select->status.bit.ch6)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[5], 6);
		/* 判断是否超标 */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[5], &PARAM_DeviceParam.chAlarmValue[5]);
		index += 6;
	}
	if (select->status.bit.ch7)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[6], 6);
		/* 判断是否超标 */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[6], &PARAM_DeviceParam.chAlarmValue[6]);
		index += 6;
	}
	if (select->status.bit.ch8)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[7], 6);
		/* 判断是否超标 */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[7], &PARAM_DeviceParam.chAlarmValue[7]);
		index += 6;
	}

	/* 打印换行，最后一个数值不需要“，”，用换行覆盖 */
	PRINT_SendBuffer[index - 1] = '\n';

	PRINT_SendData(index);

	return status;
}

/*******************************************************************************
 *
 */
static void PRINT_SetMode(void)
{
	PRINT_SendBuffer[0] = 0x1B;
	PRINT_SendBuffer[1] = 0x7B;
	PRINT_SendBuffer[2] = 0;
	PRINT_SendData(3);
}

/*******************************************************************************
 *
 */
static void PRINT_SendData(uint16_t size)
{
//	HAL_UART_Transmit_DMA(&PRINT_UART, PRINT_SendBuffer, size);
	if (PRINT_MODE_INTEGRATED == DISPLAY_Status.printMode)
	{
		HAL_UART_Transmit(&PRINT_UART, PRINT_SendBuffer, size, 1000);
	}
	else if (PRINT_MODE_BLE_LINK == DISPLAY_Status.printMode)
	{
		HAL_UART_Transmit(&BLE_UART, PRINT_SendBuffer, size, 1000);
	}
}

/*******************************************************************************
 * function：打印标题数据
 */
static void PRINT_PrintTitle(void)
{
	uint8_t index = 0;

	memcpy(&PRINT_SendBuffer[0], "********************************\n", 33);
	index += 33;

	memcpy(&PRINT_SendBuffer[index], "收货方：\n", 9);
	index += 9;

	memcpy(&PRINT_SendBuffer[index], "发货方：\n", 9);
	index += 9;

	memcpy(&PRINT_SendBuffer[index], "派送车牌：\n", 11);
	index += 11;

	memcpy(&PRINT_SendBuffer[index], "订单编号：\n", 11);
	index += 11;

	PRINT_SendData(index);
}

/*******************************************************************************
 *
 */
static void PRINT_PrintTail(void)
{
	uint8_t index = 0;

	memcpy(&PRINT_SendBuffer[0], "********************************\n", 33);
	index += 33;

	memcpy(&PRINT_SendBuffer[index], "签收人：\n", 9);
	index += 9;

	memcpy(&PRINT_SendBuffer[index], "\n\n\n", 3);
	index += 3;

	memcpy(&PRINT_SendBuffer[index], "签收日期：\n", 11);
	index += 11;

	memcpy(&PRINT_SendBuffer[index], "\n\n\n\n\n\n\n\n\n", 9);
	index += 9;

	PRINT_SendData(index);
}

/*******************************************************************************
 * function：判断数值是否超标
 */
static PRINT_DataStatusEnum PRINT_AdjustOverLimited(FILE_SaveInfoAnalogTypedef* analog,
													ParamAlarmTypedef* param)
{
	float value;

	/* 转换成float */
	value = FILE_Analog2Float(analog);

	/* 比较上下限 */
	if ((value > param->alarmValueUp) || (value < param->alarmValueLow))
		return PRINT_DATA_OVERlIMITED;
	else
		return PRINT_DATA_NORMAL;
}

/*******************************************************************************
 * @brief 打印日期
 */
static void PRINT_Date(char* date)
{
	memcpy(&PRINT_SendBuffer[0], "*************", 13);
	memcpy(&PRINT_SendBuffer[13], date, 6);
	memcpy(&PRINT_SendBuffer[19], "*************", 13);
	PRINT_SendBuffer[33] = '\n';
	PRINT_SendData(33);
}














