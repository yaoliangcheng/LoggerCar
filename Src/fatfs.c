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
FATFS FATFS_exFlashObj;				/* 文件系统对象 */
FIL   fileObj;						/* 文件对象 */
FRESULT optStatus;					/* 操作状态 */
UINT  optNumb;						/* 文件操作数量 */
BYTE  readBuf[100];
BYTE  writeBuf[] = {"hangzhoulugekejiyouxiangongsi"};
BYTE  writeBuf2[] = {"Zhejiang.Hangzhou"};

/* USER CODE END Variables */    

void MX_FATFS_Init(void) 
{
  /*## FatFS: Link the USER driver ###########################*/
  retUSER = FATFS_LinkDriver(&USER_Driver, USER_Path);

  /* USER CODE BEGIN Init */
  /* additional user code for init */

  if (0 == retUSER)
  {
	  printf("硬件层连接成功\r\n");
	  optStatus = f_mount(&FATFS_exFlashObj, "0:", 1);
	  if (optStatus == FR_NO_FILESYSTEM)
	  {
		  printf("即将进行格式化\r\n");
		  optStatus = f_mkfs("0:", 0, 0);
		  if (optStatus == FR_OK)
		  {
			  printf("格式化成功\r\n");
			  f_mount(NULL, "0:", 1);
			  f_mount(&FATFS_exFlashObj, "0:", 1);
		  }
		  else
		  {
			  printf("格式化失败\r\n");
		  }
	  }

	  optStatus = f_open(&fileObj, "0:stm32.txt", FA_CREATE_ALWAYS | FA_WRITE);
	  if (optStatus == FR_OK)
	  {
		  printf("文件打开成功\r\n");
		  optStatus = f_write(&fileObj, writeBuf, sizeof(writeBuf), &optNumb);
		  if(optStatus == FR_OK)
		  {
			  printf("文件写入成功\r\n");
		  }
		  else
		  {
			  printf("文件写入失败\r\n");
		  }
//		  optStatus = f_read(&fileObj, readBuf, sizeof(readBuf), &optNumb);
		  f_close(&fileObj);
	  }
	  else
	  {
		  printf("文件打开失败\r\n");
	  }

	  optStatus = f_open(&fileObj, "0:stm32.txt", FA_OPEN_EXISTING | FA_READ);
	  if (FR_OK == optStatus)
	  {
		  optStatus = f_read(&fileObj, readBuf, sizeof(readBuf), &optNumb);
//		  optStatus = f_write(&fileObj, writeBuf2, sizeof(writeBuf2), &optNumb);
//		  f_close(&fileObj);
//
//		  optStatus = f_open(&fileObj, "stm32.txt", FA_OPEN_EXISTING | FA_READ);
//		  optStatus = f_read(&fileObj, readBuf, sizeof(readBuf), &optNumb);
		  f_close(&fileObj);
	  }


  }
  FATFS_UnLinkDriver(USER_Path);

  /* USER CODE END Init */
}

/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
