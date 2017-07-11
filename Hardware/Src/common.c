#include "common.h"

/*******************************************************************************
 *
 */
uint8_t HalfWord_GetHighByte(uint16_t value)
{
	return (uint8_t)((value & 0xFF00) >> 8);
}

uint8_t HalfWord_GetLowByte(uint16_t value)
{
	return (uint8_t)(value & 0x00FF);
}

/******************************************************************************/
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&DEBUG_UART, (uint8_t *)&ch, 1, 0xffff);
	return ch;
}

int fgetc(FILE * f)
{
	uint8_t ch = 0;
	HAL_UART_Receive(&DEBUG_UART, &ch, 1, 0xffff);
	return ch;
}

/*******************************************************************************
 *
 */
HAL_StatusTypeDef UART_DMAIdleConfig(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
	uint32_t *tmp;

	if((pData == NULL ) || (Size == 0))
	{
	  return HAL_ERROR;
	}

	huart->pRxBuffPtr = pData;
	huart->RxXferSize = Size;

	huart->ErrorCode = HAL_UART_ERROR_NONE;

	/* Enable the DMA channel */
	tmp = (uint32_t*)&pData;
	HAL_DMA_Start(huart->hdmarx, (uint32_t)&huart->Instance->DR, *(uint32_t*)tmp, Size);

	/* Enable the DMA transfer for the receiver request by setting the DMAR bit
	   in the UART CR3 register */
	SET_BIT(huart->Instance->CR3, USART_CR3_DMAR);

	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);

	return HAL_OK;
}


