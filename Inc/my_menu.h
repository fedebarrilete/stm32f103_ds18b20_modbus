/** 
 *   @file my_menu.h
 *   @brief Some menu implementation
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 02/03/2020
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
 **/

/* 
   Interesting note about NTP data client:
   https://seriot.ch/ntp.php

   About tcp clients and DNS:
   https://eax.me/stm32-w5500/
   http://www.nazim.ru/2386
*/


#ifndef _MY_MENU_H
#define _MY_MENU_H

extern char msg[]; /* Global messafe buffer: my_menu.c */
//extern COMMAND *current_p500;
extern uint8_t prompt_def[];

int my_menu_send_to_client(uint8_t *msg, int len);
int my_menu_p500_run(uint8_t sn, int port);

#endif /* _MY_MENU_H */
