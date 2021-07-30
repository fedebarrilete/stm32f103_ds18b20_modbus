/** 
 *   @file my_w5500.h
 *   @brief initialization ethernet w5500 via ioLibrary_Driver.
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 26/01/2020
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
 *   26/01/2020: Created.
 **/


#ifndef _MY_W5500_H
#define _MY_W5500_H

#include "Ethernet/socket.h"
#include "main.h"
#include "cmd.h"

extern volatile uint8_t new_ip_assigned;

//void set_prompt(uint8_t *pt);

int my_w5500_init(SPI_HandleTypeDef *hspi, wiz_NetInfo *net_info);
void my_w5500_set_loggin_uart(UART_HandleTypeDef *u);
//void print_net_data(wiz_NetInfo *net_info);
int my_dhcp_run(void);

#endif /* _MY_W5500_H */
