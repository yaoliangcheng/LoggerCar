#include "print.h"

/******************************************************************************/
uint8_t PRINT_SendBuffer[PRINT_SEND_BYTES_MAX];

/******************************************************************************/
static void PRINT_SendData(uint8_t size);
static void PRINT_GetAnalog(uint16_t analog, uint8_t* buf);

/*******************************************************************************
 * function:打印日期
 */
void PRINT_Date(char* fileName)
{
	memcpy(&PRINT_SendBuffer[0], "*************", 13);
	memcpy(&PRINT_SendBuffer[13], fileName, 6);
	memcpy(&PRINT_SendBuffer[19], "*************", 13);
	PRINT_SendBuffer[33] = '\n';
	PRINT_SendData(33);
}

/*******************************************************************************
 * function:打印输出
 */
void PRINT_DataOut(FILE_InfoTypedef* info, PRINT_ChannelSelectTypedef* select)
{
	uint8_t index = 0;

	/* 时间转换 */
	BCD2ASCII((char*)&PRINT_SendBuffer[0], &info->realTime.hour, 1);
	PRINT_SendBuffer[2] = ':';
	BCD2ASCII((char*)&PRINT_SendBuffer[3], &info->realTime.min, 1);
	PRINT_SendBuffer[5] = ':';
	BCD2ASCII((char*)&PRINT_SendBuffer[6], &info->realTime.sec, 1);
	index += 8;

	/* 根据打印选择，输出数据 */
	if (select->status.bit.ch1)
	{
		PRINT_GetAnalog(info->analogValue.temp1, &PRINT_SendBuffer[index]);
		index += 6;
	}
	if (select->status.bit.ch2)
	{
		PRINT_GetAnalog(info->analogValue.humi1, &PRINT_SendBuffer[index]);
		index += 6;
	}
	if (select->status.bit.ch3)
	{
		PRINT_GetAnalog(info->analogValue.temp2, &PRINT_SendBuffer[index]);
		index += 6;
	}
	if (select->status.bit.ch4)
	{
		PRINT_GetAnalog(info->analogValue.humi2, &PRINT_SendBuffer[index]);
		index += 6;
	}
	if (select->status.bit.ch5)
	{
		PRINT_GetAnalog(info->analogValue.temp3, &PRINT_SendBuffer[index]);
		index += 6;
	}
	if (select->status.bit.ch6)
	{
		PRINT_GetAnalog(info->analogValue.humi3, &PRINT_SendBuffer[index]);
		index += 6;
	}
	if (select->status.bit.ch7)
	{
		PRINT_GetAnalog(info->analogValue.humi4, &PRINT_SendBuffer[index]);
		index += 6;
	}
	if (select->status.bit.ch8)
	{
		PRINT_GetAnalog(info->analogValue.temp1, &PRINT_SendBuffer[index]);
		index += 6;
	}

	/* 打印换行 */
	PRINT_SendBuffer[index] = '\n';
	index++;

	PRINT_SendData(index);
}

/*******************************************************************************
 * function：打印标题数据
 */
void PRINT_TitleOut(void)
{
	uint8_t index = 0;

	memcpy(&PRINT_SendBuffer[0], "********************************\n", 33);
	index += 33;

	memcpy(&PRINT_SendBuffer[index], "收货方：\n", 9);
	index += 9;

	memcpy(&PRINT_SendBuffer[index], "发货方：\n", 9);
	index += 9;

	memcpy(&PRINT_SendBuffer[index], "派送车牌：\n", 11);
	index += 11;

	memcpy(&PRINT_SendBuffer[index], "订单编号：\n", 11);
	index += 11;

	PRINT_SendData(index);
}

/*******************************************************************************
 *
 */
void PRINT_TailOut(void)
{
	uint8_t index = 0;

	memcpy(&PRINT_SendBuffer[0], "********************************\n", 33);
	index += 33;

	memcpy(&PRINT_SendBuffer[index], "签收人：\n", 9);
	index += 9;

	memcpy(&PRINT_SendBuffer[index], "\n\n\n", 3);
	index += 3;

	memcpy(&PRINT_SendBuffer[index], "签收日期：\n", 11);
	index += 11;

	memcpy(&PRINT_SendBuffer[index], "\n\n\n", 3);
	index += 3;

	PRINT_SendData(index);
}

/*******************************************************************************
 *
 */
static void PRINT_SendData(uint8_t size)
{
//	HAL_UART_Transmit_DMA(&PRINT_UART, PRINT_SendBuffer, size);
	HAL_UART_Transmit(&PRINT_UART, PRINT_SendBuffer, size, 1000);
}

/*******************************************************************************
 *
 */
static void PRINT_GetAnalog(uint16_t analog, uint8_t* buf)
{
	float temp;
	uint16_t data;

	data = (HalfWord_GetLowByte(analog) << 8) | HalfWord_GetHighByte(analog);

	/* 判断正负 */
	if ((data & 0x8000) != 0)
		temp = -((float)(data & 0x7FFF) / 10);
	else
		temp = (float)(data & 0x7FFF) / 10;

	sprintf((char*)buf, "%6.1f", temp);
}


























