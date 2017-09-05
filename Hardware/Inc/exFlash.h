#ifndef __EX_FLASH_H
#define __EX_FLASH_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "common.h"

#include "spi.h"
#include "input.h"
#include "gps.h"

#include "RealTime.h"


/******************************************************************************/
#define SPI_FLASH_CS_LOW()  \
		HAL_GPIO_WritePin(SPI_CS_FLASH_GPIO_Port, SPI_CS_FLASH_Pin, GPIO_PIN_RESET);
#define SPI_FLASH_CS_HIGH() \
		HAL_GPIO_WritePin(SPI_CS_FLASH_GPIO_Port, SPI_CS_FLASH_Pin, GPIO_PIN_SET);

#define exFLASH_SPI		(hspi1)

/*******************************************************************************
*W25Q常用指令
*/
//#define  sFLASH_ID              0xEF3015   //W25X16
//#define  sFLASH_ID              0xEF4015	 //W25Q16
//#define  sFLASH_ID              0XEF4018   //W25Q128
#define  sFLASH_ID              		(0XEF4017)    //W25Q64

#define SPI_FLASH_PageSize              (256)
#define SPI_FLASH_PerWritePageSize      (256)

/*命令定义-开头*******************************/
#define W25X_WriteEnable		      	(0x06)
#define W25X_WriteDisable		      	(0x04)
#define W25X_ReadStatusReg		    	(0x05)
#define W25X_WriteStatusReg		    	(0x01)
#define W25X_ReadData			        (0x03)
#define W25X_FastReadData		      	(0x0B)
#define W25X_FastReadDual		      	(0x3B)
#define W25X_PageProgram		      	(0x02)
#define W25X_BlockErase			      	(0xD8)
#define W25X_SectorErase		      	(0x20)
#define W25X_ChipErase			      	(0xC7)
#define W25X_PowerDown			      	(0xB9)
#define W25X_ReleasePowerDown	    	(0xAB)
#define W25X_DeviceID			        (0xAB)
#define W25X_ManufactDeviceID   		(0x90)
#define W25X_JedecDeviceID		    	(0x9F)

#define Dummy_Byte                		(0xFF)

#define exFLASH_SECTOR_SIZE_BYTES		(4096)
#define exFLASH_SECTOR_COUNT			(2048)
#define exFLASH_SECTOR_OFFSET			(4096)
#define exFLASH_BLOCK_SIZE				(1)		/* 扇区数 */

#define WIP_Flag                  		(0x01)	/* WIP(busy)标志，FLASH内部正在写入 */




/******************************************************************************/
void SPI_FLASH_Init(void);
void SPI_FLASH_SectorErase(uint32_t SectorAddr);
void SPI_FLASH_BulkErase(void);
void SPI_FLASH_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void SPI_FLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void SPI_FLASH_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
uint32_t SPI_FLASH_ReadID(void);
uint32_t SPI_FLASH_ReadDeviceID(void);
void SPI_FLASH_StartReadSequence(uint32_t ReadAddr);
void SPI_Flash_PowerDown(void);
void SPI_Flash_WAKEUP(void);


uint8_t SPI_FLASH_ReadByte(void);
uint8_t SPI_FLASH_SendByte(uint8_t byte);
void SPI_FLASH_WriteEnable(void);
void SPI_FLASH_WaitForWriteEnd(void);

#endif
