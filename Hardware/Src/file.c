#include "file.h"
#include "print.h"
#include "fatfs.h"
#include "gprs.h"

char FILE_FileName[11];						/* 储存文件名 */
char FILE_PrintFileName[11]; 				/* 打印文件名 */
FILE_PatchPackTypedef FILE_PatchPack;		/* 补传文件信息 */
FILE_DeviceParamTypedef FILE_DeviceParam;	/* 设备参数信息 */

/******************************************************************************/
static void FILE_GetFileNameDependOnTime(FILE_RealTime* time, char* fileName);
static void FILE_GetNextFileName(char* fileName);
static void LocationFormatConvert(double value, uint8_t* pBuffer);
static uint16_t SearchTimeInFile(FILE_RealTime* pTime);
static void selectDataPrint(char* fileName,
							uint16_t startPoint, uint16_t endPoint,
							PRINT_ChannelSelectTypedef* select);
static void AnalogDataFormatConvert(float value, DataFormatEnum format,
							uint8_t* pBuffer);

/*******************************************************************************
 *
 */
void FILE_Init(void)
{
	/* 向文件名添加固定的后缀名 */
	memcpy(&FILE_FileName[6], ".txt\0", 5);
	memcpy(&FILE_PrintFileName[6], ".txt\0", 5);
}

/*******************************************************************************
 *
 */
ErrorStatus FILE_SaveInfo(FILE_InfoTypedef* saveInfo, uint16_t* fileStructCount)
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
	FILE_GetFileNameDependOnTime(&saveInfo->realTime, FILE_FileName);

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
	if (ERROR == FATFS_FileWrite((BYTE*)saveInfo, sizeof(FILE_InfoTypedef)))
	{
		printf("结构体写入失败\r\n");
		return ERROR;
	}

	/* 获取文件大小 */
	*fileStructCount = FATFS_GetFileStructCount();

	if (ERROR == FATFS_FileClose())
		return ERROR;

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 *
 */
