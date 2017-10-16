#ifndef __GPRS_PROCESS_H
#define __GPRS_PROCESS_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "gprs.h"

/*******************************************************************************
 * AT指令
 */
#define AT_CMD_RESPOND_OK				  		("OK")
/* 模块初始化指令 */
#define AT_CMD_CHECK_STATUS				  		("AT\r\n")
#define AT_CMD_CHECK_STATUS_RESPOND		  		(AT_CMD_RESPOND_OK)
#define AT_CMD_POWER_ON_READY_RESPOND	  		("+CFUN: 1")
#define AT_CMD_MODULE_START_RESPOND	  	  		("SMS Ready")
#define AT_CMD_SET_BAUD_RATE			  		("AT+IPR=115200\r\n")			/* 设置固定波特率 */
#define AT_CMD_SET_BAUD_RATE_RESPOND	  		(AT_CMD_RESPOND_OK)
#define AT_CMD_CHECK_SIM_STATUS			  		("AT+CPIN?\r\n")	  			/* 查询SIM卡状态 */
#define AT_CMD_CHECK_SIM_STATUS_RESPOND	  		("+CPIN: READY")
#define AT_CMD_SEARCH_NET_STATUS		  		("AT+CREG?\r\n")	  			/* 查询找网络状态 */
#define AT_CMD_SEARCH_NET_STATUS_RESPOND	  	("+CREG: 0")
#define AT_CMD_SEARCH_NET_LOCATION_RESPOND  	("+CREG: 0,1")					/* 注册到本地网络 */
#define AT_CMD_SEARCH_NET_ROAM_RESPOND  		("+CREG: 0,5")					/* 注册到漫游网络 */
#define AT_CMD_CHECK_GPRS_STATUS		  		("AT+CGATT?\r\n")				/* 查询GPRS附着是否成功 */
#define AT_CMD_CHECK_GPRS_STATUS_RESPOND  		("+CGATT: 1")

/* 配置GPRS为透传模式 */
#define AT_CMD_SET_SINGLE_LINK			  		("AT+CIPMUX=0\r\n")	 			/* 设置为单路模式 */
#define AT_CMD_SET_SINGLE_LINK_RESPOND	  		(AT_CMD_RESPOND_OK)
#define AT_CMD_SET_SERIANET_MODE		  		("AT+CIPMODE=1\r\n")	 		/* 设置为透传模式 */
#define AT_CMD_SET_SERIANET_MODE_RESPOND  		(AT_CMD_RESPOND_OK)
#define AT_CMD_SET_APN_NAME				  		("AT+CSTT=\"UNINET\"\r\n")		/* 设置APN名称 */
#define AT_CMD_SET_APN_NAME_RESPOND       		(AT_CMD_RESPOND_OK)
#define AT_CMD_ACTIVE_PDP				  		("AT+CIICR\r\n")		 		/* 激活PDP场景 */
#define AT_CMD_ACTIVE_PDP_RESPOND		  		(AT_CMD_RESPOND_OK)
#define AT_CMD_GET_SELF_IP_ADDR			  		("AT+CIFSR\r\n")		 		/* 获取本机IP地址 */
#define AT_CMD_GET_SELF_IP_ADDR_RESPOND   		(AT_CMD_RESPOND_OK)
#define AT_CMD_GPS_ENABLE				  		("AT+EGPSC=1\r\n")				/* 使能GPS功能 */
//#define AT_CMD_GPS_ENABLE_RESPOND		  		("+MGPSSTATUS:1,1,0")
#define AT_CMD_GPS_ENABLE_RESPOND		  		(AT_CMD_RESPOND_OK)
#define AT_CMD_GPS_CHECK_STATUS			  		("AT+GETGPS=\"GNRMC\"\r\n")		/* 获取GNRMC状态 */
#define AT_CMD_GPS_CHECK_STATUS_RESPOND	  		(AT_CMD_RESPOND_OK)
#define AT_CMD_GPS_GET_GNRMC			  		("AT+GETGPS=\"GNRMC\"\r\n")		/* 获取GNRMC */
#define AT_CMD_GPS_GET_GNRMC_RESPOND	  		("$GNRMC,")
#define AT_CMD_GPS_DISABLE				  		("AT+EGPSC=0\r\n")				/* 失能GPS功能 */
#define AT_CMD_GPS_DISABLE_RESPOND		  		(AT_CMD_RESPOND_OK)
#define AT_CMD_SET_SERVER_IP_ADDR		  		("AT+CIPSTART=\"TCP\",\"112.124.106.188\",\"8080\"\r\n")/* 连接服务器 */
#define AT_CMD_SET_SERVER_IP_ADDR_RESPOND 		("CONNECT OK")
#define AT_CMD_DATA_SEND_SUCCESS_RESPOND  		("Save Data")
#define AT_CMD_DATA_SEND_ERROR_RESPOND    		("Error Data")
#define AT_CMD_EXIT_SERIANET_MODE	  	  		("+++")			 				/* 退出透传数据模式 */
#define AT_CMD_EXIT_SERIANET_MODE_RESPOND 		(AT_CMD_RESPOND_OK)
#define AT_CMD_RETURN_SERIANET		  	  		("ATO\r\n")			 			/* 返回透传模式 */
#define AT_CMD_RETURN_SERIANET_RESPOND	  		("ATO\r\n")
#define AT_CMD_EXIT_LINK_MODE			  		("AT+CIPCLOSE=0\r\n")	 		/* 关闭单路Socket连接 */
#define AT_CMD_EXIT_LINK_MODE_RESPOND	  		("CLOSE OK")
#define AT_CMD_SHUT_MODELU	 		  	  		("AT+CIPSHUT\r\n")		 		/* 关闭移动场景 */
#define AT_CMD_SHUT_MODELU_RESPOND		  		("SHUT OK")

