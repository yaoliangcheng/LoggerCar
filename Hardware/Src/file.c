#include "file.h"

/******************************************************************************/
FILE_PatchPackTypedef FILE_PatchPack;		/* 补传文件信息 */
uint64_t FILE_DataSaveStructCnt;			/* 数据储存文件结构体总数 */


FILE_SaveStructTypedef FILE_SaveStruct;			/* 储存信息写入结构体 */
FILE_SaveStructTypedef FILE_ReadStruct[SEND_PACK_CNT_MAX];
												/* 储存信息读取结构体 */

/******************************************************************************/
extern FATFS objFileSystem;
extern FIL   objFile;

/******************************************************************************/
static void SaveStructSymbolInit(void);
static void Analog2ASCII(FILE_SaveInfoAnalogTypedef* buffer, float value);

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
		f_write(&objFile, pBuffer, size, &byteWrite);
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
static void FILE_SaveInfo(BYTE* saveInfo, UINT size)
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
		f_write(&objFile, saveInfo, size, &byteWrite);
		/* 获取文件中结构体数 */
		FILE_DataSaveStructCnt = objFile.fsize / sizeof(FILE_SaveStructTypedef);
		f_close(&objFile);
	}

	/* 退出临界区 */
	taskEXIT_CRITICAL();
	f_mount(NULL, USERPath, 1);
}

/*******************************************************************************
 *
 */
void FILE_SaveSendInfo(FILE_SaveStructTypedef* saveInfo, RT_TimeTypedef* curtime,
		GPS_LocateTypedef* location, ANALOG_ValueTypedef* analog)
{
	/* 时间转换 */
	HEX2ASCII(saveInfo->year,  &curtime->date.Year,    1);
	HEX2ASCII(saveInfo->month, &curtime->date.Month,   1);
	HEX2ASCII(saveInfo->day,   &curtime->date.Date,    1);
	HEX2ASCII(saveInfo->hour,  &curtime->time.Hours,   1);
	HEX2ASCII(saveInfo->min,   &curtime->time.Minutes, 1);
	HEX2ASCII(saveInfo->sec,   &curtime->time.Seconds, 1);
	/* 获取外部电源状态 */
	FILE_SaveStruct.exPwrStatus = INPUT_CheckPwrOnStatus() + '0';

	/* 模拟量转换为ASCII */
	sprintf(saveInfo->batQuality, "%3d", ANALOG_value.batVoltage);
	Analog2ASCII(&saveInfo->analogValue[0], ANALOG_value.channel1);
	Analog2ASCII(&saveInfo->analogValue[1], ANALOG_value.channel2);
	Analog2ASCII(&saveInfo->analogValue[2], ANALOG_value.channel3);
	Analog2ASCII(&saveInfo->analogValue[3], ANALOG_value.channel4);
	Analog2ASCII(&saveInfo->analogValue[4], ANALOG_value.channel5);
	Analog2ASCII(&saveInfo->analogValue[5], ANALOG_value.channel6);
	Analog2ASCII(&saveInfo->analogValue[6], ANALOG_value.channel7);
	Analog2ASCII(&saveInfo->analogValue[7], ANALOG_value.channel8);


	/* 定位值 */
	saveInfo->locationStatus = GPS_LOCATION_TYPE_GPS + '0';
	sprintf(saveInfo->longitude, "%10.5f", location->longitude);
	sprintf(saveInfo->latitude,  "%10.5f", location->latitude);

	/* CVS文件格式 */
	saveInfo->batQuality[3] = '%';		/* 电池电量百分号 */
	saveInfo->str8   		= ',';
	saveInfo->str9   		= ',';

	/* 储存数据 */
	FILE_SaveInfo((uint8_t*)saveInfo, sizeof(FILE_SaveStructTypedef));
}

/*******************************************************************************
 *
 */
uint8_t FILE_ReadSaveInfo(FILE_SaveStructTypedef* readInfo, uint32_t* structoffset)
{
	uint16_t sendPackCnt = 0;			/* 发送的包数 */
	uint64_t offset = 0;

	/* 重新申请一个变量记录指针内容，避免指针冲突 */
	offset = *structoffset;

	/* 代表没有补传数据，读取最新的一条数据 */
	if (offset == 0)
	{
		FILE_ReadFile(FILE_NAME_SAVE_DATA,
			(FILE_DataSaveStructCnt - 1) * sizeof(FILE_SaveStructTypedef),
			(uint8_t*)readInfo, sizeof(FILE_SaveStructTypedef));
		return 1;
	}

	sendPackCnt = FILE_DataSaveStructCnt - offset;
	/* 不能够一次性发送完成 */
	if (sendPackCnt > SEND_PACK_CNT_MAX)
	{
		sendPackCnt = SEND_PACK_CNT_MAX;
		offset += SEND_PACK_CNT_MAX;
		*structoffset = offset;
	}
	else
	{
		/* 已经补传完成 */
		*structoffset = 0;
	}
	FILE_ReadFile(FILE_NAME_SAVE_DATA,
			offset * sizeof(FILE_SaveStructTypedef),
			(uint8_t*)readInfo, sendPackCnt * sizeof(FILE_SaveStructTypedef));

	/* 返回本次读取的结构体数 */
	return sendPackCnt;
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
	FILE_SaveStruct.str7   = ',';
	FILE_SaveStruct.end[0] = 0x0D;
	FILE_SaveStruct.end[1] = 0x0A;
}

/*******************************************************************************
 * @brief 判断模拟量值是否有效，无效则填充“ NULL”，有效则转换成ASCII
 */
static void Analog2ASCII(FILE_SaveInfoAnalogTypedef* buffer, float value)
{
	if (value == ANALOG_CHANNLE_INVALID_VALUE)
		memcpy(buffer->value, ANALOG_INVALID_VALUE, 5);
	else
	{
		/* %5.1表示有效数据长度为5，小数1位 */
		sprintf(buffer->value, "%5.1f", value);
		buffer->str = ',';
	}
}






















