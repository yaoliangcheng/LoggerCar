/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include "led.h"
#include "debug.h"
#include "RealTime.h"
#include "MainProcess.h"
#include "GPRSProcess.h"
#include "TFTLCDProcess.h"

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;

/* USER CODE BEGIN Variables */

/* 任务句柄 */
osThreadId ledTaskHandle;
osThreadId debugTaskHandle;
osThreadId realtimeTaskHandle;
osThreadId tftlcdTaskHandle;
osThreadId mainprocessTaskHandle;
osThreadId gprsprocessTaskHandle;

/* 队列句柄 */
osMessageQId realtimeMessageQId;
osMessageQId adjustTimeMessageQId;
osMessageQId analogMessageQId;
osMessageQId infoMessageQId;
osMessageQId infoCntMessageQId;

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);

extern void MX_FATFS_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  osThreadDef(LED, LED_Task, osPriorityNormal, 0, 128);
  ledTaskHandle = osThreadCreate(osThread(LED), NULL);
  
  osThreadDef(DEBUG, DEBUG_Task, osPriorityNormal, 0, 128);
  debugTaskHandle = osThreadCreate(osThread(DEBUG), NULL);

  osThreadDef(REALTIME, REALTIME_Task, osPriorityNormal, 0, 512);
  realtimeTaskHandle = osThreadCreate(osThread(REALTIME), NULL);
  /* 任务创建成功后再开启RTC的秒中断，否则会出错 */
  HAL_RTCEx_SetSecond_IT(&hrtc);

  osThreadDef(TFTLCD, TFTLCD_Task, osPriorityNormal, 0, 128);
  tftlcdTaskHandle = osThreadCreate(osThread(TFTLCD), NULL);

  osThreadDef(MAINPROCESS, MAINPROCESS_Task, osPriorityAboveNormal, 0, 2048);
  mainprocessTaskHandle = osThreadCreate(osThread(MAINPROCESS), NULL);
  osThreadSuspend(mainprocessTaskHandle);

  osThreadDef(GPRSPROCESS, GPRSPROCESS_Task, osPriorityRealtime, 0, 512);
  gprsprocessTaskHandle = osThreadCreate(osThread(GPRSPROCESS), NULL);
  osThreadSuspend(gprsprocessTaskHandle);


  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  osMessageQDef(REALTIME_MESSAGE, 2, sizeof(uint32_t*));
  realtimeMessageQId = osMessageCreate(osMessageQ(REALTIME_MESSAGE), NULL);

  osMessageQDef(ADJUSTTIME_MESSAGE, 2, sizeof(uint32_t*));
  adjustTimeMessageQId = osMessageCreate(osMessageQ(ADJUSTTIME_MESSAGE), NULL);

  osMessageQDef(ANALOG_MESSAGE, 2, sizeof(uint32_t*));
  analogMessageQId = osMessageCreate(osMessageQ(ANALOG_MESSAGE), NULL);

  osMessageQDef(INFO_MESSAGE, 2, sizeof(uint32_t*));
  infoMessageQId = osMessageCreate(osMessageQ(INFO_MESSAGE), NULL);

  /* 数据条数传递的是值本身 */
  osMessageQDef(INFO_CNT_MESSAGE, 2, sizeof(uint16_t));
  infoCntMessageQId = osMessageCreate(osMessageQ(INFO_CNT_MESSAGE), NULL);


  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{
  /* init code for FATFS */
  MX_FATFS_Init();

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