/* 设备属性 */
#define AT_CMD_GET_SIGNAL_QUALITY		  		("AT+CSQ\r\n")					/* 获取信号质量 */
#define AT_CMD_GET_SIGNAL_QUALITY_RESPOND 		("+CSQ:")
#define AT_CMD_GET_OPERATOR				  		("AT+COPS?\r\n")				/* 获取运营商 */
#define AT_CMD_GET_OPERATOR_RESPOND		  		("+COPS:")
#define AT_CMD_SET_TEXT_FORMAT_GSM				("AT+CSCS=\"GSM\"\r\n")				/* 设置字符集为GSM */
#define AT_CMD_SET_TEXT_FORMAT_GSM_RESPOND		(AT_CMD_RESPOND_OK)
#define AT_CMD_GET_SUBSCRIBER_NUMB		  		("AT+CNUM\r\n")					/* 获取本机号码 */
#define AT_CMD_GET_SUBSCRIBER_NUMB_RESPOND 		("+CNUM:")
#define AT_CMD_ECHO_ENABLE				  		("ATE1\r\n")					/* 开启命令回显模式 */
#define AT_CMD_ECHO_ENABLE_RESPOND		  		(AT_CMD_RESPOND_OK)
#define AT_CMD_ECHO_DISABLE				  		("ATE0\r\n")					/* 关闭命令回显模式 */
#define AT_CMD_ECHO_DISABLE_RESPOND		  		(AT_CMD_RESPOND_OK)

/* 发送短信 */
#define AT_CMD_SET_MESSAGE_MODE					("AT+CMGF=1\r\n")
#define AT_CMD_SET_MESSAGE_MODE_RESPOND			(AT_CMD_RESPOND_OK)
#define AT_CMD_SET_TEXT_PARAM					("AT+CSMP=17,167,2,25\r\n")
#define AT_CMD_SET_TEXT_PARAM_RESPOND			(AT_CMD_RESPOND_OK)
#define AT_CMD_SET_MESSAGE_TEXT_FORMAT			("AT+CSCS=\"UCS2\"\r\n")
#define AT_CMD_SET_MESSAGE_TEXT_FORMAT_RESPOND	(AT_CMD_RESPOND_OK)
#define AT_CMD_SEND_MESSAGE_NUMB				("AT+CMGS=\"18367053909\"\r\n")
#define AT_CMD_SEND_MESSAGE_NUMB_RESPOND		(">")
#define AT_CMD_SEND_MESSAGE_SUCCESS_RESPOND		(AT_CMD_RESPOND_OK)