ErrorStatus FILE_ReadInfo(FILE_InfoTypedef* readInfo)
{
	/* 挂载文件系统 */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	/* 获取文件，文件名存在则打开写入，不存在则创建写入 */
	if (ERROR == FATFS_FileOpen(FILE_FileName, FATFS_MODE_OPEN_EXISTING_READ))
	{
		printf("文件打开失败\r\n");
		return ERROR;
	}

	/* 将写指针指向文件的末尾 */
	if (ERROR == FATFS_FileSeekBackwardOnePack())
	{
		printf("指向文件末尾失败\r\n");
		return ERROR;
	}

	/* 把结构体写入文件 */
	if (ERROR == FATFS_FileRead((BYTE*)readInfo, sizeof(FILE_InfoTypedef)))
	{
		printf("结构体写入失败\r\n");
		return ERROR;
	}

	if (ERROR == FATFS_FileClose())
		return ERROR;

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 * function:根据补传的信息获取数据
 * @patch：补传信息
 * @readInfo：读出缓存指针
 */
uint16_t FILE_ReadPatchInfo(FILE_PatchPackTypedef* patch, FILE_InfoTypedef* readInfo)
{
	uint16_t readInfoCount;

	/* 挂载文件系统 */
	if (ERROR == FATFS_FileLink())
		return 0;

	/* 获取文件，文件名存在则打开写入，不存在则创建写入 */
	if (ERROR == FATFS_FileOpen(patch->patchFileName, FATFS_MODE_OPEN_EXISTING_READ))
		return 0;

	/* 将读指针指向文件的指定位置 */
	if (ERROR == FATFS_FileSeek(patch->patchStructOffset * sizeof(FILE_InfoTypedef)))
		return 0;

	/* 当前文件还有多少个结构体可以读 */
	readInfoCount = FATFS_GetFileStructCount() - patch->patchStructOffset;

	/* 文件中结构体数不能一次读完 */
	if (readInfoCount > GPRS_PATCH_PACK_NUMB_MAX)
	{
		readInfoCount = GPRS_PATCH_PACK_NUMB_MAX;
		patch->patchStructOffset += GPRS_PATCH_PACK_NUMB_MAX;
	}
	/* 当前文件剩余的结构体能够被一次读完 */
	else
	{
		patch->patchStructOffset = 0;

		/* 比较当前补传文件是否是当前文件 */
		if (memcmp(patch->patchFileName, FILE_FileName, 6) == 0)
		{
			/* 是，则证明全部数据补传完毕,补传文件清空 */
			memcpy(patch->patchFileName, "\0\0\0\0\0\0", 6);
		}
		else
		{
			/* 否则传递到下一个文件 */
			FILE_GetNextFileName(patch->patchFileName);
		}
	}

	if (ERROR == FATFS_FileRead((BYTE*)readInfo, (readInfoCount * sizeof(FILE_InfoTypedef))))
		return 0;

	if (ERROR == FATFS_FileClose())
		return 0;

	FATFS_FileUnlink();

	return readInfoCount;
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
 * function:设备参数文件初始化
 */
ErrorStatus FILE_ParamFileInit(void)
{
	/* 挂载文件系统 */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	/* 如果文件不存在 */
	if (FATFS_FileOpen(PARAM_FILE_NAME, FATFS_MODE_OPEN_EXISTING_READ) == ERROR)
	{
		memcpy(FILE_DeviceParam.deviceSN, "1708151515", sizeof(FILE_DeviceParam.deviceSN));
		FILE_DeviceParam.locationType    = LOCATION_GPS;
		FILE_DeviceParam.firmwareVersion = 10;
		FILE_DeviceParam.recordInterval  = 2;
		FILE_DeviceParam.overLimitRecordInterval = 2;
		FILE_DeviceParam.exitAnalogChannelNumb = ANALOG_CHANNEL_NUMB;
		FILE_DeviceParam.param[0].channelType = TYPE_TEMP;
		FILE_DeviceParam.param[0].channelUnit = UNIT_TEMP;
		FILE_DeviceParam.param[0].dataFormat  = FORMAT_ONE_DECIMAL;
		FILE_DeviceParam.param[1].channelType = TYPE_HUMI;
		FILE_DeviceParam.param[1].channelUnit = UNIT_HUMI;
		FILE_DeviceParam.param[1].dataFormat  = FORMAT_ONE_DECIMAL;
		FILE_DeviceParam.param[2].channelType = TYPE_TEMP;
		FILE_DeviceParam.param[2].channelUnit = UNIT_TEMP;
		FILE_DeviceParam.param[2].dataFormat  = FORMAT_ONE_DECIMAL;
		FILE_DeviceParam.param[3].channelType = TYPE_HUMI;
		FILE_DeviceParam.param[3].channelUnit = UNIT_HUMI;
		FILE_DeviceParam.param[3].dataFormat  = FORMAT_ONE_DECIMAL;
		FILE_DeviceParam.param[4].channelType = TYPE_TEMP;
		FILE_DeviceParam.param[4].channelUnit = UNIT_TEMP;
		FILE_DeviceParam.param[4].dataFormat  = FORMAT_ONE_DECIMAL;
		FILE_DeviceParam.param[5].channelType = TYPE_HUMI;
		FILE_DeviceParam.param[5].channelUnit = UNIT_HUMI;
		FILE_DeviceParam.param[5].dataFormat  = FORMAT_ONE_DECIMAL;
		FILE_DeviceParam.param[6].channelType = TYPE_TEMP;
		FILE_DeviceParam.param[6].channelUnit = UNIT_TEMP;
		FILE_DeviceParam.param[6].dataFormat  = FORMAT_ONE_DECIMAL;
		FILE_DeviceParam.param[7].channelType = TYPE_HUMI;
		FILE_DeviceParam.param[7].channelUnit = UNIT_HUMI;
		FILE_DeviceParam.param[7].dataFormat  = FORMAT_ONE_DECIMAL;

		FATFS_FileOpen(PARAM_FILE_NAME, FATFS_MODE_OPEN_ALWAYS_WRITE);
		FATFS_FileWrite((uint8_t*)&FILE_DeviceParam, sizeof(FILE_DeviceParam));
	}
	/* 打开成功则说明文件存在 */
	else
	{
		FATFS_FileRead((uint8_t*)&FILE_DeviceParam, sizeof(FILE_DeviceParam));
	}

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
	uint16_t startTimeStructOffset;			/* 开始打印时间，结构体偏移 */
	uint16_t endTimePoint;					/* 结束时间点 */
	char stopPrintFileName[6];					/* 停止打印文件名 */

	/* 获取开始打印时间文件名 */
	/* 注意：这时候时间是十进制的 */
	HEX2ASCII(&startTime->year, (uint8_t*)FILE_PrintFileName, 3);
	HEX2ASCII(&stopTime->year,  (uint8_t*)stopPrintFileName,  3);

	/* 挂载文件系统 */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	if (FATFS_FileOpen(FILE_PrintFileName, FATFS_MODE_OPEN_EXISTING_READ) == SUCCESS)
	{
		/* 寻找开始时间的结构体偏移量 */
		startTimeStructOffset = SearchTimeInFile(startTime);
	}
	else
		printf("未找到有效的开始打印时间文件\r\n");

	if (FATFS_FileClose() == ERROR)
		return ERROR;

	/* 先打印标题 */
	PRINT_TitleOut();

	/* 还没有打印到停止时间文件 */
	while (memcmp(FILE_PrintFileName, stopPrintFileName, 6) != 0)
	{
		/* 打开文件并打印到文件结束 */
		selectDataPrint(FILE_PrintFileName, startTimeStructOffset, FILE_PRINT_TO_END, select);
		/* 转换到下一个文件 */
		FILE_GetNextFileName(FILE_PrintFileName);
		/* 下一个文件结构体偏移为0 */
		startTimeStructOffset = 0;
	}

	/* 生成结束打印时间 */
	HEX2BCD(&stopTime->hour, (uint8_t*)(&endTimePoint) + 1, 1);
	HEX2BCD(&stopTime->min,  (uint8_t*)(&endTimePoint),     1);
	/* 开始打印 */
	selectDataPrint(FILE_PrintFileName, startTimeStructOffset, endTimePoint, select);

	/* 打印结束 */
	PRINT_TailOut();

	FATFS_FileUnlink();
	
	return SUCCESS;
}

/*******************************************************************************
 * function:根据结构体的时间，转换成文件名
 */
static void FILE_GetFileNameDependOnTime(FILE_RealTime* time, char* fileName)
{
	/* 将时间转换成ASCII码 */
	BCD2ASCII(fileName, (uint8_t*)time, 3);
}

/*******************************************************************************
 * function：根据FILE_FileName得到下一个文件名
 */
static void FILE_GetNextFileName(char* fileName)
{
	uint8_t temp1, temp2;

	uint8_t year, month, day;

	str2numb((uint8_t*)&fileName[0], &temp1, 1);
	str2numb((uint8_t*)&fileName[1], &temp2, 1);
	year = temp1 * 10 + temp2;

	str2numb((uint8_t*)&fileName[2], &temp1, 1);
	str2numb((uint8_t*)&fileName[3], &temp2, 1);
	month = temp1 * 10 + temp2;

	str2numb((uint8_t*)&fileName[4], &temp1, 1);
	str2numb((uint8_t*)&fileName[5], &temp2, 1);
	day = temp1 * 10 + temp2;

	/* 时间计算 */
	if((month == 1U) || (month == 3U) || (month == 5U) || (month == 7U) || \
	   (month == 8U) || (month == 10U) || (month == 12U))
	{
		if(day < 31U)
		{
			day++;
		}
		/* Date structure member: day = 31 */
		else
		{
			if(month != 12U)
			{
				month++;
				day = 1U;
			}
			/* Date structure member: day = 31 & month =12 */
			else
			{
				month = 1U;
				day = 1U;
				year++;
			}
		}
	}
	else if((month == 4U) || (month == 6U) || (month == 9U) || (month == 11U))
	{
		if(day < 30U)
		{
			day++;
		}
		/* Date structure member: day = 30 */
		else
		{
			month++;
			day = 1U;
		}
	}
	else if(month == 2U)
	{
		if(day < 28U)
		{
			day++;
		}
		else if(day == 28U)
		{
			/* Leap year */
			if(year % 4 == 0)
			{
				day++;
			}
			else
			{
				month++;
				day = 1U;
			}
		}
		else if(day == 29U)
		{
			month++;
			day = 1U;
		}
	}

	/* 日期转回ASCII */
	HEX2ASCII(&year,  (uint8_t*)&fileName[0], 1);
	HEX2ASCII(&month, (uint8_t*)&fileName[2], 1);
	HEX2ASCII(&day,   (uint8_t*)&fileName[4], 1);
}

/*******************************************************************************
 * function:模拟量数据格式转换
 */
static void AnalogDataFormatConvert(float value, DataFormatEnum format,
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
 * function:根据时间点打印同一个文件内的数据,endPoint为0xffff，则打印到文件结尾
 */
static void selectDataPrint(char* fileName,
							uint16_t startPoint, uint16_t endPoint,
							PRINT_ChannelSelectTypedef* select)
{
	FILE_InfoTypedef info;

	/* 打开文件 */
	FATFS_FileOpen(fileName, FATFS_MODE_OPEN_EXISTING_READ);

	/* 先打印日期 */
	PRINT_Date(fileName);

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
	
	FATFS_FileClose();
}






























