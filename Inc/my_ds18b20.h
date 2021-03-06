/** 
 *   @file my_ds18b20.h
 *   @brief Aplication layer for ds18b20.
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 05/03/2020
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

#ifndef _MY_DS18B20_H
#define _MY_DS18B20_H

#include "main.h"
#include "my_config.h"

#if MY_CFG_18B20_ENABLE
#  define THERM_NUM 77 /* 1 sec: 15, 2 sec: 77 */
#  define THERM_CALC_FLOAT 1

#  if THERM_NUM <= 15
#    define THERM_END_CICLE 250
#  elif  THERM_NUM <= 77
#    define THERM_END_CICLE 500
#  else
#    error "Invalid value THERM_NUM"
#  endif
#else
#  define THERM_NUM 0
#endif


typedef struct _ther_18b20_alarm {
	uint8_t id:4; /* Id on alarm table */
	uint8_t flag:2; /* 0: disable, 1: upper limit enable, 
			   2: lower limit enable, 3: both */
	uint8_t dumy:2;
} THER18B20ALARM;


typedef struct _ther_18b20_data { /* thermometer */
	uint8_t value_raw[2];
	uint8_t valid;
#if THERM_CALC_FLOAT
	float max;
	float min;
	float value;
	float value_avg;
#else
	int16_t val;
	int16_t max;
	int16_t min;
#endif
} THER18B20DATA;

typedef struct _alarm_table {
	int8_t up[16];
	int8_t low[16];
} ALARMTABLE;


typedef struct _ther_18b20_rom { /* thermometer */
	ALARMTABLE t;
	/* The size is always a multiple of 4 bytes: */
	uint8_t rom[THERM_NUM][8];
	int8_t offset[(THERM_NUM+3)&0xfffc]; /* calibration offset +/- 127
				   * centÚsimas de grado */
	/* The size is adjusted to complete multiples of 4 bytes: */
	THER18B20ALARM a[(THERM_NUM+3)&0xfffc];
        /* 0 to 15 of table ALARMTABLE. */
} THER18B20ROM;


//uint8_t crc8( uint8_t *addr, uint8_t len);
void my_18b20_init(UART_HandleTypeDef *huart);
void my_18b20_make_index_table(void);
int my_18b20_get_number(void);
uint8_t *my_18b20_get_id_table();
THER18B20DATA *my_18b20_get_data_p(void);
THER18B20ROM *my_18b20_get_rom_p(void);
int my_18b20_loop(void);
void my_18b20_scan(void);


#endif /* _MY_DS18B20_H */

