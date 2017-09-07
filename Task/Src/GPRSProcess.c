#include "GPRSProcess.h"
#include "gprs.h"
#include "gps.h"
#include "rt.h"

#include "osConfig.h"
#include "MainProcess.h"

/*******************************************************************************
 *
 */
void GPRSPROCESS_Task(void)
{
	osEvent signal;
	GPRS_ModuleStatusEnum moduleStatus = MODULE_INVALID;		/* GPRS模块状态 */
	char* expectString;											/* 预期收到的字符串 */

	GPS_LocateTypedef  location;

	GPRS_TaskStatusEnum taskStatus;
	uint8_t moduleTimeoutCnt;									/* 模块超时计数 */
	uint8_t moduleErrorCnt;										/* 模块接收错误指令计数 */
	uint16_t curPatchPack;										/* 本次上传条数 */
	BOOL gprsInited;											/* gprs功能初始化标志位 */

	GPRS_Init(&GPRS_SendBuffer);

	while(1)
	{
		/* 发送部分 */
		switch (moduleStatus)
		{
		/* 如果模块无效，则先执行开机 */
		case MODULE_INVALID:
			DebugPrintf("模块开机\r\n");
			/* 开机 */
			GPRS_PWR_CTRL_ENABLE();
			expectString = AT_CMD_POWER_ON_READY_RESPOND;
			moduleStatus = MODULE_VALID;
			break;

		/* 设置波特率 */
		case SET_BAUD_RATE:
			DebugPrintf("设置波特率\r\n");
			GPRS_SendCmd(AT_CMD_SET_BAUD_RATE);
			expectString = AT_CMD_SET_BAUD_RATE_RESPOND;
			moduleStatus = SET_BAUD_RATE_FINISH;
			break;

		/* 关闭回显模式 */
		case ECHO_DISABLE:
			DebugPrintf("关闭回显模式\r\n");
			GPRS_SendCmd(AT_CMD_ECHO_DISABLE);
			expectString = AT_CMD_ECHO_DISABLE_RESPOND;
			moduleStatus = ECHO_DISABLE_FINISH;
			break;

		/* 使能GPS功能 */
		case ENABLE_GPS:
			DebugPrintf("使能GPS功能\r\n");
			GPRS_SendCmd(AT_CMD_GPS_ENABLE);
			expectString = AT_CMD_GPS_ENABLE_RESPOND;
			moduleStatus = ENABLE_GPS_FINISH;
			break;

		/* 初始状态 */
		case INIT:
			signal = osMessageGet(gprsTaskMessageQid, osWaitForever);
			taskStatus = (GPRS_TaskStatusEnum)signal.value.v;
			switch (taskStatus)
			{
			/* 开启定位 */
			case START_TASK_GPS:
				moduleStatus = GET_GPS_GNRMC;
				break;

			/* 开启GPRS发送数据 */
			case START_TASK_GPRS:
				/* 获取本次发送的条数 */
				signal = osMessageGet(infoCntMessageQId, 2000);
				curPatchPack = signal.value.v;

				/* 如果模块已经初始化完成 */
				if (gprsInited == TRUE)
					moduleStatus = GET_SIGNAL_QUALITY;
				else
					moduleStatus = CHECK_SIM_STATUS;
				break;

			/* 开启短信发送 */
			case START_TASK_GSM:
				break;

			default:
				break;
			}

			/* 获取状态后，先发一次AT命令 */
			GPRS_SendCmd(AT_CMD_CHECK_STATUS);
			expectString = AT_CMD_CHECK_STATUS_RESPOND;
			break;

		/* 获取GNRMC定位值 */
		case GET_GPS_GNRMC:
			DebugPrintf("获取GNRMC定位值\r\n");
			/* GPS功能使能比较慢，需要先延时一段时间 */
			GPRS_SendCmd(AT_CMD_GPS_GET_GNRMC);
			expectString = AT_CMD_GPS_GET_GNRMC_RESPOND;
			moduleStatus = GET_GPS_GNRMC_FINISH;
			break;

		/* 查询SIM卡状态 */
		case CHECK_SIM_STATUS:
			DebugPrintf("查询sim卡状态\r\n");
			GPRS_SendCmd(AT_CMD_CHECK_SIM_STATUS);
			expectString = AT_CMD_CHECK_SIM_STATUS_RESPOND;
			moduleStatus = CHECK_SIM_STATUS_FINISH;
			break;

		/* 查找网络状态 */
		case SEARCH_NET_STATUS:
			DebugPrintf("查找网络\r\n");
			GPRS_SendCmd(AT_CMD_SEARCH_NET_STATUS);
			expectString = AT_CMD_SEARCH_NET_STATUS_RESPOND;
			moduleStatus = SEARCH_NET_STATUS_FINISH;
			break;

		/* 获取运营商信息 */
		case GET_OPERATOR:
			DebugPrintf("获取运营商信息\r\n");
			GPRS_SendCmd(AT_CMD_GET_OPERATOR);
			expectString = AT_CMD_GET_OPERATOR_RESPOND;
			moduleStatus = GET_OPERATOR_FINISH;
			break;

		/* 设置字符集格式为GSM */
		case SET_TEXT_FORMAT_GSM:
			DebugPrintf("设置字符集格式为GSM\r\n");
			GPRS_SendCmd(AT_CMD_SET_TEXT_FORMAT_GSM);
			expectString = AT_CMD_SET_TEXT_FORMAT_GSM_RESPOND;
			moduleStatus = SET_TEXT_FORMAT_GSM_FINISH;
			break;

		/* 获取本机号码 */
		case GET_SUBSCRIBER_NUMB:
			DebugPrintf("获取本机号码\r\n");
			GPRS_SendCmd(AT_CMD_GET_SUBSCRIBER_NUMB);
			expectString = AT_CMD_GET_SUBSCRIBER_NUMB_RESPOND;
			moduleStatus = GET_SUBSCRIBER_NUMB_FINISH;
			break;

		/* 查找GPRS状态 */
		case CHECK_GPRS_STATUS:
			DebugPrintf("查找GPRS状态\r\n");
			GPRS_SendCmd(AT_CMD_CHECK_GPRS_STATUS);
			expectString = AT_CMD_CHECK_GPRS_STATUS_RESPOND;
			moduleStatus = CHECK_GPRS_STATUS_FINISH;
			break;

		/* 设置单连接方式 */
		case SET_SINGLE_LINK:
			DebugPrintf("设置单连方式\r\n");
			GPRS_SendCmd(AT_CMD_SET_SINGLE_LINK);
			expectString = AT_CMD_SET_SINGLE_LINK_RESPOND;
			moduleStatus = SET_SINGLE_LINK_FINISH;
			break;

		/* 设置为透传模式 */
		case SET_SERIANET_MODE:
			DebugPrintf("设置透传模式\r\n");
			GPRS_SendCmd(AT_CMD_SET_SERIANET_MODE);
			expectString = AT_CMD_SET_SERIANET_MODE_RESPOND;
			moduleStatus = SET_SERIANET_MODE_FINISH;
			break;

		/* 设置APN名称 */
		case SET_APN_NAME:
			DebugPrintf("设置APN名称\r\n");
			GPRS_SendCmd(AT_CMD_SET_APN_NAME);
			expectString = AT_CMD_SET_APN_NAME_RESPOND;
			moduleStatus = SET_APN_NAME_FINISH;
			break;

		/* 激活PDP场景 */
		case ACTIVE_PDP:
			DebugPrintf("激活PDP场景\r\n");
			GPRS_SendCmd(AT_CMD_ACTIVE_PDP);
			expectString = AT_CMD_ACTIVE_PDP_RESPOND;
			moduleStatus = ACTIVE_PDP_FINISH;
			break;

		/* 获取本机IP地址 */
		case GET_SELF_IP_ADDR:
			DebugPrintf("获取本机IP地址\r\n");
			GPRS_SendCmd(AT_CMD_GET_SELF_IP_ADDR);
			expectString = AT_CMD_GET_SELF_IP_ADDR_RESPOND;
			moduleStatus = GET_SELF_IP_ADDR_FINISH;
			break;

		/* 获取信号质量 */
		case GET_SIGNAL_QUALITY:
			DebugPrintf("获取信号质量\r\n");
			GPRS_SendCmd(AT_CMD_GET_SIGNAL_QUALITY);
			expectString = AT_CMD_GET_SIGNAL_QUALITY_RESPOND;
			moduleStatus = GET_SIGNAL_QUALITY_FINISH;
			break;

		/* 设置服务器地址 */
		case SET_SERVER_IP_ADDR:
			DebugPrintf("获取服务器地址\r\n");
			GPRS_SendCmd(AT_CMD_SET_SERVER_IP_ADDR);
			expectString = AT_CMD_SET_SERVER_IP_ADDR_RESPOND;
			moduleStatus = SET_SERVER_IP_ADDR_FINISH;
			break;

		/* 模块准备好了 */
		case READY:
			DebugPrintf("模块准备好了，发送数据\r\n");
			/* 发送数据到平台 */
			GPRS_SendProtocol(&GPRS_SendBuffer, curPatchPack);
			expectString = AT_CMD_DATA_SEND_SUCCESS_RESPOND;
			moduleStatus = DATA_SEND_FINISH;
			break;

		/* 退出透传模式 */
		case EXTI_SERIANET_MODE:
			DebugPrintf("退出透传模式\r\n");
			GPRS_SendCmd(AT_CMD_EXIT_SERIANET_MODE);
			expectString = AT_CMD_EXIT_SERIANET_MODE_RESPOND;
			moduleStatus = EXTI_SERIANET_MODE_FINISH;
			break;

		/* 退出连接模式 */
		case EXTI_LINK_MODE:
			DebugPrintf("退出连接模式\r\n");
			GPRS_SendCmd(AT_CMD_EXIT_LINK_MODE);
			expectString = AT_CMD_EXIT_LINK_MODE_RESPOND;
			moduleStatus = EXTI_LINK_MODE_FINISH;
			break;

		/* 关闭移动场景 */
		case SHUT_MODULE:
			DebugPrintf("关闭移动场景\r\n");
			GPRS_SendCmd(AT_CMD_SHUT_MODELU);
			expectString = AT_CMD_SHUT_MODELU_RESPOND;
			moduleStatus = SHUT_MODULE_FINISH;
			break;

		default:
			break;
		}

		signal = osSignalWait(GPRS_PROCESS_TASK_RECV_ENABLE, 3000);
		/* 发送超时 */
		if (signal.status == osEventTimeout)
		{
			DebugPrintf("GMS模块指令接收等待超时\r\n");
			/* 模块超时计数,如果超过2次，放弃本次发送，挂起任务 */
			moduleTimeoutCnt++;
			switch (moduleStatus)
			{
			/* 可能因为看门狗等因素，导致单片机重启，也需要重启模块 */
			case MODULE_VALID:
				if (moduleTimeoutCnt > 1)
				{
					moduleTimeoutCnt = 0;
					/* 断开电源控制脚，重新开启模块 */
					GPRS_PWR_CTRL_DISABLE();
					osDelay(50);
					moduleStatus = MODULE_INVALID;
				}
				break;

			/* 发送到平台的数据没有收到答复,放弃本次数据发送，将模式切换到退出透传模式 */
			case DATA_SEND_FINISH:
				if (moduleTimeoutCnt > 2)
				{
					moduleTimeoutCnt = 0;
					moduleStatus = EXTI_SERIANET_MODE;
					osMessageGet(realtimeMessageQId, 1);
					DebugPrintf("未接收到平台正确回文\r\n");
				}

				break;

			/* GPS启动过程比较慢，暂时忽略超时等待 */
			case ENABLE_GPS_FINISH:
				if (moduleTimeoutCnt > 3)
				{
					moduleTimeoutCnt = 0;
					moduleStatus = INIT;
				}
				break;

			case GET_GPS_GNRMC_FINISH:
				if (moduleTimeoutCnt > 3)
				{
					moduleTimeoutCnt = 0;
					moduleStatus = INIT;
				}
				break;

			default:
				/* 其他情况则将状态向前移动一步 */
				moduleStatus--;

				if (moduleTimeoutCnt > 2)
				{
					moduleTimeoutCnt = 0;
					moduleStatus = INIT;
					DebugPrintf("模块指令接收超时3次,放弃本次发送\r\n");
				}
				break;
			}
		}
		else if ((signal.value.signals & GPRS_PROCESS_TASK_RECV_ENABLE)
				== GPRS_PROCESS_TASK_RECV_ENABLE)
		{
			/* 接收到任意数据，则将超时计数清空 */
			moduleTimeoutCnt = 0;

			/* 寻找预期接收的字符串是否在接收的数据中 */
			if (NULL != strstr((char*)GPRS_RecvBuffer.recvBuffer, expectString))
			{
				/* 正确接收，则接收错误清空 */
				moduleErrorCnt = 0;

				switch (moduleStatus)
				{
				/* 模块可用 */
				case MODULE_VALID:
					DebugPrintf("模块可用\r\n");
					/* 开机完成，断开power控制引脚 */
					GPRS_PWR_CTRL_DISABLE();

					/* 模块开机适当延时 */
					osDelay(5000);
					moduleStatus = SET_BAUD_RATE;
					break;

				/* 设置波特率完成 */
				case SET_BAUD_RATE_FINISH:
					DebugPrintf("设置波特率完成\r\n");
					moduleStatus = ECHO_DISABLE;
					break;

				/* 关闭回显模式完成 */
				case ECHO_DISABLE_FINISH:
					DebugPrintf("关闭回显模式完成\r\n");
					moduleStatus = ENABLE_GPS;
					break;

				/* 使能GPS功能完成 */
				case ENABLE_GPS_FINISH:
					DebugPrintf("使能GPS功能完成\r\n");
					/* 使能GPS功能后，开机已经完成，回到Init模式 */
					moduleStatus = INIT;
					/* 开启GPS后，5s再获取定位数据 */
					osDelay(5000);
					break;

				/* 获取GNRMC定位值完成 */
				case GET_GPS_GNRMC_FINISH:
					DebugPrintf("获取GNRMC定位值完成\r\n");
					/* 转换定位数据 */
					GPS_GetLocation(GPRS_RecvBuffer.recvBuffer, &location);
					printf("定位数据是%50s\r\n",GPRS_RecvBuffer.recvBuffer);
					/* 传递定位信息 */
					osMessagePut(infoMessageQId, (uint32_t)&location, 100);
					osSignalSet(mainprocessTaskHandle, MAINPROCESS_GPS_CONVERT_FINISH);
					/* GPS定位成功，回到init状态 */
					moduleStatus = INIT;
					break;

				/* 检测sim卡状态完成 */
				case CHECK_SIM_STATUS_FINISH:
					DebugPrintf("检测sim卡状态完成\r\n");
					moduleStatus = SEARCH_NET_STATUS;
					break;

				/* 查找网络状态完成 */
				case SEARCH_NET_STATUS_FINISH:
					DebugPrintf("查找网络状态完成\r\n");
					moduleStatus = GET_OPERATOR;
					break;

				/* 获取运营商信息完成 */
				case GET_OPERATOR_FINISH:
					DebugPrintf("获取运营商信息完成\r\n");
//					moduleStatus = SET_TEXT_FORMAT_GSM;
					moduleStatus = CHECK_GPRS_STATUS;
					break;

				/* 设置字符集格式为GSM完成 */
				case SET_TEXT_FORMAT_GSM_FINISH:
					DebugPrintf("设置字符集格式为GSM完成\r\n");
					moduleStatus = GET_SUBSCRIBER_NUMB;
					break;

				/* 获取本机号码完成 */
				case GET_SUBSCRIBER_NUMB_FINISH:
					DebugPrintf("获取本机号码完成\r\n");
					moduleStatus = CHECK_GPRS_STATUS;
					break;

				/* 查找GPRS状态完成 */
				case CHECK_GPRS_STATUS_FINISH:
					DebugPrintf("查找GPRS状态完成\r\n");
					moduleStatus = SET_SINGLE_LINK;
					break;

				/* 设置单连方式完成 */
				case SET_SINGLE_LINK_FINISH:
					DebugPrintf("设置单连方式完成\r\n");
					moduleStatus = SET_SERIANET_MODE;
					break;

				/* 设置透传模式完成 */
				case SET_SERIANET_MODE_FINISH:
					DebugPrintf("设置透传模式完成\r\n");
					moduleStatus = SET_APN_NAME;
					break;

				/* 设置APN名称完成 */
				case SET_APN_NAME_FINISH:
					DebugPrintf("设置APN名称完成\r\n");
					moduleStatus = ACTIVE_PDP;
					break;

				/* 激活PDP场景完成 */
				case ACTIVE_PDP_FINISH:
					DebugPrintf("激活PDP场景完成\r\n");
					moduleStatus = GET_SELF_IP_ADDR;
					break;

				/* 获取本机IP地址完成 */
				case GET_SELF_IP_ADDR_FINISH:
					DebugPrintf("获取本机IP地址完成\r\n");
					moduleStatus = GET_SIGNAL_QUALITY;
					/* 标记GPRS功能初始化完成 */
					gprsInited = TRUE;
					break;

				/* 获取信号质量完成 */
				case GET_SIGNAL_QUALITY_FINISH:
					DebugPrintf("获取信号质量完成\r\n");
					GPRS_signalQuality = GPRS_GetSignalQuality(GPRS_RecvBuffer.recvBuffer);
					printf("信号强度=%d\r\n", GPRS_signalQuality);
					moduleStatus = SET_SERVER_IP_ADDR;
					break;

				/* 设置服务器地址完成 */
				case SET_SERVER_IP_ADDR_FINISH:
					DebugPrintf("设置服务器地址完成\r\n");
					moduleStatus = READY;
					break;

				/* 数据发送完成 */
				case DATA_SEND_FINISH:
					DebugPrintf("数据发送成功\r\n");
					printf("服务器返回数据是%50s\r\n",GPRS_RecvBuffer.recvBuffer);

					/* 将本地时间与云时间对比，时间校准 */
					RT_TimeAdjustWithCloud(GPRS_RecvBuffer.recvBuffer);

					moduleStatus = EXTI_SERIANET_MODE;

					/* GPRS发送完成 */
					osSignalSet(mainprocessTaskHandle, MAINPROCESS_GPRS_SEND_FINISHED);
					break;

				/* 退出透传模式完成 */
				case EXTI_SERIANET_MODE_FINISH:
					DebugPrintf("退出透传模式完成\r\n");
					moduleStatus = EXTI_LINK_MODE;
					break;

				/* 退出单连模式完成 */
				case EXTI_LINK_MODE_FINISH:
					DebugPrintf("退出单连模式完成\r\n");
					moduleStatus = SHUT_MODULE;
					break;

				/* 关闭移动场景完成 */
				case SHUT_MODULE_FINISH:
					DebugPrintf("关闭移动场景完成\r\n");
					/* 模块发送完成，把状态设置成使能GPS定位，下次启动直接连接服务器地址即可发送 */
					moduleStatus = INIT;
					break;

				default:
					break;
				}
			}
			/* 模块返回的指令不正确 */
			else
			{
				/* 错误计数 */
				moduleErrorCnt++;
				if (moduleErrorCnt >= 10)
				{
					DebugPrintf("模块接收到错误指令超过10次\r\n");
					moduleErrorCnt = 0;
					switch (moduleStatus)
					{
					case MODULE_VALID:
						break;

					/* 链接服务器地址出现“FAIL”或者“ERROR”，不能链接上服务器 */
					case SET_SERVER_IP_ADDR_FINISH:
						if (NULL != strstr((char*)GPRS_RecvBuffer.recvBuffer, "FAIL ERROR"))
						{
							/* 放弃本次发送 */
							moduleStatus = INIT;

							DebugPrintf("不能链接上服务器，放弃本次发送\r\n");
						}
						break;

					case DATA_SEND_FINISH:
						break;

					case ENABLE_GPS:
						break;

					case GET_GPS_GNRMC:
						moduleStatus = INIT;
						break;

					default:
						moduleStatus = INIT;
						gprsInited = FALSE;
						DebugPrintf("模块配置错误，等待下次重新配置\r\n");
						break;
					}
				}
			}
			memset(GPRS_RecvBuffer.recvBuffer, 0, GPRS_RecvBuffer.bufferSize);
		}
	}
}

