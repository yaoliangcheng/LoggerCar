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
FATFS fs;													/* FatFs文件系统对象 */
FIL fnew;													/* 文件对象 */
FRESULT res_flash;                /* 文件操作结果 */
UINT fnum;            					  /* 文件成功读写数量 */
BYTE ReadBuffer[100]={0};        /* 读缓冲区 */
BYTE WriteBuffer[] =              /* 写缓冲区*/
"This is a stm32 fatfs test\r\n";

/* USER CODE END Variables */    

void MX_FATFS_Init(void) 
{
  /*## FatFS: Link the USER driver ###########################*/
  retUSER = FATFS_LinkDriver(&USER_Driver, USER_Path);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
  if (0 == retUSER)
  {
	  /* 挂载spi flash */
	  res_flash = f_mount(&fs, USER_Path, 1);

	  /* 如果没有文件系统就格式化创建创建文件系统 */
	  if(res_flash == FR_NO_FILESYSTEM)
	  {
		  /* 格式化 */
		  res_flash=f_mkfs(USER_Path, 0, 0);
		  if(res_flash == FR_OK)
		  {
			  /* 格式化后，先取消挂载 */
			  res_flash = f_mount(NULL, USER_Path, 1);
			  /* 重新挂载	*/
			  res_flash = f_mount(&fs, USER_Path, 1);
		  }
	  }

	  /*----------------------- 文件系统测试：写测试 -------------------*/
	  /* 打开文件，每次都以新建的形式打开，属性为可写 */
	  res_flash = f_open(&fnew, "FatFs.txt",FA_CREATE_ALWAYS | FA_WRITE );
	  if ( res_flash == FR_OK )
	  {
		  /* 将指定存储区内容写入到文件内 */
		  res_flash=f_write(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);

		  /* 不再读写，关闭文件 */
		  f_close(&fnew);
	  }
  }

  /*------------------- 文件系统测试：读测试 --------------------------*/
  res_flash = f_open(&fnew, "FatFs.txt",FA_OPEN_EXISTING | FA_READ);
  if (res_flash == FR_OK)
  {
	  res_flash = f_read(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum);

	  /* 不再读写，关闭文件 */
	  f_close(&fnew);
  }

  /* 不再使用文件系统，取消挂载文件系统 */
  f_mount(NULL,USER_Path,1);

  /* USER CODE END Init */
}

/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
