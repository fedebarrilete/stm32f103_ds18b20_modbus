/** 
 *   @file my_tcp.h
 *   @brief TCP server implementation.
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 15/09/2020
 * 
 *   @copyright (c) Copyright 2020 Federico Bareilles <fede@fcaglp.unlp.edu.ar>.
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
 *   15/09/2020: Created.
 **/

#ifndef _MY_TCP_H
#define _MY_TCP_H

#include "cmd.h"

#define MAX_SOCKS _WIZCHIP_SOCK_NUM_

/* From 0 to 7 */


#define TCP0_SOCKET     0 // ModBus
#define TCP1_SOCKET     1 // Config
#define TCP2_SOCKET     2 // Config
#define NTP_SOCKET      3 // NTP
#define DHCP_SOCKET     4
#define DNS_SOCKET      5
//#define HTTP_SOCKET     6
#define TCP7_SOCKET     7


void set_prompt(uint8_t sn, uint8_t *pt);

int tcp_server_listen_on(uint8_t sn, uint16_t port);
int tcp_server_run(uint8_t sn, uint16_t port);
int tcp_server_menu(uint8_t sn, COMMAND *cmd, uint8_t show_pt);

#endif /* _MY_TCP_H */
