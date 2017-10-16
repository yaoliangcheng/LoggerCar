#include "file.h"
#include "fatfs.h"

/******************************************************************************/
FILE_PatchPackTypedef FILE_PatchPack;		/* 补传文件信息 */
uint64_t FILE_DataSaveStructCnt;			/* 数据储存文件结构体总数 */


FILE_SaveStructTypedef FILE_SaveStruct;			/* 储存信息写入结构体 */
FILE_SaveStructTypedef FILE_ReadStruct[GPRS_PATCH_PACK_NUMB_MAX];
												/* 储存信息读取结构体 */

/******************************************************************************/
extern FATFS objFileSystem;
extern FIL   objFile;

/******************************************************************************/
static void SaveStructSymbolInit(void);
static void FormatConvert(GPRS_SendInfoTypedef*   sendInfo,
						  FILE_SaveStructTypedef* readInfo);

/*******************************************************************************
 * function：文件系统初始化，获取数据储存文件的结构体总数
 */
void FILE_Init(void)
{
	/* 挂载文件系统 */
	if (ERROR == FATFS_FileLink())
		return;

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

	/* 储存数据结构初始化 */
	SaveStructSymbolInit();
}

/*******************************************************************************
 * @brief 读文件
 * @param offset:读指针偏移量
 * @param fileName:文件名
 * @param pBuffer：接收指针
 * @param size:读取长度
 */
void FILE_ReadFile(char* fileName, DWORD offset, BYTE* pBuffer, UINT size)
{
	UINT byteRead = 0;

	if (FR_OK != f_mount(&objFileSystem, USERPath, 1))
		return;
	/* 进入临界区 */
	taskENTER_CRITICAL();

	if (FR_OK == f_open(&objFile, fileName, FA_OPEN_EXISTING | FA_READ))
	{
		f_lseek(&objFile, offset);
		f_read(&objFile, pBuffer, size, &byteRead);
		f_close(&objFile);
	}

	f_mount(NULL, USERPath, 1);
	/* 退出临界区 */
	taskEXIT_CRITICAL();
}

/*******************************************************************************
 * @brief 写文件
 * @param offset:写指针偏移量
 * @param fileName:文件名
 * @param pBuffer：写入指针
 * @param size:写入长度
 */
void FILE_WriteFile(char* fileName, DWORD offset, BYTE* pBuffer, UINT size)
{
	UINT byteWrite = 0;

	if (FR_OK != f_mount(&objFileSystem, USERPath, 1))
		return;
	/* 进入临界区 */
	taskENTER_CRITICAL();

	if (FR_OK == f_open(&objFile, fileName, FA_OPEN_ALWAYS | FA_WRITE))
	{
		f_lseek(&objFile, offset);
		f_read(&objFile, pBuffer, size, &byteWrite);
		f_close(&objFile);
	}

	f_mount(NULL, USERPath, 1);
	/* 退出临界区 */
	taskEXIT_CRITICAL();
}

/*******************************************************************************
 * function:储存结构体到Flash
 * saveInfo：储存结构体指针
 * fileStructCount：当前文件结构体数
 */
void FILE_SaveInfo(void)
{
	uint32_t byteWrite = 0;

	if (FR_OK != f_mount(&objFileSystem, USERPath, 1))
		return;
	/* 进入临界区 */
	taskENTER_CRITICAL();

	if (FR_OK == f_open(&objFile, FILE_NAME_SAVE_DATA, FA_OPEN_ALWAYS | FA_WRITE))
	{
		/* 先判断写入地址是否对齐，对齐才写入，不对齐则覆盖不对齐的数据 */
		if (objFile.fsize % sizeof(FILE_SaveStructTypedef) == 0)
			f_lseek(&objFile, objFile.fsize);
		else
			/* 覆盖当前结构体 */
			f_lseek(&objFile,
					(objFile.fsize / sizeof(FILE_SaveStructTypedef))
					* sizeof(FILE_SaveStructTypedef));
		/* 把结构体写入文件 */
		f_write(&objFile, (BYTE*)&FILE_SaveStruct,
				sizeof(FILE_SaveStructTypedef), &byteWrite);
		/* 获取文件大小 */
		FILE_DataSaveStructCnt = objFile.fsize / sizeof(FILE_SaveStructTypedef);

		f_close(&objFile);
	}

	/* 退出临界区 */
	taskEXIT_CRITICAL();
	f_mount(NULL, USERPath, 1);
}

/*******************************************************************************
 * function：从Flash中读出最近一个结构体
 * readInfo：储存读出结构体指针
 * @patch：补传信息
 */
