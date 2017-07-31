#include "file.h"

char FILE_FileName[10];

/******************************************************************************/
static ErrorStatus FILE_SaveInfo(FILE_InfoTypedef* info);
static ErrorStatus FILE_ReadInfo(FILE_InfoTypedef* info, uint8_t count);
static void FILE_GetFileNameDependOnTime(FILE_RealTime* time);
static void AnalogDataFormatConvert(float value, EE_DataFormatEnum format,
							uint8_t* pBuffer);
static void LocationFormatConvert(double value, uint8_t* pBuffer);


/*******************************************************************************
 *
 */
void FILE_Init(void)
{
	/* 向文件名添加固定的后缀名 */
	memcpy(&FILE_FileName[6], ".txt", 4);
}

/*******************************************************************************
 *
 */
ErrorStatus FILE_SaveReadInfo(FILE_InfoTypedef* saveInfo,
		FILE_InfoTypedef* readInfo, uint8_t readInfoCount)
{
	/* 挂载文件系统 */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	/* 获取总空间和空闲空间 */
	/* todo */

	/* 根据时间获取文件名 */
	FILE_GetFileNameDependOnTime(&saveInfo->realTime);

	if (ERROR == FILE_SaveInfo(saveInfo))
	{
		/* 保存出错处理 */
		/* todo */
		printf("数据储存失败\r\n");
	}

	if (ERROR == FILE_ReadInfo(readInfo, readInfoCount))
	{
		/* 读取操作出错处理 */
		/* todo */
		printf("数据读取失败\r\n");
	}

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 *
 */
void FILE_InfoFormatConvert(FILE_InfoTypedef*    saveInfo,
							RT_TimeTypedef*      realTime,
							GPS_LocateTypedef*   location,
							ANALOG_ValueTypedef* analogValue)
{
	/* 结构体复位，避免数据出错 */
	memset(saveInfo, 0, sizeof(FILE_InfoTypedef));

	/* 获取时钟 */
	saveInfo->realTime.year  = realTime->date.Year;
	saveInfo->realTime.month = realTime->date.Month;
	saveInfo->realTime.day   = realTime->date.Date;
	saveInfo->realTime.hour  = realTime->time.Hours;
	saveInfo->realTime.min   = realTime->time.Minutes;
	/* 为了数据的整齐，将秒置位0 */
	saveInfo->realTime.sec = 0;

	/* 储存电池电量 */
	saveInfo->batteryLevel = analogValue->batVoltage;

	/* 外部电池状态 */
//	saveInfo->externalPowerStatus = INPUT_CheckPwrOnStatus();

	LocationFormatConvert(location->latitude,  (uint8_t*)&saveInfo->latitude);
	LocationFormatConvert(location->longitude, (uint8_t*)&saveInfo->longitude);

	/* 模拟数据格式转换 */
	AnalogDataFormatConvert(analogValue->temp1, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp1);
	AnalogDataFormatConvert(analogValue->humi1, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi1);
	AnalogDataFormatConvert(analogValue->temp2, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp2);
	AnalogDataFormatConvert(analogValue->humi2, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi2);
	AnalogDataFormatConvert(analogValue->temp3, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp3);
	AnalogDataFormatConvert(analogValue->humi3, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi3);
	AnalogDataFormatConvert(analogValue->temp4, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp4);
	AnalogDataFormatConvert(analogValue->humi4, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi4);
}

/*******************************************************************************
 *
 */
static ErrorStatus FILE_SaveInfo(FILE_InfoTypedef* info)
{
	/* 获取文件，文件名存在则打开写入，不存在则创建写入 */
	if (ERROR == FATFS_FileOpen(FILE_FileName, FATFS_MODE_OPEN_ALWAYS_WRITE))
	{
		printf("文件打开失败\r\n");
		return ERROR;
	}

	/* 将写指针指向文件的末尾 */
	if (ERROR == FATFS_FileSeekEnd())
	{
		printf("指向文件末尾失败\r\n");
		return ERROR;
	}

	/* 把结构体写入文件 */
	if (ERROR == FATFS_FileWrite((BYTE*)info, sizeof(FILE_InfoTypedef)))
	{
		printf("结构体写入失败\r\n");
		return ERROR;
	}

	if (ERROR == FATFS_FileClose())
		return ERROR;

	return SUCCESS;
}

/*******************************************************************************
 *
 */
static ErrorStatus FILE_ReadInfo(FILE_InfoTypedef* info, uint8_t count)
{
	WORD byteToRead;

	/* 获取文件，文件名存在则打开写入，不存在则创建写入 */
	if (FATFS_FileOpen(FILE_FileName, FATFS_MODE_OPEN_EXISTING_READ) == ERROR)
		return ERROR;

	byteToRead = count * sizeof(FILE_InfoTypedef);

	/* 将写指针指向文件的末尾 */
	if (FATFS_FileSeekBackward(byteToRead) == ERROR)
		return ERROR;

	/* 把结构体写入文件 */
	if (FATFS_FileRead((BYTE*)info, byteToRead) == ERROR)
		return ERROR;

	if (FATFS_FileClose() == ERROR)
		return ERROR;

	return SUCCESS;
}

/*******************************************************************************
 * function:根据结构体的时间，转换成文件名
 */
static void FILE_GetFileNameDependOnTime(FILE_RealTime* time)
{
	/* 将时间转换成ASCII码 */
	BCD2ASCII(FILE_FileName, (uint8_t*)time, 3);
}

/*******************************************************************************
 * function:模拟量数据格式转换
 */
static void AnalogDataFormatConvert(float value, EE_DataFormatEnum format,
							uint8_t* pBuffer)
{
	BOOL negative = FALSE;
	uint16_t temp = 0;

	/* 判断是否为负数 */
	if (value < 0)
		negative = TRUE;

	switch (format)
	{
	case FORMAT_INT:
		temp = (uint16_t)abs((int)(value));
		break;

	case FORMAT_ONE_DECIMAL:
		temp = (uint16_t)abs((int)(value * 10));
		break;

	case FORMAT_TWO_DECIMAL:
		temp = (uint16_t)abs((int)(value * 100));
		break;
	default:
		break;
	}

	*pBuffer = HalfWord_GetHighByte(temp);
	*(pBuffer + 1) = HalfWord_GetLowByte(temp);

	/* 负数则最高位置一 */
	if (negative)
		*pBuffer |= 0x8000;
}

/*******************************************************************************
 *
 */
static void LocationFormatConvert(double value, uint8_t* pBuffer)
{
	BOOL negative = FALSE;
	uint32_t temp;

	if (value < 0)
		negative = TRUE;

	/* 获取整数部分 */
	*pBuffer = abs((int)value);

	temp = (uint32_t)((value - (*pBuffer)) * 1000000);

	if (negative)
		temp |= 0x800000;

	*(pBuffer + 1) = (uint8_t)((temp & 0x00FF0000) >> 16);
	*(pBuffer + 2) = (uint8_t)((temp & 0x0000FF00) >> 8);
	*(pBuffer + 3) = (uint8_t)(temp & 0x000000FF);
}





































