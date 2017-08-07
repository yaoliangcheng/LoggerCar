#include "file.h"
#include "print.h"
#include "fatfs.h"

char FILE_FileName[11];
FILE_PatchPackTypedef FILE_PatchPack;

/******************************************************************************/
static ErrorStatus FILE_SaveInfo(FILE_InfoTypedef* info);
static ErrorStatus FILE_ReadInfo(FILE_InfoTypedef* info, uint8_t count);
static void FILE_GetFileNameDependOnTime(FILE_RealTime* time);
static void AnalogDataFormatConvert(float value, EE_DataFormatEnum format,
							uint8_t* pBuffer);
static void LocationFormatConvert(double value, uint8_t* pBuffer);
static uint16_t SearchTimeInFile(FILE_RealTime* pTime);
static void selectDataPrint(uint16_t startPoint, uint16_t endPoint, PRINT_ChannelSelectTypedef* select);


/*******************************************************************************
 *
 */
void FILE_Init(void)
{
	/* 向文件名添加固定的后缀名 */
	memcpy(&FILE_FileName[6], ".txt\0", 5);

	/* 创建补传文件 */
	FATFS_MakeFile(PATCH_PACK_FILE_NAME);
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
	if (ERROR == FATFS_GetSpaceInfo())
	{
		printf("数据空间获取失败\r\n");
	}

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
 * function：读补传文件patch.txt
 * pBuffer：接收指针
 */
ErrorStatus FILE_ReadPatchPackFile(FILE_PatchPackTypedef* pBuffer)
{
	/* 挂载文件系统 */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	if (FATFS_FileOpen(PATCH_PACK_FILE_NAME, FATFS_MODE_OPEN_EXISTING_READ) == ERROR)
		return ERROR;

	/* 把结构体写入文件 */
	if (FATFS_FileRead((BYTE*)pBuffer, sizeof(FILE_PatchPackTypedef)) == ERROR)
		return ERROR;

	if (FATFS_FileClose() == ERROR)
		return ERROR;

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 * function：写补传文件patch.txt
 * pBuffer：写入指针
 */
ErrorStatus FILE_WritePatchPackFile(FILE_PatchPackTypedef* pBuffer)
{
	/* 挂载文件系统 */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	if (FATFS_FileOpen(PATCH_PACK_FILE_NAME, FATFS_MODE_OPEN_ALWAYS_WRITE) == ERROR)
		return ERROR;

	/* 把结构体写入文件 */
	if (FATFS_FileWrite((BYTE*)pBuffer, sizeof(FILE_PatchPackTypedef)) == ERROR)
		return ERROR;

	if (FATFS_FileClose() == ERROR)
		return ERROR;

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 * function:寻找文件中该时间点的数据，返回该数据所在结构体地址
 * @time：时间指针，注意：该时间是十进制format
 *
 */
ErrorStatus FILE_PrintDependOnTime(FILE_RealTime* startTime, FILE_RealTime* stopTime,
		PRINT_ChannelSelectTypedef* select)
{
	uint16_t startTimePoint, endTimePoint;

	/* 获取开始打印时间文件名 */
	/* 注意：这时候时间是十进制的 */
	HEX2ASCII(&startTime->year, (uint8_t*)FILE_FileName, 3);

	/* 挂载文件系统 */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	if (FATFS_FileOpen(FILE_FileName, FATFS_MODE_OPEN_EXISTING_READ) == SUCCESS)
	{
		/* 寻找开始时间的结构体偏移量 */
		startTimePoint = SearchTimeInFile(startTime);
	}
	else
		printf("未找到有效的开始打印时间文件\r\n");

	/* 开始打印时间和结束打印时间是同一天 */
	if ((startTime->year == stopTime->year) && (startTime->month == stopTime->month)
			&& (startTime->day == stopTime->day))
	{
		/* 生成结束打印时间 */
		HEX2BCD(&stopTime->hour, (uint8_t*)(&endTimePoint) + 1, 1);
		HEX2BCD(&stopTime->min,  (uint8_t*)(&endTimePoint),     1);

		/* 寻找结束时间的结构体偏移量 */
//		endTimePoint   = SearchTimeInFile(stopTime);

		/* 开始打印 */
		selectDataPrint(startTimePoint, endTimePoint, select);
	}
	/* 开始打印和结束打印时间是跨天的 */
	else
	{
		/* todo */
//		/* 先关闭开始打印时间所在的文件 */
//		FATFS_FileClose();
//
//		/* 获取结束打印时间文件名 */
//		sprintf(FILE_FileName[0], "%2d", stopTime->year);
//		sprintf(FILE_FileName[2], "%2d", stopTime->month);
//		sprintf(FILE_FileName[4], "%2d", stopTime->day);
//
//		if (FATFS_FileOpen(FILE_FileName, FATFS_MODE_OPEN_EXISTING_READ) == SUCCESS)
//		{
//			/* 寻找结束时间的结构体偏移量 */
//			endTimePoint   = SearchTimeInFile(stopTime);
//		}

	}

	if (FATFS_FileClose() == ERROR)
		return ERROR;

	FATFS_FileUnlink();
	
	return SUCCESS;
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
		*pBuffer |= 0x80;
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

/*******************************************************************************
 * function:在文件内寻找时间点
 * @pTime:要寻找的时间点
 */
static uint16_t SearchTimeInFile(FILE_RealTime* pTime)
{
	uint16_t sourceTime, destTime;		/* 源时间点，和目标时间点 */
	uint16_t fileStructNumbStart, fileStructNumbEnd;	/* 文件中结构体个数 */
	uint16_t searchPoint;
	FILE_InfoTypedef info;

	/* 找到文件夹，则将其转换为BCD */
	/* 寻找时间点，只需根据时分两个参数去查找 */
	/* 注意：stm32内部采用小端模式 */
	HEX2BCD(&pTime->hour, (uint8_t*)(&destTime) + 1, 1);
	HEX2BCD(&pTime->min,  (uint8_t*)(&destTime),     1);

	fileStructNumbStart = 0;
	/* 读取数据，则从最后一个结构体的起始地址开始读 */
	fileStructNumbEnd = FATFS_GetFileStructCount();

	while (1)
	{
		searchPoint = (fileStructNumbEnd + fileStructNumbStart) / 2;
		
		/* 如果介于两个结构体之间的，以本次读到的数据为准 */
		if (searchPoint == fileStructNumbStart)
			break;

		FATFS_FileSeek(searchPoint * sizeof(FILE_InfoTypedef));
		if (FATFS_FileRead((BYTE*)&info, sizeof(FILE_InfoTypedef)) == SUCCESS)
		{
			/* 结构体中获取时间变量 */
			sourceTime = (info.realTime.hour << 8) | (info.realTime.min);

			if (sourceTime == destTime)
				break;
			else if (sourceTime > destTime)
				fileStructNumbEnd = searchPoint;
			else
				fileStructNumbStart = searchPoint;
		}
	}

	return searchPoint;
}

/*******************************************************************************
 * function:根据时间点打印同一个文件内的数据
 */
static void selectDataPrint(uint16_t startPoint, uint16_t endPoint, PRINT_ChannelSelectTypedef* select)
{
	FILE_InfoTypedef info;

	PRINT_PWR_ENABLE();
	
	PRINT_TitleOut();

	/* 打印开始和结束时间点间的数据 */
	while(startPoint <= (FATFS_GetFileStructCount() - 1))
	{
		FATFS_FileSeek(startPoint * sizeof(FILE_InfoTypedef));
		if (FATFS_FileRead((BYTE*)&info, sizeof(FILE_InfoTypedef)) == SUCCESS)
		{
			if (endPoint < ((info.realTime.hour << 8) | (info.realTime.min)))
				break;
			else
			{
				PRINT_DataOut(&info, select);
				startPoint++;
			}
		}
	}
	

	PRINT_TailOut();
//	PRINT_PWR_DISABLE();
}

