/******************************************************************************/
#define GPRSPROCESS_WAIT_FOR_PARAM_INIT			(1 << 0)
//#define GPRSPROCESS_SEND_DATA_ENABLE			(1 << 1)
#define GPRS_PROCESS_TASK_RECV_ENABLE			(1 << 2)

/******************************************************************************/
typedef enum
{
	MODULE_INVALID,									/* 模块无效 */
	MODULE_VALID,									/* 模块有效 */
	SET_BAUD_RATE,									/* 设置波特率 */
	SET_BAUD_RATE_FINISH,							/* 波特率设置完成 */
	ECHO_DISABLE,									/* 关闭回显模式 */
	ECHO_DISABLE_FINISH,							/* 关闭回显模式完成 */
	ENABLE_GPS,										/* 使能GPS功能 */
	ENABLE_GPS_FINISH,								/* 使能GPS功能完成 */
	INIT,											/* 初始状态 */
	GET_GPS_GNRMC,									/* 获取GNRMC定位值 */
	GET_GPS_GNRMC_FINISH,							/* 获取GNRMC定位值完成 */
	CHECK_SIM_STATUS,								/* 查询SIM卡状态 */
	CHECK_SIM_STATUS_FINISH,						/* 查询SIM卡状态完成 */
	SEARCH_NET_STATUS,								/* 查找网络状态 */
	SEARCH_NET_STATUS_FINISH,						/* 查找网络状态完成 */
	GET_OPERATOR,									/* 获取运营商信息 */
	GET_OPERATOR_FINISH,							/* 获取运营商信息完成 */
	SET_TEXT_FORMAT_GSM,							/* 设置字符集格式为GSM */
	SET_TEXT_FORMAT_GSM_FINISH,						/* 设置字符集格式为GSM完成 */
	GET_SUBSCRIBER_NUMB,							/* 获取本机号码 */
	GET_SUBSCRIBER_NUMB_FINISH,						/* 获取本机号码完成 */
	CHECK_GPRS_STATUS,								/* 查找GPRS状态 */
	CHECK_GPRS_STATUS_FINISH,						/* 查找GPRS状态完成 */
	SET_SINGLE_LINK,								/* 设置单连接方式 */
	SET_SINGLE_LINK_FINISH,							/* 设置单连接方式完成 */
	SET_SERIANET_MODE,								/* 设置为透传模式 */
	SET_SERIANET_MODE_FINISH,						/* 设置为透传模式完成 */
	SET_APN_NAME,									/* 设置APN名称 */
	SET_APN_NAME_FINISH,							/* 设置APN名称完成 */
	ACTIVE_PDP,										/* 激活PDP场景 */
	ACTIVE_PDP_FINISH,								/* 激活PDP场景完成 */
	GET_SELF_IP_ADDR,								/* 获取本机IP地址 */
	GET_SELF_IP_ADDR_FINISH,						/* 获取本机IP地址完成 */
	GET_SIGNAL_QUALITY,								/* 获取信号质量 */
	GET_SIGNAL_QUALITY_FINISH,						/* 获取信号质量完成 */
	SET_SERVER_IP_ADDR,								/* 设置服务器地址 */
	SET_SERVER_IP_ADDR_FINISH,						/* 设置服务器地址完成 */
	READY,											/* 模块准备好 */
	DATA_SEND_FINISH,								/* 数据发送完成 */
	EXTI_SERIANET_MODE,								/* 退出透传模式 */
	EXTI_SERIANET_MODE_FINISH,						/* 退出透传模式完成 */
	RETURN_SERIANET_MODE,							/* 返回透传模式 */
	RETURN_SERIANET_MODE_FINISH,					/* 返回透传模式完成 */
	EXTI_LINK_MODE,									/* 退出连接模式 */
	EXTI_LINK_MODE_FINISH,							/* 退出连接模式完成 */
	SHUT_MODULE,									/* 关闭移动场景 */
	SHUT_MODULE_FINISH,								/* 关闭移动场景完成 */
} GPRS_ModuleStatusEnum;

typedef enum
{
	START_TASK_GPS,
	START_TASK_GPRS,
	START_TASK_GSM,
} GPRS_TaskStatusEnum;

/******************************************************************************/
void GPRSPROCESS_Task(void);


#endif
