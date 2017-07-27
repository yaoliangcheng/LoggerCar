#include "GPRSProcess.h"
#include "gprs.h"
#include "gps.h"

#include "osConfig.h"
#include "MainProcess.h"

/*******************************************************************************
 *
 */
void GPRSPROCESS_Task(void)
{
	osEvent signal;
	GPRS_ModuleStatusEnum moduleStatus = SET_BAUD_RATE;		/* GPRS模块状态 */
	char* expectString;						/* 预期收到的字符串 */
	char* str;
	GPRS_StructTypedef sendStruct;			/* 发送结构 */
	GPS_LocationTypedef location;
	BOOL isLoop = FALSE;

	GPRS_Init();
	/* 初始化发送结构体 */
	GPRS_StructInit(&sendStruct);

	while(1)
	{
		/* 获取模拟量信息 */
		signal = osMessageGet(infoMessageQId, 100);
		memcpy(&sendStruct.dataPack, (uint32_t*)signal.value.v, sizeof(exFLASH_InfoTypedef));
		isLoop = TRUE;

		while(isLoop)
		{
			/* 发送部分 */
			switch (moduleStatus)
			{
			/* 如果模块无效，则先执行开机 */
			case MODULE_INVALID:
				printf("模块开机\r\n");
				/* 开机 */
				GPRS_PWR_CTRL_ENABLE();
				osDelay(5000);
				GPRS_PWR_CTRL_DISABLE();
				expectString = AT_CMD_POWER_ON_READY_RESPOND;
				moduleStatus = MODULE_VALID;
				break;

			/* 设置波特率 */
			case SET_BAUD_RATE:
				printf("设置波特率\r\n");
				GPRS_SendCmd(AT_CMD_SET_BAUD_RATE);
				expectString = AT_CMD_SET_BAUD_RATE_RESPOND;
				moduleStatus = SET_BAUD_RATE_FINISH;
				break;

			/* 查询SIM卡状态 */
			case CHECK_SIM_STATUS:
				printf("查询sim卡状态\r\n");
				GPRS_SendCmd(AT_CMD_CHECK_SIM_STATUS);
				expectString = AT_CMD_CHECK_SIM_STATUS_RESPOND;
				moduleStatus = CHECK_SIM_STATUS_FINISH;
				break;

			/* 查找网络状态 */
			case SEARCH_NET_STATUS:
				printf("查找网络\r\n");
				GPRS_SendCmd(AT_CMD_SEARCH_NET_STATUS);
				expectString = AT_CMD_SEARCH_NET_STATUS_RESPOND;
				moduleStatus = SEARCH_NET_STATUS_FINISH;
				break;

			/* 查找GPRS状态 */
			case CHECK_GPRS_STATUS:
				printf("查找GPRS状态\r\n");
				GPRS_SendCmd(AT_CMD_CHECK_GPRS_STATUS);
				expectString = AT_CMD_CHECK_GPRS_STATUS_RESPOND;
				moduleStatus = CHECK_GPRS_STATUS_FINISH;
				break;

			/* 设置单连接方式 */
			case SET_SINGLE_LINK:
				printf("设置单连方式\r\n");
				GPRS_SendCmd(AT_CMD_SET_SINGLE_LINK);
				expectString = AT_CMD_SET_SINGLE_LINK_RESPOND;
				moduleStatus = SET_SINGLE_LINK_FINISH;
				break;

			/* 设置为透传模式 */
			case SET_SERIANET_MODE:
				printf("设置透传模式\r\n");
				GPRS_SendCmd(AT_CMD_SET_SERIANET_MODE);
				expectString = AT_CMD_SET_SERIANET_MODE_RESPOND;
				moduleStatus = SET_SERIANET_MODE_FINISH;
				break;

			/* 设置APN名称 */
			case SET_APN_NAME:
				printf("设置APN名称\r\n");
				GPRS_SendCmd(AT_CMD_SET_APN_NAME);
				expectString = AT_CMD_SET_APN_NAME_RESPOND;
				moduleStatus = SET_APN_NAME_FINISH;
				break;

			/* 激活PDP场景 */
			case ACTIVE_PDP:
				printf("激活PDP场景\r\n");
				GPRS_SendCmd(AT_CMD_ACTIVE_PDP);
				expectString = AT_CMD_ACTIVE_PDP_RESPOND;
				moduleStatus = ACTIVE_PDP_FINISH;
				break;

			/* 获取本机IP地址 */
			case GET_SELF_IP_ADDR:
				printf("获取本机IP地址\r\n");
				GPRS_SendCmd(AT_CMD_GET_SELF_IP_ADDR);
				expectString = AT_CMD_GET_SELF_IP_ADDR_RESPOND;
				moduleStatus = GET_SELF_IP_ADDR_FINISH;
				break;

			/* 使能GPS功能 */
			case ENABLE_GPS:
				printf("使能GPS功能\r\n");
				GPRS_SendCmd(AT_CMD_GPS_ENABLE);
				expectString = AT_CMD_GPS_ENABLE_RESPOND;
				moduleStatus = ENABLE_GPS_FINISH;
				break;

			/* 获取GNRMC定位值 */
			case GET_GPS_GNRMC:
				printf("获取GNRMC定位值\r\n");
				/* GPS功能使能比较慢，需要先延时一段时间 */
				GPRS_SendCmd(AT_CMD_GPS_GET_GNRMC);
				expectString = AT_CMD_GPS_GET_GNRMC_RESPOND;
				moduleStatus = GET_GPS_GNRMC_FINISH;
				break;

			/* 失能GPS功能 */
			case DISABLE_GPS:
				printf("失能GPS功能\r\n");
				GPRS_SendCmd(AT_CMD_GPS_DISABLE);
				expectString = AT_CMD_GPS_DISABLE_RESPOND;
				moduleStatus = DISABLE_GPS_FINISH;
				break;

			/* 设置服务器地址 */
			case SET_SERVER_IP_ADDR:
				printf("获取服务器地址\r\n");
				GPRS_SendCmd(AT_CMD_SET_SERVER_IP_ADDR);
				expectString = AT_CMD_SET_SERVER_IP_ADDR_RESPOND;
				moduleStatus = SET_SERVER_IP_ADDR_FINISH;
				break;

			/* 模块准备好了 */
			case READY:
				printf("模块准备好了，发送数据\r\n");
				/* 发送数据到平台 */
				GPRS_SendProtocol(&sendStruct);
				expectString = AT_CMD_DATA_SEND_SUCCESS_RESPOND;
				moduleStatus = DATA_SEND_FINISH;
				break;

			/* 退出透传模式 */
			case EXTI_SERIANET_MODE:
				printf("退出透传模式\r\n");
				GPRS_SendCmd(AT_CMD_EXIT_SERIANET_MODE);
				expectString = AT_CMD_EXIT_SERIANET_MODE_RESPOND;
				moduleStatus = EXTI_SERIANET_MODE_FINISH;
				break;

			/* 退出连接模式 */
			case EXTI_LINK_MODE:
				printf("退出连接模式\r\n");
				GPRS_SendCmd(AT_CMD_EXIT_LINK_MODE);
				expectString = AT_CMD_EXIT_LINK_MODE_RESPOND;
				moduleStatus = EXTI_LINK_MODE_FINISH;
				break;

			/* 关闭移动场景 */
			case SHUT_MODULE:
				printf("关闭移动场景\r\n");
				GPRS_SendCmd(AT_CMD_SHUT_MODELU);
				expectString = AT_CMD_SHUT_MODELU_RESPOND;
				moduleStatus = SHUT_MODULE_FINISH;
				break;

			default:
				break;
			}

			signal = osSignalWait(GPRS_PROCESS_TASK_RECV_ENABLE, 10000);
			/* 发送超时 */
			if (signal.status == osEventTimeout)
			{
				printf("等待接收超时\r\n");
				/* 发送到平台的数据没有收到答复 */
				if (DATA_SEND_FINISH == moduleStatus)
				{
					/* 放弃本次数据发送，将模式切换到退出透传模式 */
					moduleStatus = EXTI_SERIANET_MODE;

					/* 数据发送失败，记录，等待下次发送 */
					/* todo */
				}
				/* 其他命令没有收到预期答复，则重复发送 */
				else
				{
					/* 模式切换到前一步再次触发发送 */
					moduleStatus--;
				}
			}
			else if ((signal.value.signals & GPRS_PROCESS_TASK_RECV_ENABLE)
					== GPRS_PROCESS_TASK_RECV_ENABLE)
			{
				/* 寻找预期接收的字符串是否在接收的数据中 */
				str = strstr((char*)GPRS_BufferStatus.recvBuffer, expectString);
				if (NULL != str)
				{
					switch (moduleStatus)
					{
					/* 模块可用 */
					case MODULE_VALID:
						printf("模块可用\r\n");
						/* 开机完成，断开power控制引脚 */
						GPRS_PWR_CTRL_DISABLE();
						expectString = AT_CMD_MODULE_START_RESPOND;
						moduleStatus = MODULE_START;
						break;

						/* 模块启动 */
					case MODULE_START:
						printf("模块启动\r\n");
						/* 开始设置模块参数 */
						moduleStatus = SET_BAUD_RATE;
						break;

						/* 设置波特率完成 */
					case SET_BAUD_RATE_FINISH:
						printf("设置波特率完成\r\n");
						moduleStatus = CHECK_SIM_STATUS;
						break;

						/* 检测sim卡状态完成 */
					case CHECK_SIM_STATUS_FINISH:
						printf("检测sim卡状态完成\r\n");
						moduleStatus = SEARCH_NET_STATUS;
						break;

						/* 查找网络状态完成 */
					case SEARCH_NET_STATUS_FINISH:
						printf("查找网络状态完成\r\n");
						moduleStatus = CHECK_GPRS_STATUS;
						break;

						/* 查找GPRS状态完成 */
					case CHECK_GPRS_STATUS_FINISH:
						printf("查找GPRS状态完成\r\n");
						moduleStatus = SET_SINGLE_LINK;
						break;

						/* 设置单连方式完成 */
					case SET_SINGLE_LINK_FINISH:
						printf("设置单连方式完成\r\n");
						moduleStatus = SET_SERIANET_MODE;
						break;

						/* 设置透传模式完成 */
					case SET_SERIANET_MODE_FINISH:
						printf("设置透传模式完成\r\n");
						moduleStatus = SET_APN_NAME;
						break;

						/* 设置APN名称完成 */
					case SET_APN_NAME_FINISH:
						printf("设置APN名称完成\r\n");
						moduleStatus = ACTIVE_PDP;
						break;

						/* 激活PDP场景完成 */
					case ACTIVE_PDP_FINISH:
						printf("激活PDP场景完成\r\n");
						moduleStatus = GET_SELF_IP_ADDR;
						break;

						/* 获取本机IP地址完成 */
					case GET_SELF_IP_ADDR_FINISH:
						printf("获取本机IP地址完成\r\n");
						moduleStatus = ENABLE_GPS;
						break;

						/* 使能GPS功能完成 */
					case ENABLE_GPS_FINISH:
						printf("使能GPS功能完成\r\n");
						moduleStatus = GET_GPS_GNRMC;
						break;

						/* 获取GNRMC定位值完成 */
					case GET_GPS_GNRMC_FINISH:
						printf("获取GNRMC定位值完成\r\n");
						GPS_GetLocation(GPRS_BufferStatus.recvBuffer, &location);
						/* 把指针传递回原函数 */
						/* todo */
						moduleStatus = DISABLE_GPS;
						
						break;

						/* 失能GPS功能完成 */
					case DISABLE_GPS_FINISH:
						printf("失能GPS功能完成\r\n");
						moduleStatus = SET_SERVER_IP_ADDR;

						isLoop = FALSE;
						/* 传递定位信息 */
						osMessagePut(infoMessageQId, (uint32_t)&location, 100);
						osSignalSet(mainprocessTaskHandle, MAINPROCESS_GPS_CONVERT_FINISH);
						/* GPS定位成功，将自己挂起 */
						osThreadSuspend(NULL);
						break;

						/* 设置服务器地址完成 */
					case SET_SERVER_IP_ADDR_FINISH:
						printf("设置服务器地址完成\r\n");
						moduleStatus = READY;
						break;

						/* 数据发送完成 */
					case DATA_SEND_FINISH:
						printf("数据发送成功\r\n");
						moduleStatus = EXTI_SERIANET_MODE;
						break;

						/* 退出透传模式完成 */
					case EXTI_SERIANET_MODE_FINISH:
						printf("退出透传模式完成\r\n");
						moduleStatus = EXTI_LINK_MODE;
						break;

						/* 退出单连模式完成 */
					case EXTI_LINK_MODE_FINISH:
						printf("退出单连模式完成\r\n");
						moduleStatus = SHUT_MODULE;
						break;

						/* 关闭移动场景完成 */
					case SHUT_MODULE_FINISH:
						printf("关闭移动场景完成\r\n");
						/* 模块发送完成，把状态设置成使能GPS定位，下次启动直接连接服务器地址即可发送 */
						moduleStatus = ENABLE_GPS;

						/* GPRS发送完成 */
						osSignalSet(mainprocessTaskHandle, MAINPROCESS_GPRS_SEND_FINISHED);
						printf("数据发送完成\r\n");
						isLoop = FALSE;
						/* 将自己挂起 */
						osThreadSuspend(NULL);
						break;

					default:
						break;
					}
				}
				/* 不是接收数据，并且不是获取GPS的GNRMC */
				else if (moduleStatus != DATA_SEND_FINISH)
				{
					/* 判断接收的数据是否错误 */
					str = strstr((char*)GPRS_BufferStatus.recvBuffer, "Error");

					/* 数据错误，必须重新初始化模块 */
					if (str != NULL)
					{
						moduleStatus = SET_BAUD_RATE;
						printf("数据发送错误\r\n");
					}
				}
				memset(GPRS_BufferStatus.recvBuffer, 0, GPRS_BufferStatus.bufferSize);
				osDelay(1000);
			}
		}
	}
}



