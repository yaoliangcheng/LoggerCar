#include "print.h"



/******************************************************************************/
uint8_t PRINT_SendBuffer[PRINT_SEND_BYTES_MAX];

/******************************************************************************/
static void PRINT_SendData(uint16_t size);
static BOOL PRINT_GetAnalogAndAdjust(uint16_t analog, uint8_t* buf, float alarmUp, float alarmLow);
static uint32_t PRINT_SearchStartTime(PRINT_TimeCompareTypedef destTime);
static PRINT_DataStatusEnum PRINT_DataPrint(uint64_t offset, PRINT_TimeCompareTypedef endTimePoint,
		ChannelSelectTypedef* select);

/*******************************************************************************
 * function:打印日期
 */
void PRINT_Date(char* fileName)
{
	memcpy(&PRINT_SendBuffer[0], "*************", 13);
	memcpy(&PRINT_SendBuffer[13], fileName, 6);
	memcpy(&PRINT_SendBuffer[19], "*************", 13);
	PRINT_SendBuffer[33] = '\n';
	PRINT_SendData(33);
}

/*******************************************************************************
 *
 */
void PRINT_SetMode(void)
{
	PRINT_SendBuffer[0] = 0x1B;
	PRINT_SendBuffer[1] = 0x7B;
	PRINT_SendBuffer[2] = 0;
	PRINT_SendData(3);
}

/*******************************************************************************
 * function:打印输出
 */
void PRINT_DataOut(FILE_RealTimeTypedef* startTime,
		FILE_RealTimeTypedef* endTime, ChannelSelectTypedef* select)
{
	PRINT_TimeCompareTypedef startTimePoint, endTimePoint;
	uint32_t offsetStruct;
	PRINT_DataStatusEnum status;

	startTimePoint.year = startTime->year;
	startTimePoint.date = (startTime->month << 24) | (startTime->day << 16) |
			(startTime->hour << 8) | (startTime->min);

	endTimePoint.year = endTime->year;
	endTimePoint.date = (endTime->month << 24) | (endTime->day << 16) |
			(endTime->hour << 8) | (endTime->min);

	offsetStruct = PRINT_SearchStartTime(startTimePoint);

	while (1)
	{
		status = PRINT_DataPrint(offsetStruct, endTimePoint, select);

		if (PRINT_DATA_END == status)
			break;
		else if (PRINT_DATA_NORMAL == status)
			offsetStruct += 5;
		else if (PRINT_DATA_OVERlIMITED == status)
			offsetStruct += 2;
	}
}

/*******************************************************************************
 * function：打印标题数据
 */
void PRINT_TitleOut(void)
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
void PRINT_TailOut(void)
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

	memcpy(&PRINT_SendBuffer[index], "\n\n\n", 3);
	index += 3;

	PRINT_SendData(index);
}

/*******************************************************************************
 *
 */
static void PRINT_SendData(uint16_t size)
{
//	HAL_UART_Transmit_DMA(&PRINT_UART, PRINT_SendBuffer, size);
	HAL_UART_Transmit(&PRINT_UART, PRINT_SendBuffer, size, 1000);
}

/*******************************************************************************
 * function:根据开始时间查找结构体偏移，返回结构体偏移
 */
static uint32_t PRINT_SearchStartTime(PRINT_TimeCompareTypedef destTime)
{
	PRINT_TimeCompareTypedef sourceTime;
	uint8_t month, day, hour, min;
	uint32_t fileStructStart, fileStructEnd, searchPoint;
	FILE_SaveInfoTypedef info;

	FATFS_FileLink();

	FATFS_FileOpen(FILE_NAME_SAVE_DATA, FATFS_MODE_OPEN_EXISTING_READ);

	fileStructStart = 0;
	fileStructEnd   = FATFS_GetFileStructCount();

	while(1)
	{
		searchPoint = (fileStructStart + fileStructEnd) / 2;

		/* 如果介于两个结构体之间的，以本次读到的数据为准 */
		if (searchPoint == fileStructStart)
			break;

		FATFS_FileSeek(searchPoint * sizeof(FILE_SaveInfoTypedef));
		if (FATFS_FileRead((BYTE*)&info, sizeof(FILE_SaveInfoTypedef)) == SUCCESS)
		{
			ASCII2HEX((uint8_t*)info.year, &sourceTime.year, 2);

			if (sourceTime.year == destTime.year)
			{
				ASCII2HEX((uint8_t*)info.month, &month, 2);
				ASCII2HEX((uint8_t*)info.day,   &day,   2);
				ASCII2HEX((uint8_t*)info.hour,  &hour,  2);
				ASCII2HEX((uint8_t*)info.min,   &min,   2);

				sourceTime.date = CHAR2LONG(month, day, hour, min);

				if (sourceTime.date == destTime.date)
					break;
				else if (sourceTime.date > destTime.date)
				{
					fileStructEnd = searchPoint;
				}
				else
				{
					fileStructStart = searchPoint;
				}
			}
			else if (sourceTime.year > destTime.year)
			{
				fileStructEnd = searchPoint;
			}
			else
			{
				fileStructStart = searchPoint;
			}
		}
	}

	FATFS_FileClose();

	FATFS_FileUnlink();

	return searchPoint;
}

