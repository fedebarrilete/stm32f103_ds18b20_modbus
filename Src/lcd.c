/** 
 *   @file lcd.c
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

#include <stdio.h>
#include <string.h>
#include "my_config.h"
#include "lcd.h"
#if MY_CFG_LCD_I2C
#include "i2c.h"
#endif
#if MY_CFG_LCD_SPI
#include "spi.h"
#endif


#if MY_CFG_LCD_ENABLE


#if MY_CFG_LCD_I2C
#define I2C_USE_IRQ 1
static I2C_HandleTypeDef *_hi2c = NULL;
static int _lcd_addr = 0;
static volatile int i2c_tx_complete = 1;
#endif
static uint8_t data_t[4];
static char _str_out[33];
static char *_str = NULL;
static int _init = 0;
static uint32_t tickstart = 0;

static uint8_t light = 0x00;//0x08;

#if MY_CFG_LCD_SPI
static SPI_HandleTypeDef *_hspi = NULL;
static GPIO_TypeDef * _Latch_Port = NULL;
static uint16_t _Latch_Pin;
static uint8_t bit1 = 0x00; /* 0x00 or 0x02 */
#endif


#if MY_CFG_LCD_SPI
void lcd_bit1_set(int val)
{
	if (val) bit1 = 0x02;
	else bit1 = 0x00;
}


void lcd_bit1_toggle(void)
{
	bit1 ^= 0x02;
}


