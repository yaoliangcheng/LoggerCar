#include "ble.h"
#include "osConfig.h"
#include "TFTLCDProcess.h"
#include "public.h"

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
 * @brief ����������ӡ��
 * 		  �������Ӳ��裺
 * 		  ����ATָ��鿴�豸�Ƿ�����������ͬ��������
 * 		  ����SCAN�����ܱߵ�������
 * 		  ��������·��������ӡ�������¼���������
 * 		  �ȴ���������
 * 		  ���ݱ����������
 * 		  ������ɼ��ɿ�ʼ��ӡ
 */
ErrorStatus BLE_LinkPrint(void)
{
	BLE_ModeEnum BLE_Mode = BLE_MODE_TEST;
	char* expectString;							/* Ԥ���յ����ַ��� */
	osEvent signal;
	char BLE_CMD_SPP_CONNECTED[14] = "AT+SPPCONN= \r\n";
	uint8_t timeOutCnt = 0;						/* ��ʱ���� */

	while (1)
	{
		switch (BLE_Mode)
		{
		case BLE_MODE_TEST:
			BLE_SendCmd(BLE_CMD_AT_TEST);
			expectString = BLE_CMD_AT_TEST_RESPOND;
			BLE_Mode = BLE_MODE_TEST_FINISH;
			break;

		case BLE_MODE_SCAN:
			BLE_SendCmd(BLE_CMD_SCAN_DEVICE);
			expectString = BLE_CMD_SCAN_DEVICE_RESPOND;
			BLE_Mode = BLE_MODE_SCAN_DEVICE;
			break;

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
					BLE_Mode = BLE_MODE_SCAN;
					break;

				case BLE_MODE_SCAN_DEVICE:
					BLE_CMD_SPP_CONNECTED[BLE_SCAN_CONNECT_INDEX_OFFSET]
						 = BLE_RecvBuffer.recvBuffer[BLE_SCAN_DEVICE_INDEX_OFFSET];
					BLE_Mode = BLE_MODE_SCAN_FINISH;
					expectString = BLE_CMD_SCAN_FINISH_RESPOND;
					break;

				case BLE_MODE_SCAN_FINISH:
					BLE_Mode = BLE_MODE_SPP_CONNECTED;
					break;

				case BLE_MODE_SPP_CONNECTED_FINISH:
					BLE_Mode = BLE_MODE_LINK_DEVICE;
					break;

				default:
					break;
				}
			}
		}

		/* �����������,����δ�������������˳�whileѭ�� */
		if ((BLE_Mode == BLE_MODE_LINK_DEVICE) || (timeOutCnt > 10))
		{
			break;
		}
	}
	if (BLE_Mode == BLE_MODE_LINK_DEVICE)
		return SUCCESS;
	else
		return ERROR;
}

/*******************************************************************************
 *
 */
static void BLE_SendCmd(char* str)
{
	HAL_UART_Transmit_DMA(&BLE_UART, (uint8_t*)str, strlen(str));
}