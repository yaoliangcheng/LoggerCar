#include "exFlash.h"




/*******************************************************************************
 * 发送flash写使能指令
 */
static void exFLASH_WriteEnable(void)
{
	uint8_t enableCmd = exFLASH_CMD_WRITE_ENABLE;

	exFLASH_CS_ENABLE();

	HAL_SPI_Transmit(&exFLASH_SPI, &enableCmd, 1, 100);

	exFLASH_CS_DISABLE();
}

/*******************************************************************************
 * FLASH 芯片向内部存储矩阵写入数据需要消耗一定的时间，并不是在总线通讯结束的一瞬间完成的，所以在写操作
 * 后需要确认FLASH芯片“空闲”时才能再次写入
 */
static void exFLASH_WaitForIdle(void)
{
	uint8_t readRegStatus = exFLASH_CMD_READ_STATUS_REG;
	uint8_t sendByte = DUMMY_BYTE;
	uint8_t readByte = 0x01;

	exFLASH_CS_ENABLE();

	HAL_SPI_Transmit(&exFLASH_SPI, &readRegStatus, 1, 100);

	/* flash正忙，等待，寄存器的最后一位为BUSY位 */
	while(readByte & 0x01)
	{
		HAL_SPI_TransmitReceive(&exFLASH_SPI, &sendByte, &readByte, 1, 100);
	}

	exFLASH_CS_DISABLE();
}

/*******************************************************************************
 *
 */
static void exFLASH_SendCmdAddr(uint8_t cmd, uint32_t addr)
{
	/* 发送扇区擦除指令，和扇区地址（高位在前） */
	uint8_t sendBytes[4] = {cmd,
							(addr & 0xFF0000 >> 16),
							(addr & 0x00FF00 >> 8),
							(addr & 0x0000FF)};

	HAL_SPI_Transmit(&exFLASH_SPI, sendBytes, sizeof(sendBytes), 100);
}

/*******************************************************************************
 * 注意：地址对齐4KB
 */
void exFLASH_SectorErase(uint32_t sectorAddr)
{
	exFLASH_WriteEnable();
	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	/* 发送扇区擦除命令和地址 */
	exFLASH_SendCmdAddr(exFLASH_CMD_SECTOR_ERASE, sectorAddr);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}

/*******************************************************************************
 *
 */
void exFLASH_ChipErase(void)
{
	/* 发送扇区擦除指令，和扇区地址（高位在前） */
	uint8_t chipEraseCmd = exFLASH_CMD_CHIP_ERASE;

	exFLASH_WriteEnable();
	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	HAL_SPI_Transmit(&exFLASH_SPI, &chipEraseCmd, 1, 100);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}

/******************************************************************************/
uint32_t exFLASH_ReadDeviceID(void)
{
	uint8_t temp[4] = {exFLASH_CMD_JEDEC_DIVICE_ID,
					   DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE};

	uint32_t deviceID;

//	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	HAL_SPI_TransmitReceive(&exFLASH_SPI, temp, temp, sizeof(temp), 1000);

	exFLASH_CS_DISABLE();

	deviceID = (temp[1] << 16) | (temp[2] << 8) | temp[3];
	return deviceID;
}

/*******************************************************************************
 *
 */
static void exFLASH_WritePageBytes(uint32_t writeAddr, uint8_t* pBuffer,
								 uint16_t dataLength)
{
	/* 写数据的长度不能大于flash的页字节 */
	if (dataLength > exFLASH_PAGE_SIZE_MAX)
		return;

	exFLASH_WriteEnable();

	exFLASH_CS_ENABLE();

	/* 发送命令和地址 */
	exFLASH_SendCmdAddr(exFLASH_CMD_PAGE_PROGRAM, writeAddr);

	/* 发送数据 */
	HAL_SPI_Transmit(&exFLASH_SPI, pBuffer, dataLength, 100);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}

/*******************************************************************************
 *
 */
void exFLASH_WriteBuffer(uint32_t writeAddr, uint8_t* pBuffer, uint16_t dataLength)
{
	uint8_t pageBytesRemainder;				/* 该页剩余可写字节数 */
	uint8_t pageWriteNumb;					/* 需要写页数 */
	uint8_t dataBytesRemainder;				/* 最后一页还剩字节数 */

	/* 写地址与flash页地址对齐 */
	if (0 == (writeAddr % FLASH_PAGE_SIZE))
	{
		/* 字节长度<=页字节长度 */
		if (dataLength <=  FLASH_PAGE_SIZE)
			exFLASH_WritePageBytes(writeAddr, pBuffer, dataLength);
		else
		{
			/* 需要写的页数 */
			pageWriteNumb = dataLength / FLASH_PAGE_SIZE;
			/* 写完页，还需写的字节数 */
			dataBytesRemainder = dataLength % FLASH_PAGE_SIZE;

			while (pageWriteNumb--)
			{
				exFLASH_WritePageBytes(writeAddr, pBuffer, FLASH_PAGE_SIZE);
				writeAddr += FLASH_PAGE_SIZE;
				pBuffer += FLASH_PAGE_SIZE;
			}
			exFLASH_WritePageBytes(writeAddr, pBuffer, dataBytesRemainder);
		}
	}
	else
	{
		/* 当前页可写入字节数 */
		pageBytesRemainder = FLASH_PAGE_SIZE - (writeAddr % FLASH_PAGE_SIZE);

		/* 当前页可以写完 */
		if (dataLength <= pageBytesRemainder)
			exFLASH_WritePageBytes(writeAddr, pBuffer, dataLength);
		else
		{
			/* 先把当前页写满 */
			exFLASH_WritePageBytes(writeAddr, pBuffer, pageBytesRemainder);
			writeAddr += pageBytesRemainder;
			pBuffer += pageBytesRemainder;
			dataLength -= pageBytesRemainder;

			/* 需要写的页数 */
			pageWriteNumb = dataLength / FLASH_PAGE_SIZE;
			while (pageWriteNumb--)
			{
				exFLASH_WritePageBytes(writeAddr, pBuffer, FLASH_PAGE_SIZE);
				writeAddr += FLASH_PAGE_SIZE;
				pBuffer += FLASH_PAGE_SIZE;
			}

			/* 写完页，还需写的字节数 */
			dataBytesRemainder = dataLength % FLASH_PAGE_SIZE;
			if (dataBytesRemainder > 0)
				exFLASH_WritePageBytes(writeAddr, pBuffer, dataBytesRemainder);
		}
	}
}

