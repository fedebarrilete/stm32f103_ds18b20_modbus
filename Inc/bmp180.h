/** 
 *
 *   @file bmp180.h
 *   @brief data adq. for barometer bmp180 
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 15/06/2021
 * 
 *   BMP180 pressure sensors
 *
 *   (c) Copyright 2021 Federico Bareilles <fede@fcaglp.unlp.edu.ar>.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 3 of
 *   the License, or (at your option) any later version.
 *     
 *   The author does NOT admit liability nor provide warranty for any
 *   of this software. This material is provided "AS-IS" in the hope
 *   that it may be useful for others.
 *
 *
 * No lo use, pero puede servir como referencia:
 * https://github.com/Seeed-Studio/Grove_Barometer_Sensor/blob/master/BMP085.cpp
 * 2018: escrito para arduino.
 *
 * 2021/06/15: Lo re escrivo la HAL de ST.

 **/

#ifndef _BMP180_H
#define _BMP180_H

#include "main.h"

#define BMP180_ADDR 0xee 

#define	BMP180_REG_CONTROL 0xf4
#define	BMP180_REG_RESULT 0xf6

#define	BMP180_CMD_TEMPERATURE 0x2f
#define	BMP180_CMD_PRESSURE0 0x34
#define	BMP180_CMD_PRESSURE1 0x74
#define	BMP180_CMD_PRESSURE2 0xb4
#define	BMP180_CMD_PRESSURE3 0xf4


typedef struct _bmp180 {
	int16_t AC1,AC2,AC3,VB1,VB2,MB,MC,MD;
	uint16_t AC4,AC5,AC6; 

	int32_t B3, B5, B6;
	uint32_t B4, B7;
	int32_t X3;

	uint8_t addr;
	uint8_t obersampling;
	uint8_t oss;
	uint8_t cicle;
	int32_t t; /* last temperature read*/
	int32_t p; /* last pressure read (raw)*/
	double p_fast;
	double p_slow; /* presure filtered */
	double press_total_avg;
	double press_rot_mem[20];
	double x, y;
	uint8_t rot_mem_pos;
	uint8_t rot_men_full;
	I2C_HandleTypeDef *hi2c;
	uint8_t loop;
} BMP180;

#define BMP180_GET_PRESSURE(b) b->p_slow
#define BMP180_GET_TEMPERATURE(b) b->t

BMP180 *bmp180_init(I2C_HandleTypeDef *hi2c, uint8_t addr,
		    uint8_t obersampling);
int baro_loop(BMP180 *baro);



#endif /* _BMP180_H */
