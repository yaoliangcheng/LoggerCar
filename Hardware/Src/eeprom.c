#include "eeprom.h"

#include "exFlash.h"

/******************************************************************************/
uint8_t EE_DeviceInit = 0xAA;							/* EE储存标志位 */
char 	EE_deviceSN[10] = "1707041010";					/* 设备SN号 */
uint8_t EE_firmwareVersion = 10;						/* 固件版本号 */
uint8_t EE_recordInterval = 1;							/* 记录间隔 */
uint8_t EE_overLimitRecordInterval = 2;					/* 超标记录间隔 */
uint8_t EE_exitAnalogChannelNumb = 8;					/* 外部模拟量通道数 */
EE_ParamTypedef EE_Param[ANALOG_CHANNEL_NUMB_MAX];		/* 通道参数信息 */

uint32_t EE_FlashInfoSaveAddr = 0;						/* 外部Flash结构体信息保存地址 */
uint32_t EE_FlashInfoReadAddr = 0;						/* 外部Flash结构体信息保存地址 */
uint32_t EE_FlashDeviceID = 0;							/* 外部flash型号 */

/*******************************************************************************
 * @function：等待eeprom空闲
 * 如果eeprom正在执行内部的写操作，则不会响应主机的应答信号。
 */
static ErrorStatus EEPROM_WaitForIdle(void)
{
	uint16_t tmp = RESET;

	do
	{
		EEPROM_I2C.Instance->CR1 &= (1 << 8);

		/* 读 I2C->SR1 寄存器 */
		tmp = EEPROM_I2C.Instance->SR1;
		tmp = tmp;

		/* 发送 EEPROM 地址 + 写方向 */
		EEPROM_I2C.Instance->DR = EEPROM_DEVICE_ADDR;
	}
	while ((EEPROM_I2C.Instance->SR1) & 0x0002);

	/* 清除AF标志位 */
	EEPROM_I2C.Instance->SR1 |= (uint16_t)~(1 << 10);

	/* 产生停止信号 */
	EEPROM_I2C.Instance->CR1 &= (1 << 9);

	return SUCCESS;
}

/*******************************************************************************
*
*/
void EEPROM_WriteBytes(uint16_t addr, void* pBuffer, uint8_t dataLength)
{
	EEPROM_WaitForIdle();
	
	if (addr >= 256)
		HAL_I2C_Mem_Write(&EEPROM_I2C, EEPROM_DEVICE_ADDR & 0x20, addr,
				sizeof(uint8_t), pBuffer, dataLength, 100);
	else
		HAL_I2C_Mem_Write(&EEPROM_I2C, EEPROM_DEVICE_ADDR, addr,
				sizeof(uint8_t), pBuffer, dataLength, 100);
}

void EEPROM_ReadBytes(uint16_t addr, uint8_t* pBuffer, uint8_t dataLength)
{
	if (addr >= 256)
		HAL_I2C_Mem_Read(&EEPROM_I2C, EEPROM_DEVICE_ADDR & 0x20, addr,
				sizeof(uint8_t), pBuffer, dataLength, 100);
	else
		HAL_I2C_Mem_Read(&EEPROM_I2C, EEPROM_DEVICE_ADDR, addr,
				sizeof(uint8_t), pBuffer, dataLength, 100);
}

/*******************************************************************************
 *
 */
