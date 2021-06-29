/** 
 *   @file huart_tools.c
 *   @brief Common tools for HUART port.
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 15/10/2019
 * 
 *   @copyright (c) Copyright 2018 Federico Bareilles <fede@fcaglp.unlp.edu.ar>.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *     
 *   The author does NOT admit liability nor provide warranty for any
 *   of this software. This material is provided "AS-IS" in the hope
 *   that it may be useful for others.
 *
 *   15/10/2019: Created.
 **/

#include "main.h"

#include "my_config.h"

#if MY_CFG_UPRINTF_ENABLE
#include <stdio.h>
#include <stdarg.h>
#endif

#include "huart_tools.h"


#if MY_CFG_UPRINTF_ENABLE

#ifdef UPRINTF_BUFFER_SIZE
#define __BUFFER_SIZE UPRINTF_BUFFER_SIZE
#else
#define __BUFFER_SIZE 80
#endif
#endif

/**
  * @brief USARTx at spesific baud rate Initialization Function
  * @param None
  * @retval None
  */
void MX_UART_Baud_Rate_Init(UART_HandleTypeDef *huart,
					  int baud_rate)
{
	//huart->Instance = USART2;
	huart->Init.BaudRate = baud_rate;
	//huart->Init.WordLength = UART_WORDLENGTH_8B;
	//huart->Init.StopBits = UART_STOPBITS_1;
	//huart->Init.Parity = UART_PARITY_NONE;
	//huart->Init.Mode = UART_MODE_TX_RX;
	//huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	//huart->Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(huart) != HAL_OK)
	{
		Error_Handler();
	}
	return;
}




#if MY_CFG_UPRINTF_ENABLE
/*
void HAL_UART_RxCpltCallback (UART_HandleTypeDef *huart)
{

	if (huart->Instance == USART1) {

	}
	return;
}
*/


int uprintf(UART_HandleTypeDef *huart, const char *fmt, ...)
{
        static char *buff[__BUFFER_SIZE];
	va_list ap;
#if MY_CFG_UPRINTF_USE_TO
	uint32_t stt = HAL_GetTick() + MY_CFG_UPRINTF_USE_TO;
#endif
	int n;
	
	if ( huart == NULL )
		return -2;
	while( huart->gState != HAL_UART_STATE_READY
#if MY_CFG_UPRINTF_USE_TO
	       && stt >= HAL_GetTick()
#endif
		);
#if MY_CFG_UPRINTF_USE_TO
	if (  huart->gState != HAL_UART_STATE_READY )
		return -1;
#endif
	va_start(ap, fmt);
	n = vsnprintf ((char *) buff, __BUFFER_SIZE, fmt, ap);
	va_end(ap);
	
        if ( n ) {
		HAL_UART_Transmit_DMA(huart, (uint8_t *) buff, n);
	}

        return n;
}

#endif



/*
void HAL_UART_TxCpltCallback (UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)      _huart1_busy = 0;
	else if (huart->Instance == USART2) _huart2_busy = 0;
	else if (huart->Instance == USART3) _huart3_busy = 0;
					       
	return;
}
*/