/*******************************************************************************
 *
 */
void exFLASH_ReadBuffer(uint32_t readAddr, uint8_t* pBuffer, uint16_t dataLength)
{
	exFLASH_CS_ENABLE();

	exFLASH_SendCmdAddr(exFLASH_CMD_READ_DATA, readAddr);

	HAL_SPI_Receive(&exFLASH_SPI, pBuffer, dataLength, 100);

	exFLASH_CS_DISABLE();
}

/*******************************************************************************
 * 模拟量数据格式转换
 */
static void exFLASH_DataFormatConvert(float value, EE_DataFormatEnum format,
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

uint32_t flashDeviceID = 0;
/*******************************************************************************
 *
 */
void exFLASH_Init(void)
{
//	exFLASH_ChipErase();
	flashDeviceID = exFLASH_ReadDeviceID();
}

/*******************************************************************************
 *
 */
void exFLASH_SaveStructInfo(RT_TimeTypedef* realTime,
							ANALOG_ValueTypedef* analogValue,
							EE_DataFormatEnum format)
{
	exFLASH_InfoTypedef exFLASH_SaveInfo;

	/* 结构体复位，避免数据出错 */
	memset(&exFLASH_SaveInfo, 0, sizeof(exFLASH_SaveInfo));

	/* 获取时钟 */
	exFLASH_SaveInfo.realTime.year = realTime->date.Year;
	exFLASH_SaveInfo.realTime.month = realTime->date.Month;
	exFLASH_SaveInfo.realTime.day = realTime->date.Date;
	exFLASH_SaveInfo.realTime.hour = realTime->time.Hours;
	exFLASH_SaveInfo.realTime.min = realTime->time.Minutes;
	/* 为了数据的整齐，将秒置位0 */
	exFLASH_SaveInfo.realTime.sec = 0;

	/* 储存电池电量 */
	exFLASH_SaveInfo.batteryLevel = analogValue->batVoltage;

	/* 外部电池状态 */
	exFLASH_SaveInfo.externalPowerStatus = 1;

	/* 模拟数据格式转换 */
	exFLASH_DataFormatConvert(analogValue->temp1, format,
			(uint8_t*)&exFLASH_SaveInfo.analogValue.temp1);
	exFLASH_DataFormatConvert(analogValue->humi1, format,
			(uint8_t*)&exFLASH_SaveInfo.analogValue.humi1);
	exFLASH_DataFormatConvert(analogValue->temp2, format,
			(uint8_t*)&exFLASH_SaveInfo.analogValue.temp2);
	exFLASH_DataFormatConvert(analogValue->humi2, format,
			(uint8_t*)&exFLASH_SaveInfo.analogValue.humi2);
	exFLASH_DataFormatConvert(analogValue->temp3, format,
			(uint8_t*)&exFLASH_SaveInfo.analogValue.temp3);
	exFLASH_DataFormatConvert(analogValue->humi3, format,
			(uint8_t*)&exFLASH_SaveInfo.analogValue.humi3);
	exFLASH_DataFormatConvert(analogValue->temp4, format,
			(uint8_t*)&exFLASH_SaveInfo.analogValue.temp4);
	exFLASH_DataFormatConvert(analogValue->humi4, format,
			(uint8_t*)&exFLASH_SaveInfo.analogValue.humi4);

	exFLASH_WriteBuffer(EE_FlashInfoSaveAddr, (uint8_t*)&exFLASH_SaveInfo,
			sizeof(exFLASH_InfoTypedef));

	EE_FlashInfoSaveAddr += sizeof(exFLASH_InfoTypedef);
	EEPROM_WriteBytes(EE_ADDR_FLASH_INFO_SAVE_ADDR, &EE_FlashInfoSaveAddr,
			sizeof(EE_FlashInfoSaveAddr));
}

/*******************************************************************************
 *
 */
void exFLASH_ReadStructInfo(exFLASH_InfoTypedef* info)
{
	exFLASH_ReadBuffer(EE_FlashInfoReadAddr, (uint8_t*)info,
			sizeof(exFLASH_InfoTypedef));

	EE_FlashInfoReadAddr += sizeof(exFLASH_InfoTypedef);
	EEPROM_WriteBytes(EE_ADDR_FLASH_INFO_READ_ADDR, &EE_FlashInfoReadAddr,
			sizeof(exFLASH_InfoTypedef));
}
































