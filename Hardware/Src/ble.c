#include "ble.h"
#include "osConfig.h"
#include "TFTLCDProcess.h"
#include "public.h"

#include "tftlcd.h"

/******************************************************************************/
BLE_RecvBufferTypedef BLE_RecvBuffer;
uint8_t BLE_RecvData[BLE_UART_RX_DATA_SIZE_MAX];

/******************************************************************************/
static void BLE_SendCmd(char* str);

/*******************************************************************************
 *
 */
void BLE_Init(void)
{
	UART_DMAIdleConfig(&BLE_UART, BLE_RecvData, BLE_UART_RX_DATA_SIZE_MAX);
}

/*******************************************************************************
 *
 */
void BLE_UartIdleDeal(void)
{
	uint32_t tmp_flag = 0, tmp_it_source = 0;

	tmp_flag = __HAL_UART_GET_FLAG(&BLE_UART, UART_FLAG_IDLE);
	tmp_it_source = __HAL_UART_GET_IT_SOURCE(&BLE_UART, UART_IT_IDLE);
	if((tmp_flag != RESET) && (tmp_it_source != RESET))
	{
		__HAL_DMA_DISABLE(BLE_UART.hdmarx);
		__HAL_DMA_CLEAR_FLAG(BLE_UART.hdmarx, BLE_UART_DMA_RX_FLAG);

		/* Clear Uart IDLE Flag */
		__HAL_UART_CLEAR_IDLEFLAG(&BLE_UART);

		BLE_RecvBuffer.bufferSize = BLE_UART_RX_DATA_SIZE_MAX
						- __HAL_DMA_GET_COUNTER(BLE_UART.hdmarx);

		memcpy(BLE_RecvBuffer.recvBuffer, BLE_RecvData, BLE_RecvBuffer.bufferSize);
		memset(BLE_RecvData, 0, BLE_RecvBuffer.bufferSize);

		osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_BLE_RECV_ENABLE);

		BLE_UART.hdmarx->Instance->CNDTR = BLE_UART.RxXferSize;
		__HAL_DMA_ENABLE(BLE_UART.hdmarx);
	}
}

/*******************************************************************************
 * @brief 连接蓝牙打印机
 * 		  蓝牙连接步骤：
 * 		  发送AT指令，查看设备是否正常，并且同步波特率
 * 		  发送SCAN搜索周边的蓝牙，
 * 		  当搜索到路格蓝牙打印机，则记录下搜索编号
 * 		  等待搜索结束
 * 		  根据编号连接蓝牙
 * 		  连接完成即可开始打印
 */
PrintModeEnum BLE_LinkPrint(void)
{
	volatile BLE_ModeEnum BLE_Mode = BLE_MODE_TEST;
	char* expectString;							/* 预期收到的字符串 */
	osEvent signal;
	char BLE_CMD_SPP_CONNECTED[14] = "AT+SPPCONN= \r\n";
	uint8_t timeOutCnt = 0;						/* 超时计数 */
	char* bleIndex;

	osThreadSetPriority(tftlcdTaskHandle, osPriorityRealtime);

	while (1)
	{
		switch (BLE_Mode)
		{
		/* 发送测试指令 */
		case BLE_MODE_TEST:
			BLE_SendCmd(BLE_CMD_AT_TEST);
			expectString = BLE_CMD_AT_TEST_RESPOND;
			BLE_Mode = BLE_MODE_TEST_FINISH;
			break;

		/* 开启扫描 */
		case BLE_MODE_SCAN:
			BLE_SendCmd(BLE_CMD_SCAN_DEVICE);
			expectString = BLE_CMD_SCAN_DEVICE_RESPOND;
			BLE_Mode = BLE_MODE_SCAN_DEVICE;
			break;

		/* SPP连接 */
		case BLE_MODE_SPP_CONNECTED:
			BLE_SendCmd(BLE_CMD_SPP_CONNECTED);
			expectString = BLE_CMD_SPP_CONNECTED_RESPOND;
			BLE_Mode = BLE_MODE_SPP_CONNECTED_FINISH;
			break;

		default:
			break;
		}

		signal = osSignalWait(TFTLCD_TASK_BLE_RECV_ENABLE, 1000);
		if (signal.status == osEventTimeout)
		{
			timeOutCnt++;
		}
		else if ((signal.value.signals & TFTLCD_TASK_BLE_RECV_ENABLE)
						== TFTLCD_TASK_BLE_RECV_ENABLE)
		{
			timeOutCnt = 0;

			if (NULL != strstr((char*)BLE_RecvBuffer.recvBuffer, expectString))
			{
				switch (BLE_Mode)
				{
				case BLE_MODE_TEST_FINISH:
					TFTLCD_TextValueUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TEXT_BLE_STATUS,
							"正在搜索打印机", 14);
					BLE_Mode = BLE_MODE_SCAN;
					break;

				case BLE_MODE_SCAN_DEVICE:
					TFTLCD_TextValueUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TEXT_BLE_STATUS,
							"找到打印机", 10);
					bleIndex = strstr((char*)BLE_RecvBuffer.recvBuffer, "+SCAN=");
//					BLE_CMD_SPP_CONNECTED[BLE_SCAN_CONNECT_INDEX_OFFSET]
//						 = BLE_RecvBuffer.recvBuffer[bleIndex + 6];
					BLE_CMD_SPP_CONNECTED[BLE_SCAN_CONNECT_INDEX_OFFSET] = *(bleIndex + 6);
					BLE_Mode = BLE_MODE_SCAN_FINISH;
					expectString = BLE_CMD_SCAN_FINISH_RESPOND;
					break;

				case BLE_MODE_SCAN_FINISH:
					BLE_Mode = BLE_MODE_SPP_CONNECTED;
					break;

				case BLE_MODE_SPP_CONNECTED_FINISH:
					TFTLCD_TextValueUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TEXT_BLE_STATUS,
							"连接成功", 14);
					BLE_Mode = BLE_MODE_LINK_DEVICE;
					break;

				default:
					break;
				}

//				osDelay(1000);
			}
		}

		/* 蓝牙配置完成,或者未搜索到蓝牙，退出while循环 */
		if ((BLE_Mode == BLE_MODE_LINK_DEVICE) || (timeOutCnt > 10))
		{
			if (timeOutCnt > 10)
			{
				TFTLCD_TextValueUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TEXT_BLE_STATUS,
					"打印机连接失败", 14);
			}
			break;
		}
	}

	osThreadSetPriority(tftlcdTaskHandle, osPriorityNormal);

	if (BLE_Mode == BLE_MODE_LINK_DEVICE)
		return PRINT_MODE_BLE_LINK;
	else
		return PRINT_MODE_BLE_UNLINK;
}

/*******************************************************************************
 *
 */
static void BLE_SendCmd(char* str)
{
	HAL_UART_Transmit_DMA(&BLE_UART, (uint8_t*)str, strlen(str));
}
