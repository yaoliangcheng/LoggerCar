#include "file.h"
#include "print.h"
#include "fatfs.h"

FILE_PatchPackTypedef FILE_PatchPack;		/* 补传文件信息 */
uint64_t FILE_DataSaveStructCnt;			/* 数据储存文件结构体总数 */

/******************************************************************************/
static void SaveInfoFormatConvert(FILE_SaveInfoTypedef* info,
							GPRS_SendInfoTypedef* sendInfo);

/*******************************************************************************
 * function：文件系统初始化，获取数据储存文件的结构体总数
 */
void FILE_Init(void)
{
	/* 挂载文件系统 */
	FATFS_FileLink();

	/* 打开文件，如果不存在则先创建文件，确保设备存在必要的文件 */
	FATFS_FileOpen(FILE_NAME_SAVE_DATA, FATFS_MODE_OPNE_ALWAYS);
	FATFS_FileClose();
	FATFS_FileOpen(FILE_NAME_PATCH_PACK, FATFS_MODE_OPNE_ALWAYS);
	FATFS_FileClose();
	FATFS_FileOpen(FILE_NAME_PARAM, FATFS_MODE_OPNE_ALWAYS);
	FATFS_FileClose();

	/* 读数据文件的结构体大小 */
	if (SUCCESS == FATFS_FileOpen(FILE_NAME_SAVE_DATA, FATFS_MODE_OPEN_EXISTING_READ))
	{
		FILE_DataSaveStructCnt = FATFS_GetFileStructCount();
		PARAM_DeviceParam.deviceCapacity = FATFS_GetSpaceInfo();
	}

	FATFS_FileClose();

	FATFS_FileUnlink();
}

/*******************************************************************************
 * function：数据储存结构体中添加符号，使其能用Excel软件打开
 */
void FILE_SaveInfoSymbolInit(FILE_SaveInfoTypedef* info)
{
	info->str1   			  = ' ';
	info->str2   			  = ':';
	info->str3  			  = ':';
	info->str4   			  = ',';
	info->str5   			  = ',';
	info->str6   			  = ',';
	info->str7   			  = ',';
	info->str8   			  = ',';
	info->analogValue[0].str  = ',';
	info->analogValue[1].str  = ',';
	info->analogValue[2].str  = ',';
	info->analogValue[3].str  = ',';
	info->analogValue[4].str  = ',';
	info->analogValue[5].str  = ',';
	info->analogValue[6].str  = ',';
	info->analogValue[7].str  = ',';
	info->end[0] 			  = 0x0D;
	info->end[1] 			  = 0x0A;
}

/*******************************************************************************
 * function:储存结构体到Flash
 * saveInfo：储存结构体指针
 * fileStructCount：当前文件结构体数
 */
