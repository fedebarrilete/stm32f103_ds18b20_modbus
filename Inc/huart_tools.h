/** 
 *   @file huart_tools.h
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

#ifndef _HUART_TOOLS_H
#define _HUART_TOOLS_H

#include "usart.h"

#include "my_config.h"


void MX_UART_Baud_Rate_Init(UART_HandleTypeDef *huart, int baud_rate);

#define WAIT_R(u) while( u->gState != HAL_UART_STATE_READY)

#if MY_CFG_UPRINTF_ENABLE
int uprintf(UART_HandleTypeDef *huart, const char *fmt, ...);
#else
#define uprintf(...)
#endif

	
#endif /* _HUART_TOOLS_H */