/*******************************************************************************
 * function:获取模拟量的值，将其输出打印，并且判断是否超标
 * analog：模拟量值
 * buf：转换成字符串的指针
 * alarmUp:报警上限
 * alarmLow：报警下限
 * return: true->超标，  false->正常
 */
static BOOL PRINT_GetAnalogAndAdjust(uint16_t analog, uint8_t* buf, float alarmUp, float alarmLow)
{
	float    temp;
	uint16_t data;
	BOOL     status = FALSE;

	data = (HalfWord_GetLowByte(analog) << 8) | HalfWord_GetHighByte(analog);

	/* 判断正负 */
	if ((data & 0x8000) != 0)
		temp = -((float)(data & 0x7FFF) / 10);
	else
		temp = (float)(data & 0x7FFF) / 10;

	/* 调用这个函数导致设备死机，还未找到问题，后续继续研究 */
//	sprintf((char*)buf, "%6.1f", temp);

	*buf = ' ';

	/* 如果为负数 */
	if ((data & 0x8000) != 0)
	{
		*(buf + 1) = '-';
		/* 转换为正数 */
		data &= 0x7FFF;
	}
	else
	{
		/* 正数的话才有可能是三位数 */
		*(buf + 1) = ((data % 10000) / 1000) + '0';
		if (*(buf + 1) == '0')
			*(buf + 1) = ' ';
	}

	*(buf + 2) = ((data % 1000) / 100)   + '0';
	*(buf + 3) = ((data % 100) / 10)     + '0';
	*(buf + 4) = '.';
	*(buf + 5) = (data % 10)             + '0';

	/* 报警上限值有效，并且超标 */
	if ((alarmUp != PRINT_ALARM_INVALID) && (temp > alarmUp))
		status = TRUE;

	if ((alarmLow != PRINT_ALARM_INVALID) && (temp < alarmLow))
		status = TRUE;

	return status;
}

/*******************************************************************************
 * function:根据结构体偏移，读出数据，并根据打印通道选择，打印数据，并返回该数据是否超标
 */
static PRINT_DataStatusEnum PRINT_DataPrint(uint64_t offset, PRINT_TimeCompareTypedef endTimePoint,
		ChannelSelectTypedef* select)
{
	FILE_SaveInfoTypedef saveInfo;
	uint8_t index = 0;
	PRINT_TimeCompareTypedef time;								/* 根据当前时间生成64位数值，用于比较 */
	PRINT_DataStatusEnum status;
	uint8_t month, day, hour, min;

	/* 读取数据 */
	FILE_ReadFile(FILE_NAME_SAVE_DATA, offset * sizeof(FILE_SaveInfoTypedef),
			(uint8_t*)&saveInfo, sizeof(FILE_SaveInfoTypedef));

	/* 生成时间数值 */
	ASCII2HEX((uint8_t*)saveInfo.year,  &time.year, 2);
	ASCII2HEX((uint8_t*)saveInfo.month, &month, 2);
	ASCII2HEX((uint8_t*)saveInfo.day,   &day,   2);
	ASCII2HEX((uint8_t*)saveInfo.hour,  &hour,  2);
	ASCII2HEX((uint8_t*)saveInfo.min,   &min,   2);

	time.date = CHAR2LONG(month, day, hour, min);

	/* 如果时间超过了结束时间，则返回时间结束 */
	if ((time.year >= endTimePoint.year) && (time.date > endTimePoint.date))
		return PRINT_DATA_END;

	/* 年月日时分秒 */
	memcpy((char*)&PRINT_SendBuffer[0],  &saveInfo.year, 14);
	index += 14;

	/* 根据打印选择，输出数据 */
	if (select->status.bit.ch1)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &saveInfo.analogValue[0], 6);

		/* 判断是否超标 */
		/* todo */

		index += 6;
	}
	if (select->status.bit.ch2)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &saveInfo.analogValue[1], 6);

		/* 判断是否超标 */
		/* todo */

		index += 6;
	}
	if (select->status.bit.ch3)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &saveInfo.analogValue[2], 6);

		/* 判断是否超标 */
		/* todo */

		index += 6;
	}
	if (select->status.bit.ch4)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &saveInfo.analogValue[3], 6);

		/* 判断是否超标 */
		/* todo */

		index += 6;
	}
	if (select->status.bit.ch5)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &saveInfo.analogValue[4], 6);

		/* 判断是否超标 */
		/* todo */

		index += 6;
	}
	if (select->status.bit.ch6)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &saveInfo.analogValue[5], 6);

		/* 判断是否超标 */
		/* todo */

		index += 6;
	}
	if (select->status.bit.ch7)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &saveInfo.analogValue[6], 6);

		/* 判断是否超标 */
		/* todo */

		index += 6;
	}
	if (select->status.bit.ch8)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &saveInfo.analogValue[7], 6);

		/* 判断是否超标 */
		/* todo */

		index += 6;
	}

	/* 打印换行 */
	PRINT_SendBuffer[index] = '\n';
	index++;

	PRINT_SendData(index);

	return status;
}
























