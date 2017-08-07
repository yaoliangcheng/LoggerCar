#include "print.h"

/******************************************************************************/
uint8_t PRINT_SendBuffer[PRINT_SEND_BYTES_MAX];

/******************************************************************************/
static void PRINT_SendData(uint8_t size);
static void PRINT_GetAnalog(uint16_t analog, uint8_t* buf);

/*******************************************************************************
 * function:打印输出
 */
void PRINT_DataOut(FILE_InfoTypedef* info, PRINT_ChannelSelectTypedef* select)
{
	uint8_t index = 0;

	/* 时间转换 */
	BCD2ASCII((char*)&PRINT_SendBuffer[index], &info->realTime.year, sizeof(FILE_RealTime));
	index += 12;

	/* 根据打印选择，输出数据 */
	if (select->status.bit.ch1)
	{
		PRINT_GetAnalog(info->analogValue.temp1, &PRINT_SendBuffer[index]);
		index += 5;
	}
	if (select->status.bit.ch2)
	{
		PRINT_GetAnalog(info->analogValue.humi1, &PRINT_SendBuffer[index]);
		index += 5;
	}
	if (select->status.bit.ch3)
	{
		PRINT_GetAnalog(info->analogValue.temp2, &PRINT_SendBuffer[index]);
		index += 5;
	}
	if (select->status.bit.ch4)
	{
		PRINT_GetAnalog(info->analogValue.humi2, &PRINT_SendBuffer[index]);
		index += 5;
	}
	if (select->status.bit.ch5)
	{
		PRINT_GetAnalog(info->analogValue.temp3, &PRINT_SendBuffer[index]);
		index += 5;
	}
	if (select->status.bit.ch6)
	{
		PRINT_GetAnalog(info->analogValue.humi3, &PRINT_SendBuffer[index]);
		index += 5;
	}
	if (select->status.bit.ch7)
	{
		PRINT_GetAnalog(info->analogValue.humi4, &PRINT_SendBuffer[index]);
		index += 5;
	}
	if (select->status.bit.ch8)
	{
		PRINT_GetAnalog(info->analogValue.temp1, &PRINT_SendBuffer[index]);
		index += 5;
	}

	/* 打印换行 */
	PRINT_SendBuffer[index] = '\n';
	index++;

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

	sprintf((char*)buf, "%5.1f", temp);
}


