static void _lcd_spi_out_byte(uint8_t *byte)
{
	*byte &= ~0x0a; /* clear bit1 and light bit */
	*byte |= bit1 | light; /* set it need */
	HAL_SPI_Transmit(_hspi, byte, 1, 1);
	HAL_GPIO_WritePin(_Latch_Port, _Latch_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(_Latch_Port, _Latch_Pin, GPIO_PIN_SET);
}
#endif


void lcd_light_set(int val)
{
	if (val) light = 0x08;
	else light = 0x00;
}


void lcd_light_toggle(void)
{
	light ^= 0x08;
}



#if I2C_USE_IRQ
void HAL_I2C_MasterTxCpltCallback (I2C_HandleTypeDef *hi2c)
{
	if (hi2c->Instance == _hi2c->Instance ) {
		i2c_tx_complete = 1;
	}
}
#endif


static void lcd_paked_byte(uint8_t *data, char cmd, uint8_t type)
{
	char data_u, data_l;

	data_u = ( cmd & 0xf0 );
	data_l = ( (cmd << 4) & 0xf0 );
#if MY_CFG_LCD_I2C
	type |= light; /* include ligth state in type bite; only for
			* i2c bus */
#endif
	data[0] = data_u|0x04|type;  //en=1, rs=0/1 (type)
	data[1] = data_u|     type;  //en=0, rs=0/1 (type)
	data[2] = data_l|0x04|type;  //en=1, rs=0/1 (type)
	data[3] = data_l|     type;  //en=0, rs=0/1 (type)
}


static void lcd_paked_byte_sent(uint8_t *data, char cmd, uint8_t type)
{
#if MY_CFG_LCD_I2C
#if I2C_USE_IRQ	
	uint32_t count = 0;
	while( i2c_tx_complete == 0 && count++ < 40000);
	if ( count < 40000 ) {
#endif
#endif		
		lcd_paked_byte(data,cmd,type);

#if MY_CFG_LCD_I2C
#if I2C_USE_IRQ		
		HAL_I2C_Master_Transmit_IT (_hi2c, _lcd_addr,(uint8_t *) data, 4);
#else
		HAL_I2C_Master_Transmit(_hi2c, _lcd_addr,(uint8_t *) data, 4, 100);
#endif
#if I2C_USE_IRQ			
		i2c_tx_complete = 0;
	}
#endif
#endif
#if MY_CFG_LCD_SPI
	for (int i=0;i<4;i++){
		_lcd_spi_out_byte(((uint8_t *) data) + i);
	}
#endif
}


static void _lcd_init()
{
	static uint32_t step = 0;
	uint32_t delta = 0;
		
	if ( !tickstart ) {
		tickstart = HAL_GetTick();
		step = 0;
	}
	delta = HAL_GetTick() - tickstart;

	/* 4 bit initialisation: */
	
	if (delta >= 0 && step == 0 ) {
		step++;
		tickstart+=delta;
		lcd_send_cmd (0x30);
	} else if (delta >= 5 && step == 1 ) {
		step++;
		tickstart+=delta;
		/* wait for >4.1ms */
		lcd_send_cmd (0x30);
	} else if (delta >= 1 && step == 2 ) {
		step++;
		tickstart+=delta;
		/*  wait for >100us */
		lcd_send_cmd (0x30);
  	} else if (delta >= 10 && step == 3 ) {
		step++;
		tickstart+=delta;
		/* 4bit mode: */
		lcd_send_cmd (0x20);
	} else if (delta >= 10 && step == 4 ) {
		step++;
		tickstart+=delta;
		/* Dislay initialisation: */
		/* Function set --> DL=0 (4 bit mode), N = 1 (2 line
		 * display) F = 0 (5x8 characters): */
		lcd_send_cmd (0x28);
	} else if (delta >= 1 && step == 5 ) {
		step++;
		tickstart+=delta;
		/* Display on/off control --> D=0,C=0, B=0 --->
		 * display off */
		lcd_send_cmd (0x08);
	} else if (delta >= 1 && step == 6 ) {
		tickstart+=delta;
		step++;
		/* clear display: */
		lcd_send_cmd (0x01);
	} else if (delta >= 2 && step == 7 ) {
		tickstart+=delta;
		step++;
		/* Entry mode set --> I/D = 1 (increment cursor) & S =
		 * 0 (no shift): */
		lcd_send_cmd (0x06);
	} else if (delta >= 1 && step == 8 ) {
		//step++;
		//tickstart+=delta;
		/* Display on/off control --> D = 1, C and B =
		 * 0. (Cursor and blink, last two bits): */
		lcd_send_cmd (0x0C);
		_init = 1;
	}  
}


int lcd_run(void)
{
	int ret = 1;
	
	if( !_init ) {
		_lcd_init();
	} else {
#if  MY_CFG_LCD_I2C
		if ( i2c_tx_complete ) {
#endif
			if ( *_str ) {
				lcd_send_data (*_str++);
			} else {
				_lcd_spi_out_byte(data_t+3);
				ret = 0;
			}
#if  MY_CFG_LCD_I2C
		}
#endif		
	}

	return ret;
}


void lcd_send_cmd (char cmd)
{	
	lcd_paked_byte_sent(data_t, cmd, 0);
}


void lcd_send_data (char data)
{
	lcd_paked_byte_sent(data_t, data, 1);
}


void lcd_clear (void)
{
	lcd_send_cmd (0x01);
}


void lcd_put_cur(int row, int col)
{
	lcd_send_cmd ( col | (row?0xc0:0x80) );
}


#if MY_CFG_LCD_SPI
int lcd_init_spi (SPI_HandleTypeDef *hspi,
		  GPIO_TypeDef * Latch_Port, uint16_t Latch_Pin)
{
	_hspi = hspi;
	_Latch_Port = Latch_Port;
	_Latch_Pin = Latch_Pin;
	tickstart = 0;
	_init = 0;
	_lcd_init();

	return 0;
}
#endif

#if MY_CFG_LCD_I2C
int lcd_init_i2c (I2C_HandleTypeDef *i2c, int addr)
{
	_hi2c = i2c;
	_lcd_addr = addr;
	tickstart = 0;
	_init = 0;
	_lcd_init();

	return 0;
}
#endif

void lcd_send_string (char *str)
{
	int len = strlen(str);
	if ( len < 32 ) {
		memcpy(_str_out, str, len );
		_str_out[len] = 0;
		_str = _str_out;
	}
}

#endif /* MY_CFG_LCD_ENABLE */
