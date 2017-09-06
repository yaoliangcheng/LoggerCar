/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include "fatfs.h"

uint8_t retUSER;    /* Return value for USER */
char USER_Path[4];  /* USER logical drive path */

/* USER CODE BEGIN Variables */
#include "file.h"
#include "param.h"
#include "osConfig.h"

FATFS objFileSystem;			/* FatFs文件系统对象 */
FIL   objFile;					/* 文件对象 */
DWORD freeClust, freeSect, totSect;	/* 空闲簇、空闲扇区、总扇区 */

/*************************************************************************************/
ErrorStatus FATFS_FileMake(void);

/* USER CODE END Variables */    

void MX_FATFS_Init(void) 
{
  /*## FatFS: Link the USER driver ###########################*/
  retUSER = FATFS_LinkDriver(&USER_Driver, USER_Path);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
  	/* 文件格式化，获取数据储存文件的结构体总数 */
  	FILE_Init();
  PARAM_ParamFileInit();
  /* 在文件系统初始化完成后，在启动GPRS任务 */
  osThreadResume(gprsprocessTaskHandle);
  /* USER CODE END Init */
}

/* USER CODE BEGIN Application */
/*******************************************************************************
 *
 */
ErrorStatus FATFS_FileLink(void)
{
	FRESULT status;

	/* 挂载spi flash */
	status = f_mount(&objFileSystem, USER_Path, 1);

	if (status == FR_OK)
	{
		/* 进入临界区 */
		taskENTER_CRITICAL();
		return SUCCESS;
	}
	else if(status == FR_NO_FILESYSTEM)	/* 如果没有文件系统就格式化创建创建文件系统 */
	{
		/* 格式化 */
		if (SUCCESS == FATFS_FileMake())
		{
			/* 进入临界区 */
			taskENTER_CRITICAL();
			return SUCCESS;
		}
		else
			return ERROR;
	}
	else
		return ERROR;
}

/*******************************************************************************
 *
 */
ErrorStatus FATFS_FileUnlink(void)
{
	/* 不再使用文件系统，取消挂载文件系统 */
	 if (FR_OK == f_mount(NULL, USER_Path, 1))
	 {
		 /* 退出临界区 */
		 taskEXIT_CRITICAL();
		 return SUCCESS;
	 }
	 else
		 return ERROR;
}

/*******************************************************************************
 * function:文件系统格式化
 */
ErrorStatus FATFS_FileMake(void)
{
	SPI_FLASH_BulkErase();
	
	if (FR_OK == f_mkfs(USER_Path, 0, 4096))
	{
		/* 格式化后，先取消挂载 */
		f_mount(NULL, USER_Path, 1);

		/* 重新挂载	*/
		if (FR_OK == f_mount(&objFileSystem, USER_Path, 1))
			return SUCCESS;
		else
			return ERROR;
	}
	else
		return ERROR;
}

/*******************************************************************************
 *
 */
ErrorStatus FATFS_FileOpen(char* fileName, FATFS_ModeEnum mode)
{
	FRESULT status;

	switch (mode)
	{
	case FATFS_MODE_OPEN_ALWAYS_WRITE:
		status = f_open(&objFile, fileName, FA_OPEN_ALWAYS | FA_WRITE);
		break;

	case FATFS_MODE_OPEN_EXISTING_READ:
		status = f_open(&objFile, fileName, FA_OPEN_EXISTING | FA_READ);
		break;

	default:
		break;
	}

	if (status == FR_OK)
		return SUCCESS;
	else
		return ERROR;
}

/*******************************************************************************
 *
 * @pBuffer:要写入的数据指针
 * @size：要写入数据的长度
 */
ErrorStatus FATFS_FileWrite(BYTE* pBuffer, WORD size)
{
	uint32_t byteWrite;

	if (FR_OK == f_write(&objFile, pBuffer, size, &byteWrite))
	{
		/* 要写入的和实际写入的必须相同 */
		if (byteWrite == size)
			return SUCCESS;
		else
			return ERROR;
	}
	else
		return ERROR;
}

