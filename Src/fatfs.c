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
FATFS objFileSystem;			/* FatFs�ļ�ϵͳ���� */
FIL   objFile;					/* �ļ����� */
DWORD freeClust, freeSect, totSect;	/* ���дء����������������� */

/*************************************************************************************/
ErrorStatus FATFS_FileMake(void);

/* USER CODE END Variables */    

void MX_FATFS_Init(void) 
{
  /*## FatFS: Link the USER driver ###########################*/
  retUSER = FATFS_LinkDriver(&USER_Driver, USER_Path);

  /* USER CODE BEGIN Init */
  /* additional user code for init */

  /* USER CODE END Init */
}

/* USER CODE BEGIN Application */
/*******************************************************************************
 *
 */
ErrorStatus FATFS_FileLink(void)
{
	FRESULT status;

	/* ����spi flash */
	status = f_mount(&objFileSystem, USER_Path, 1);

	if (status == FR_OK)
		return SUCCESS;
	else if(status == FR_NO_FILESYSTEM)	/* ���û���ļ�ϵͳ�͸�ʽ�����������ļ�ϵͳ */
	{
		/* ��ʽ�� */
		if (SUCCESS == FATFS_FileMake())
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
ErrorStatus FATFS_FileUnlink(void)
{
	/* ����ʹ���ļ�ϵͳ��ȡ�������ļ�ϵͳ */
	 if (FR_OK == f_mount(NULL, USER_Path, 1))
		 return SUCCESS;
	 else
		 return ERROR;
}

/*******************************************************************************
 * function:�ļ�ϵͳ��ʽ��
 */
ErrorStatus FATFS_FileMake(void)
{
	if (FR_OK == f_mkfs(USER_Path, 0, 0))
	{
		/* ��ʽ������ȡ������ */
		f_mount(NULL, USER_Path, 1);

		/* ���¹���	*/
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
 * @pBuffer:Ҫд�������ָ��
 * @size��Ҫд�����ݵĳ���
 */
ErrorStatus FATFS_FileWrite(BYTE* pBuffer, BYTE size)
{
	uint32_t byteWrite;

	if (FR_OK == f_write(&objFile, pBuffer, size, &byteWrite))
	{
		printf("д��%d�ֽ�\r\n", byteWrite);
		/* Ҫд��ĺ�ʵ��д��ı�����ͬ */
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
 ** @pBuffer:Ҫ����������ָ��
 * @size��Ҫ�������ݵĳ���
 */
ErrorStatus FATFS_FileRead(BYTE* pBuffer, BYTE size)
{
	uint32_t byteRead;

	if (FR_OK == f_read(&objFile, pBuffer, size, &byteRead))
	{
		printf("����%d�ֽ�\r\n", byteRead);
		/* Ҫ�����ĺ�ʵ�ʶ����ı�����ͬ */
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

	/* ��ȡ�豸��Ϣ�Ϳմش�С */
	if (FR_OK == f_getfree(USER_Path, &freeClust, &pfs))
	{
		/* ��λΪKB */
		freeSect  = (pfs->n_fatent - 2) * pfs->csize * 4;
		totSect   = freeClust           * pfs->csize * 4;

		printf("�豸�ܿռ䣺%ulKB ���ÿռ䣺%ulKB\r\n", freeSect, totSect);

		/* û�пռ��д */
		if (freeSect == 0)
		{
			printf("�޿��ÿռ䣡���뱸�ݺ����ݣ���ʽ������\r\n");
			return ERROR;
		}
		else
			return SUCCESS;
	}
	else
		return ERROR;
}

/*******************************************************************************
 * function:������дָ���ƶ����ļ���ĩβ
 */
ErrorStatus FATFS_FileSeekEnd(void)
{
	if (objFile.fsize != 0)
	{
		if (FR_OK == f_lseek(&objFile, objFile.fsize - 1))
			return SUCCESS;
		else
			return ERROR;
	}

	return SUCCESS;
}

/*******************************************************************************
 * function������дָ���ƶ����ļ�ĩβ�ĺ���ָ���ֽ�
 * @backwardByt:Ҫ���ȵ��ֽ���
 */
ErrorStatus FATFS_FileSeekBackward(WORD backwardByte)
{
	if (objFile.fsize != 0)
	{
		if (FR_OK == f_lseek(&objFile, objFile.fsize - backwardByte))
			return SUCCESS;
		else
			return ERROR;
	}

	return SUCCESS;
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/