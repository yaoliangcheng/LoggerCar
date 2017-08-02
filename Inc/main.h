/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define GPRS_PWRKEY_CTRL_Pin GPIO_PIN_2
#define GPRS_PWRKEY_CTRL_GPIO_Port GPIOE
#define GPRS_RST_CTRL_Pin GPIO_PIN_3
#define GPRS_RST_CTRL_GPIO_Port GPIOE
#define VGPRS_CTRL_Pin GPIO_PIN_4
#define VGPRS_CTRL_GPIO_Port GPIOE
#define PWR_ON_CHECK_Pin GPIO_PIN_5
#define PWR_ON_CHECK_GPIO_Port GPIOE
#define VBAT_CTRL_Pin GPIO_PIN_6
#define VBAT_CTRL_GPIO_Port GPIOE
#define TEMP1_Pin GPIO_PIN_0
#define TEMP1_GPIO_Port GPIOC
#define TEMP2_Pin GPIO_PIN_1
#define TEMP2_GPIO_Port GPIOC
#define TEMP3_Pin GPIO_PIN_2
#define TEMP3_GPIO_Port GPIOC
#define TEMP4_Pin GPIO_PIN_3
#define TEMP4_GPIO_Port GPIOC
#define TEMP5_Pin GPIO_PIN_1
#define TEMP5_GPIO_Port GPIOA
#define TEMP6_Pin GPIO_PIN_2
#define TEMP6_GPIO_Port GPIOA
#define TEMP7_Pin GPIO_PIN_3
#define TEMP7_GPIO_Port GPIOA
#define HUMI1_Pin GPIO_PIN_4
#define HUMI1_GPIO_Port GPIOA
#define HUMI2_Pin GPIO_PIN_5
#define HUMI2_GPIO_Port GPIOA
#define HUMI3_Pin GPIO_PIN_6
#define HUMI3_GPIO_Port GPIOA
#define HUMI4_Pin GPIO_PIN_7
#define HUMI4_GPIO_Port GPIOA
#define HUMI5_Pin GPIO_PIN_4
#define HUMI5_GPIO_Port GPIOC
#define HUMI6_Pin GPIO_PIN_5
#define HUMI6_GPIO_Port GPIOC
#define HUMI7_Pin GPIO_PIN_0
#define HUMI7_GPIO_Port GPIOB
#define AN_VBAT_Pin GPIO_PIN_1
#define AN_VBAT_GPIO_Port GPIOB
#define SENSOR_PWR_CTRL_Pin GPIO_PIN_2
#define SENSOR_PWR_CTRL_GPIO_Port GPIOB
#define EE_WP_Pin GPIO_PIN_15
#define EE_WP_GPIO_Port GPIOE
#define EE_SCL_Pin GPIO_PIN_10
#define EE_SCL_GPIO_Port GPIOB
#define EE_SDA_Pin GPIO_PIN_11
#define EE_SDA_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_12
#define LED_GPIO_Port GPIOB
#define GPRS_RX_Pin GPIO_PIN_5
#define GPRS_RX_GPIO_Port GPIOD
#define GPRS_TX_Pin GPIO_PIN_6
#define GPRS_TX_GPIO_Port GPIOD
#define SPI_CS_FLASH_Pin GPIO_PIN_7
#define SPI_CS_FLASH_GPIO_Port GPIOD
#define DEBUG_RX_Pin GPIO_PIN_6
#define DEBUG_RX_GPIO_Port GPIOB
#define DEBUG_TX_Pin GPIO_PIN_7
#define DEBUG_TX_GPIO_Port GPIOB
#define SCREEN_PWR_CTRL_Pin GPIO_PIN_0
#define SCREEN_PWR_CTRL_GPIO_Port GPIOE
#define PRINT_PWR_CTRL_Pin GPIO_PIN_1
#define PRINT_PWR_CTRL_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