/*******************************************************************************
 *
 ** @pBuffer:要读出的数据指针
 * @size：要读出数据的长度
 */
ErrorStatus FATFS_FileRead(BYTE* pBuffer, WORD size)
{
	uint32_t byteRead;

	if (FR_OK == f_read(&objFile, pBuffer, size, &byteRead))
	{
		/* 要读出的和实际读出的必须相同 */
		if (byteRead == size)
			return SUCCESS;
		else
			return ERROR;
	}
	else
		return ERROR;
}

/*******************************************************************************
 *
 */
ErrorStatus FATFS_FileClose(void)
{
	if (FR_OK == f_close(&objFile))
		return SUCCESS;
	else
		return ERROR;
}

/*******************************************************************************
 *
 */
ErrorStatus FATFS_GetSpaceInfo(void)
{
	FATFS* pfs;

	/* 获取设备信息和空簇大小 */
	if (FR_OK == f_getfree(USER_Path, &freeClust, &pfs))
	{
		/* 单位为KB */
		totSect  = (pfs->n_fatent - 2) * pfs->csize * 4;
		freeSect = freeClust           * pfs->csize * 4;

		printf("设备剩余储存空间：%d%%\r\n", freeSect * 100 / totSect);

		/* 没有空间可写 */
		if (freeSect == 0)
		{
			printf("无可用空间！！请备份好数据，格式化磁盘\r\n");
			return ERROR;
		}
		else
			return SUCCESS;
	}
	else
		return ERROR;
}

/*******************************************************************************
 * function:将读、写指针移动到文件的末尾
 */
ErrorStatus FATFS_FileSeekEnd(void)
{
	if (FR_OK == f_lseek(&objFile, objFile.fsize))
		return SUCCESS;
	else
		return ERROR;
}

/*******************************************************************************
 * function:判断当前写入的地址是否结构体对齐，防止储存的数据错位，当结构体不对齐时，覆盖缺省的数据空间，
 * 			。（这个函数用于数据储存过程）
 */
void FATFS_FileSeekSaveInfoStructAlign(void)
{
	if (objFile.fsize % sizeof(FILE_SaveInfoTypedef) == 0)
	{
		f_lseek(&objFile, objFile.fsize);
	}
	else
	{
		/* 覆盖当前结构体 */
		f_lseek(&objFile,
				(objFile.fsize / sizeof(FILE_SaveInfoTypedef))
				* sizeof(FILE_SaveInfoTypedef));
	}
}

/*******************************************************************************
 * function:将读、写指针移动到文件的末尾向前一个结构体
 */
ErrorStatus FATFS_FileSeekBackwardOnePack(void)
{
	if (FR_OK == f_lseek(&objFile, objFile.fsize - sizeof(FILE_SaveInfoTypedef)))
		return SUCCESS;
	else
		return ERROR;
}

/*******************************************************************************
 *
 */
ErrorStatus FATFS_FileSeek(WORD byte)
{
	if (byte <= objFile.fsize)
	{
		if (FR_OK == f_lseek(&objFile, byte))
			return SUCCESS;
		else
			return ERROR;
	}
	else
		return ERROR;
}

/*******************************************************************************
 *
 */
ErrorStatus FATFS_CreateFile(char* fileName)
{
	if (ERROR == FATFS_FileLink())
		return ERROR;

	if (ERROR == FATFS_FileOpen(fileName, FATFS_MODE_OPEN_ALWAYS_WRITE))
		return ERROR;

	if (ERROR == FATFS_FileClose())
		return ERROR;

	if (ERROR == FATFS_FileUnlink())
		return ERROR;

	return SUCCESS;
}

/*******************************************************************************
 * function:返回该文件中结构体数量
 */
uint16_t FATFS_GetFileStructCount(void)
{
	return objFile.fsize / sizeof(FILE_SaveInfoTypedef);
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