void DEVICE_Init(void)
{
	EEPROM_ReadBytes(EE_ADDR_DEVICE_INIT, &EE_DeviceInit, sizeof(EE_DeviceInit));
	if (EE_DeviceInit != 0xBB)
	{
//		EEPROM_WriteBytes(EE_ADDR_DEVICE_SN, EE_deviceSN, sizeof(EE_deviceSN));
//		EEPROM_WriteBytes(EE_ADDR_FIRMWARE_VERSION, &EE_firmwareVersion,
//												sizeof(EE_firmwareVersion));
//		EEPROM_WriteBytes(EE_ADDR_RECORD_INTERVAL, &EE_recordInterval,
//												sizeof(EE_recordInterval));
//		EEPROM_WriteBytes(EE_ADDR_OVER_LIMIT_RECORD_INTERVAL,
//				&EE_overLimitRecordInterval, sizeof(EE_overLimitRecordInterval));
//		EEPROM_WriteBytes(EE_ADDR_EXIT_ANALOG_CHANNEL_NUMB,
//				&EE_exitAnalogChannelNumb, sizeof(EE_exitAnalogChannelNumb));

//		EE_Param[0].channelType = TYPE_TEMP;
//		EE_Param[0].channelUnit = UNIT_TEMP;
//		EE_Param[0].dataFormat = FORMAT_ONE_DECIMAL;
//		EEPROM_WriteBytes(EE_ADDR_PARAM_1, &EE_Param[0], sizeof(EE_Param));

//		EE_Param[1].channelType = TYPE_HUMI;
//		EE_Param[1].channelUnit = UNIT_HUMI;
//		EE_Param[1].dataFormat = FORMAT_ONE_DECIMAL;
//		EEPROM_WriteBytes(EE_ADDR_PARAM_2, &EE_Param[1], sizeof(EE_Param));

//		EE_Param[2].channelType = TYPE_TEMP;
//		EE_Param[2].channelUnit = UNIT_TEMP;
//		EE_Param[2].dataFormat = FORMAT_ONE_DECIMAL;
//		EEPROM_WriteBytes(EE_ADDR_PARAM_3, &EE_Param[2], sizeof(EE_Param));

//		EE_Param[3].channelType = TYPE_HUMI;
//		EE_Param[3].channelUnit = UNIT_HUMI;
//		EE_Param[3].dataFormat = FORMAT_ONE_DECIMAL;
//		EEPROM_WriteBytes(EE_ADDR_PARAM_4, &EE_Param[3], sizeof(EE_Param));

//		EE_Param[4].channelType = TYPE_TEMP;
//		EE_Param[4].channelUnit = UNIT_TEMP;
//		EE_Param[4].dataFormat = FORMAT_ONE_DECIMAL;
//		EEPROM_WriteBytes(EE_ADDR_PARAM_5, &EE_Param[4], sizeof(EE_Param));

//		EE_Param[5].channelType = TYPE_HUMI;
//		EE_Param[5].channelUnit = UNIT_HUMI;
//		EE_Param[5].dataFormat = FORMAT_ONE_DECIMAL;
//		EEPROM_WriteBytes(EE_ADDR_PARAM_6, &EE_Param[5], sizeof(EE_Param));

//		EE_Param[6].channelType = TYPE_TEMP;
//		EE_Param[6].channelUnit = UNIT_TEMP;
//		EE_Param[6].dataFormat = FORMAT_ONE_DECIMAL;
//		EEPROM_WriteBytes(EE_ADDR_PARAM_7, &EE_Param[6], sizeof(EE_Param));

//		EE_Param[7].channelType = TYPE_HUMI;
//		EE_Param[7].channelUnit = UNIT_HUMI;
//		EE_Param[7].dataFormat = FORMAT_ONE_DECIMAL;
//		EEPROM_WriteBytes(EE_ADDR_PARAM_8, &EE_Param[7], sizeof(EE_Param));

		exFLASH_ChipErase();
		/* flash储存地址归零，擦除flash整片 */
		EEPROM_WriteBytes(EE_ADDR_FLASH_INFO_SAVE_ADDR,
				(uint8_t*)&EE_FlashInfoSaveAddr, sizeof(EE_FlashInfoSaveAddr));
		EEPROM_WriteBytes(EE_ADDR_FLASH_INFO_READ_ADDR,
				(uint8_t*)&EE_FlashInfoReadAddr, sizeof(EE_FlashInfoReadAddr));

		/* 测试阶段，每次开机都重新写eeprom和exflash */
		EE_DeviceInit = 0xBB;
		EEPROM_WriteBytes(EE_ADDR_DEVICE_INIT, &EE_DeviceInit,
				sizeof(EE_DeviceInit));
	}
	else
	{
//		EEPROM_ReadBytes(EE_ADDR_DEVICE_SN, (uint8_t*)&EE_deviceSN,
//				sizeof(EE_deviceSN));
//		EEPROM_ReadBytes(EE_ADDR_FIRMWARE_VERSION,
//				(uint8_t*)&EE_firmwareVersion, sizeof(EE_firmwareVersion));
//		EEPROM_ReadBytes(EE_ADDR_RECORD_INTERVAL, (uint8_t*)&EE_recordInterval,
//				sizeof(EE_recordInterval));
//		EEPROM_ReadBytes(EE_ADDR_OVER_LIMIT_RECORD_INTERVAL,
//				(uint8_t*)&EE_overLimitRecordInterval,
//				sizeof(EE_overLimitRecordInterval));
//		EEPROM_ReadBytes(EE_ADDR_EXIT_ANALOG_CHANNEL_NUMB,
//				(uint8_t*)&EE_exitAnalogChannelNumb,
//				sizeof(EE_exitAnalogChannelNumb));
//
//		EEPROM_ReadBytes(EE_ADDR_PARAM_1, (uint8_t*)&EE_Param[0],
//				sizeof(EE_Param));
//		EEPROM_ReadBytes(EE_ADDR_PARAM_2, (uint8_t*)&EE_Param[1],
//				sizeof(EE_Param));
//		EEPROM_ReadBytes(EE_ADDR_PARAM_3, (uint8_t*)&EE_Param[2],
//				sizeof(EE_Param));
//		EEPROM_ReadBytes(EE_ADDR_PARAM_4, (uint8_t*)&EE_Param[3],
//				sizeof(EE_Param));
//		EEPROM_ReadBytes(EE_ADDR_PARAM_5, (uint8_t*)&EE_Param[4],
//				sizeof(EE_Param));
//		EEPROM_ReadBytes(EE_ADDR_PARAM_6, (uint8_t*)&EE_Param[5],
//				sizeof(EE_Param));
//		EEPROM_ReadBytes(EE_ADDR_PARAM_7, (uint8_t*)&EE_Param[6],
//				sizeof(EE_Param));
//		EEPROM_ReadBytes(EE_ADDR_PARAM_8, (uint8_t*)&EE_Param[7],
//				sizeof(EE_Param));


		EEPROM_ReadBytes(EE_ADDR_FLASH_INFO_SAVE_ADDR,
				(uint8_t*)&EE_FlashInfoSaveAddr, sizeof(EE_FlashInfoSaveAddr));
		EEPROM_ReadBytes(EE_ADDR_FLASH_INFO_READ_ADDR,
				(uint8_t*)&EE_FlashInfoReadAddr, sizeof(EE_FlashInfoReadAddr));
	}

//	EE_FlashDeviceID = exFLASH_ReadDeviceID();
}