uint8_t FILE_ReadInfo(FILE_PatchPackTypedef*  patch)
{
	uint8_t readInfoCount;

	/* 没有补传数据 */
	if (patch->patchStructOffset == 0)
	{
		/* 读取数据为1组 */
		readInfoCount = 1;
		/* 读取最后一个结构体 */
		FILE_ReadFile(FILE_NAME_SAVE_DATA,
				(FILE_DataSaveStructCnt - 1) * sizeof(FILE_SaveStructTypedef),
				(uint8_t*)FILE_ReadStruct, sizeof(FILE_SaveStructTypedef));
	}
	else
	{
		/* 当前文件还有多少个结构体可以读 */
		readInfoCount = FILE_DataSaveStructCnt - patch->patchStructOffset;

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
		FILE_ReadFile(FILE_NAME_SAVE_DATA,
				patch->patchStructOffset * sizeof(FILE_SaveStructTypedef),
				(uint8_t*)FILE_ReadStruct,
				readInfoCount * sizeof(FILE_SaveStructTypedef));
	}
	return readInfoCount;
}

/*******************************************************************************
 * @brief 将读出的数据转换成发送的格式
 * @param sendInfo：发送数据结构体指针
 * @param readInfo：读取数据的结构体指针
 * @param sendPackNumb：需要转换的包数
 */
void FILE_SendInfoFormatConvert(uint8_t* sendInfo, uint8_t* readInfo,
							    uint8_t  sendPackNumb)
{
	uint8_t i;

	for (i = 0; i < sendPackNumb; i++)
	{
		FormatConvert((GPRS_SendInfoTypedef*)sendInfo, (FILE_SaveStructTypedef*)readInfo);
		readInfo += sizeof(FILE_SaveStructTypedef);
		sendInfo += sizeof(GPRS_SendInfoTypedef);
	}
}

/*******************************************************************************
 * @brief 将储存的ASCII码转换成float
 * @retval 模拟量ASCII码
 */
float FILE_Analog2Float(SaveInfoAnalogTypedef* value)
{
	char str[6];

	memcpy(str, value->value, 5);
	str[5] = '\0';
	return (float)atof(str);
}

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

	*pBuffer = HALFWORD_BYTE_H(temp);
	*(pBuffer + 1) = HALFWORD_BYTE_L(temp);

	/* 负数则最高位置一 */
	if (negative)
		*pBuffer |= 0x80;
}

/*******************************************************************************
 * @brief 将字符串型的定位值转换成协议格式
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
 * @brief 将储存的格式（ASCII码）转换成协议规定的格式
 */
static void FormatConvert(GPRS_SendInfoTypedef*   sendInfo,
						  FILE_SaveStructTypedef* readInfo)
{
	char str[4];

	/* 结构体复位，避免数据出错 */
	memset(sendInfo, 0, sizeof(GPRS_SendInfoTypedef));

	/* 时间字符串转换成BCD */
	ASCII2BCD(&sendInfo->year,  readInfo->year,  2);
	ASCII2BCD(&sendInfo->month, readInfo->month, 2);
	ASCII2BCD(&sendInfo->day,   readInfo->day,   2);
	ASCII2BCD(&sendInfo->hour,  readInfo->hour,  2);
	ASCII2BCD(&sendInfo->min,   readInfo->min,   2);
	ASCII2BCD(&sendInfo->sec,   readInfo->sec,   2);

	/* 转换电池电量 */
	memcpy(str, readInfo->batQuality, 3);
	str[3] = '\0';
	sendInfo->batteryLevel = atoi(str);

	/* 转换外部电源状态 */
	str2numb(&sendInfo->externalPowerStatus, (uint8_t*)&readInfo->exPwrStatus, 1);

	/* 转换经度 */
	LocationFormatConvert(readInfo->longitude, (uint8_t*)&sendInfo->longitude);
	LocationFormatConvert(readInfo->latitude,  (uint8_t*)&sendInfo->latitude);

	AnalogDataFormatConvert(readInfo->analogValue[0].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[0]);
	AnalogDataFormatConvert(readInfo->analogValue[1].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[1]);
	AnalogDataFormatConvert(readInfo->analogValue[2].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[2]);
	AnalogDataFormatConvert(readInfo->analogValue[3].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[3]);
	AnalogDataFormatConvert(readInfo->analogValue[4].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[4]);
	AnalogDataFormatConvert(readInfo->analogValue[5].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[5]);
	AnalogDataFormatConvert(readInfo->analogValue[6].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[6]);
	AnalogDataFormatConvert(readInfo->analogValue[7].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[7]);
}



/*******************************************************************************
 * @brief 数据储存结构体中添加符号，使其能用Excel软件打开
 */
static void SaveStructSymbolInit(void)
{
	FILE_SaveStruct.str1   = ' ';
	FILE_SaveStruct.str2   = ':';
	FILE_SaveStruct.str3   = ':';
	FILE_SaveStruct.str4   = ',';
	FILE_SaveStruct.str5   = ',';
	FILE_SaveStruct.str6   = ',';
	FILE_SaveStruct.end[0] = 0x0D;
	FILE_SaveStruct.end[1] = 0x0A;
}
