ErrorStatus FILE_SaveInfo(FILE_SaveInfoTypedef* saveInfo)
{
	/* 挂载文件系统 */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	/* 获取总空间和空闲空间 */
	FATFS_GetSpaceInfo();

	/* 获取文件，文件名存在则打开写入，不存在则创建写入 */
	if (SUCCESS == FATFS_FileOpen(FILE_NAME_SAVE_DATA, FATFS_MODE_OPEN_ALWAYS_WRITE))
	{
		/* 先判断写入地址是否对齐，对齐才写入，不对齐则跳过当前结构体，写入到下一结构体 */
		FATFS_FileSeekSaveInfoStructAlign();

		/* 把结构体写入文件 */
		FATFS_FileWrite((BYTE*)saveInfo, sizeof(FILE_SaveInfoTypedef));

		/* 获取文件大小 */
		FILE_DataSaveStructCnt = FATFS_GetFileStructCount();
	}

	FATFS_FileClose();

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 * function：从Flash中读出最近一个结构体
 * readInfo：储存读出结构体指针
 * @patch：补传信息
 */
uint16_t FILE_ReadInfo(FILE_SaveInfoTypedef* readInfo, FILE_PatchPackTypedef* patch)
{
	uint16_t readInfoCount;

	/* 挂载文件系统 */
	FATFS_FileLink();

	/* 打开数据储存文件 */
	if (SUCCESS == FATFS_FileOpen(FILE_NAME_SAVE_DATA, FATFS_MODE_OPEN_EXISTING_READ))
	{
		/* 没有补传数据 */
		if (patch->patchStructOffset == 0)
		{
			readInfoCount = 1;

			/* 指针指向最后一个结构体 */
			FATFS_FileSeekBackwardOnePack();

			FATFS_FileRead((BYTE*)readInfo, sizeof(FILE_SaveInfoTypedef));
		}
		else
		{
			/* 有补传数据 */
			FATFS_FileSeek(patch->patchStructOffset * sizeof(FILE_SaveInfoTypedef));

			/* 当前文件还有多少个结构体可以读 */
			readInfoCount = FATFS_GetFileStructCount() - patch->patchStructOffset;

			/* 文件中结构体数不能一次读完 */
			if (readInfoCount > GPRS_PATCH_PACK_NUMB_MAX)
			{
				/* 最大上传的数据组数 */
				readInfoCount = GPRS_PATCH_PACK_NUMB_MAX;
				patch->patchStructOffset += GPRS_PATCH_PACK_NUMB_MAX;
			}
			/* 当前文件剩余的结构体能够被一次读完 */
			else
			{
				/* 补传数据清空 */
				patch->patchStructOffset = 0;
			}

			FATFS_FileRead((BYTE*)readInfo, (readInfoCount * sizeof(FILE_SaveInfoTypedef)));
		}

	}

	FATFS_FileClose();

	FATFS_FileUnlink();

	return readInfoCount;
}

/*******************************************************************************
 * function：发送结构体格式转换
 */
void FILE_SendInfoFormatConvert(uint8_t* saveInfo, uint8_t* sendInfo, uint8_t  sendPackNumb)
{
	uint8_t i;

	for (i = 0; i < sendPackNumb; i++)
	{
		SaveInfoFormatConvert((FILE_SaveInfoTypedef*)saveInfo, (GPRS_SendInfoTypedef*)sendInfo);
		saveInfo += sizeof(FILE_SaveInfoTypedef);
		sendInfo += sizeof(GPRS_SendInfoTypedef);
	}
}

/*******************************************************************************
 * function：读文件
 * @offset:读指针偏移量
 * @fileName:文件名
 * @pBuffer：接收指针
 * @size:读取长度
 */
ErrorStatus FILE_ReadFile(char* fileName, uint64_t offset, BYTE* pBuffer, uint32_t size)
{
	/* 挂载文件系统 */
	if (FATFS_FileLink() == ERROR)
		return ERROR;

	if (FATFS_FileOpen(fileName, FATFS_MODE_OPEN_EXISTING_READ) == SUCCESS)
	{
		if (offset % sizeof(FILE_SaveInfoTypedef) == 0)
		{
			FATFS_FileSeek(offset);
			FATFS_FileRead(pBuffer, size);
		}
	}

	FATFS_FileClose();

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 * function：写文件
 * @offset:写指针偏移量
 * @fileName:文件名
 * @pBuffer：写入指针
 * @size:写入长度
 */
ErrorStatus FILE_WriteFile(char* fileName, uint64_t offset, BYTE* pBuffer, uint32_t size)
{
	/* 挂载文件系统 */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	if (FATFS_FileOpen(fileName, FATFS_MODE_OPEN_ALWAYS_WRITE) == SUCCESS)
	{
		FATFS_FileSeek(offset);

		FATFS_FileWrite(pBuffer, size);
	}

	FATFS_FileClose();

	FATFS_FileUnlink();

	return SUCCESS;
}

#if 0
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
	{
		printf("未找到有效的开始打印时间文件\r\n");
		FATFS_FileUnlink();
		return ERROR;
	}

	if (FATFS_FileClose() == ERROR)
		return ERROR;

	PRINT_SetMode();
	osDelay(1000);
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
#endif

/*******************************************************************************
 * function:模拟量数据格式转换
 */
static void AnalogDataFormatConvert(char* analog, DataFormatEnum format, uint8_t* pBuffer)
{
	char str[6];
	float value;
	BOOL negative = FALSE;
	uint16_t temp = 0;

	/* 将字符串转为float */
	memcpy(str, analog, 5);
	str[5] = '\0';
	value = (float)atof(str);

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
 * function：将字符串型的定位值转换成协议格式
 */
static void LocationFormatConvert(char* lacation, uint8_t* pBuffer)
{
	char str[11];
	double value;
	BOOL negative = FALSE;
	uint32_t temp;

	/* 将字符串转为double */
	memcpy(str, lacation, 10);
	str[10] = '\0';
	value = atof(str);

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
 * funtion:将储存的格式（ASCII码）转换成协议规定的格式
 */
static void SaveInfoFormatConvert(FILE_SaveInfoTypedef* info,
							GPRS_SendInfoTypedef* sendInfo)
{
	char str[4];

	/* 结构体复位，避免数据出错 */
	memset(sendInfo, 0, sizeof(GPRS_SendInfoTypedef));

	/* 时间字符串转换成BCD */
	ASCII2BCD(info->year,  &sendInfo->year,  2);
	ASCII2BCD(info->month, &sendInfo->month, 2);
	ASCII2BCD(info->day,   &sendInfo->day,   2);
	ASCII2BCD(info->hour,  &sendInfo->hour,  2);
	ASCII2BCD(info->min,   &sendInfo->min,   2);
	ASCII2BCD(info->sec,   &sendInfo->sec,   2);

	/* 转换电池电量 */
	memcpy(str, info->batQuality, 3);
	str[3] = '\0';
	sendInfo->batteryLevel = atoi(str);

	/* 转换外部电源状态 */
	str2numb((uint8_t*)&info->exPwrStatus, &sendInfo->externalPowerStatus, 1);

	/* 转换经度 */
	LocationFormatConvert(info->longitude, (uint8_t*)&sendInfo->longitude);
	LocationFormatConvert(info->latitude,  (uint8_t*)&sendInfo->latitude);

	AnalogDataFormatConvert(info->analogValue[0].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[0]);
	AnalogDataFormatConvert(info->analogValue[1].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[1]);
	AnalogDataFormatConvert(info->analogValue[2].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[2]);
	AnalogDataFormatConvert(info->analogValue[3].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[3]);
	AnalogDataFormatConvert(info->analogValue[4].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[4]);
	AnalogDataFormatConvert(info->analogValue[5].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[5]);
	AnalogDataFormatConvert(info->analogValue[6].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[6]);
	AnalogDataFormatConvert(info->analogValue[7].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[7]);
}

#if 0
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
 * >>>根据GSP规定，普通数据5分钟打印一条，正常数据2分钟打印一条。
 */
static void selectDataPrint(char* fileName,
							uint16_t startPoint, uint16_t endPoint,
							PRINT_ChannelSelectTypedef* select)
{
	FILE_InfoTypedef info;
	BOOL status = FALSE;

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
			/* 判断结束时间，如果超过，则停止打印 */
			if (endPoint < ((info.realTime.hour << 8) | (info.realTime.min)))
				break;
			else
			{
				status = PRINT_DataOut(&info, select);
				if (status)
					startPoint += 2;
				else
					startPoint += 5;
			}
		}
	}

	FATFS_FileClose();
}

#endif


/*******************************************************************************
 * function:将储存的ASCII码转换成float
 * @value:模拟量ASCII码
 */
float FILE_Analog2Float(SaveInfoAnalogTypedef* value)
{
	char str[6];

	memcpy(str, value->value, 5);
	str[5] = '\0';
	return (float)atof(str);
}


























