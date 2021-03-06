/** 
 *   @file my_lcd.h
 *   @brief lcd 16x2 implementation
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 24/08/2020
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

#ifndef _MY_LCD_H
#define _MY_LCD_H

int my_lcd_set_time_out(uint8_t index);
int my_lcd_run(int flag);

#endif /* _MY_LCD_H */
