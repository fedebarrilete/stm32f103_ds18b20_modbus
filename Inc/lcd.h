/** 
 *   @file lcd.h
 *   @brief LCD 16x2 via SPI or I2C.
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 11/3/2020
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


#ifndef _LCD_H
#define _LCD_H

#if MY_CFG_LCD_SPI
int lcd_init_spi (SPI_HandleTypeDef *spi, // initialize lcd
		  GPIO_TypeDef * Latch_Port, uint16_t Latch_Pin);
#endif
#if MY_CFG_LCD_I2C
int lcd_init_i2c ( I2C_HandleTypeDef *i2c, int addr );   // initialize lcd
#endif

void lcd_send_cmd (char cmd);  // send command to the lcd
void lcd_send_data (char data);  // send data to the lcd
void lcd_send_string (char *str);  // send string to the lcd
void lcd_put_cur(int row, int col);  // put cursor at the entered position row (0 or 1), col (0-15);
void lcd_clear (void);
int lcd_run(void);

void lcd_light_set(int val);
void lcd_light_toggle(void);


#if MY_CFG_LCD_SPI
void lcd_bit1_set(int val);
void lcd_bit1_toggle(void);
#endif



#endif /* _LCD_H */
