/** 
 *
 *   BMP180 pressure sensors
 *
 *   (c) Copyright 2018 Federico Bareilles <fede@fcaglp.unlp.edu.ar>.
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
 *   TODO: ALL
 *
 * No lo use, pero puede servir como referencia:
 * https://github.com/Seeed-Studio/Grove_Barometer_Sensor/blob/master/BMP085.cpp
 *
 * 2021/06/15: Lo re escrivo la HAL de ST.

 **/

#define _GNU_SOURCE

/* BMP180: */

#include <stdlib.h>
#include "bmp180.h"

#define USE_STATIC 1 /* 1: dont use malloc and no more one barometer
			0: use malloc and many barometer. */

#if USE_STATIC
static BMP180 __baro;
#endif

static int16_t _wire_read_int16(I2C_HandleTypeDef *hi2c, int addr, uint8_t cmd)
{
        uint8_t data[2];// = {cmd,};
	
        HAL_I2C_Master_Transmit(hi2c, addr, &cmd, 1, 1);
        HAL_I2C_Master_Receive( hi2c, addr, data, 2, 1);
	
        return data[0] << 8 | data[1];
}


BMP180 *bmp180_init(I2C_HandleTypeDef *hi2c, uint8_t addr, uint8_t obersampling)
{
	BMP180 *b = NULL;
	
	if (HAL_I2C_IsDeviceReady(hi2c , addr, 1, 1) == HAL_OK) {
#if USE_STATIC
		b = &__baro;
#else
		if ((b = (BMP180 *) malloc(sizeof(BMP180))) == NULL) {
			return NULL;
		}
#endif		
		
		b->addr = addr;
		b->hi2c = hi2c;
		b->obersampling = obersampling;
		switch( b->obersampling ) {
		case BMP180_CMD_PRESSURE0:
			b->cicle = 2;
			b->oss = 0;
			break;
		case BMP180_CMD_PRESSURE1:
			b->cicle = 2;
			b->oss = 1;
			break;
		case BMP180_CMD_PRESSURE2:
			b->cicle = 4;
			b->oss = 2;
			break;
		case BMP180_CMD_PRESSURE3:
			b->cicle = 7;
			b->oss = 3;
			break;
		}	
		b->AC1 = _wire_read_int16(b->hi2c, b->addr, 0xaa);
		b->AC2 = _wire_read_int16(b->hi2c, b->addr, 0xac);
		b->AC3 = _wire_read_int16(b->hi2c, b->addr, 0xae);
		b->AC4 = (uint16_t) _wire_read_int16(b->hi2c, b->addr, 0xb0);
		b->AC5 = (uint16_t) _wire_read_int16(b->hi2c, b->addr, 0xb2);
		b->AC6 = (uint16_t) _wire_read_int16(b->hi2c, b->addr, 0xb4);
		b->VB1 = _wire_read_int16(b->hi2c, b->addr, 0xb6);
		b->VB2 = _wire_read_int16(b->hi2c, b->addr, 0xb8);
		b->MB  = _wire_read_int16(b->hi2c, b->addr, 0xba);
		b->MC  = _wire_read_int16(b->hi2c, b->addr, 0xbc);
		b->MD  = _wire_read_int16(b->hi2c, b->addr, 0xbe);
	
		b->rot_men_full = 0;
		b->press_total_avg = 0.0;
		b->rot_mem_pos = 0;
		b->p = b->p_slow = b->p_fast = 0.0;
		b->loop = 0;
	}
	return b;
}


static int bmp180_start_temperature(BMP180 *baro)
{
	uint8_t data[2] = {BMP180_REG_CONTROL,BMP180_CMD_TEMPERATURE};

	if ( HAL_I2C_Master_Transmit(baro->hi2c,
				     baro->addr,  data, 2, 1) == HAL_OK )
                return 0;
        else
                return -1;
}


static double bmp180_get_temperature(BMP180 *baro)
{
	int16_t UT;
	int32_t X1, X2, X3;
	
	UT =  _wire_read_int16( baro->hi2c, baro->addr, BMP180_REG_RESULT);
	
	X1 = (((int32_t)UT - (int32_t)baro->AC6) * (int32_t)baro->AC5) >> 15;
	X2 = ((int32_t)baro->MC << 11) / (X1 + baro->MD);
	baro->B5 = X1 + X2;
	baro->t = (baro->B5 + 8) >> 4;

	/* For future pressure calculation: */
	baro->B6 = baro->B5 - 4000;
	X1 = (int32_t) (baro->VB2*((int32_t) (baro->B6*baro->B6) >> 12)) >> 11;
       
	X2 = (int32_t)(baro->AC2*baro->B6) >> 11;
	X3 = X1 + X2;
	baro->B3 = (((((int32_t)baro->AC1) * 4 + X3) << baro->oss) + 2) >> 2;
	
	X1 = (int32_t)(baro->AC3 * baro->B6 ) >> 13;
	X2 = (int32_t) (baro->VB1*((int32_t) (baro->B6*baro->B6) >> 12)) >> 16;
	baro->X3 = ((X1 + X2) + 2) >> 2;
		
	return baro->t;
}


static int bmp180_start_pressure(BMP180 *baro)
{
	uint8_t data[2] = {BMP180_REG_CONTROL, baro->obersampling};

	if ( HAL_I2C_Master_Transmit(baro->hi2c, baro->addr,
				     data, 2, 1) == HAL_OK )
                return 0;
        else
                return -1;
}


static double bmp180_get_pressure(BMP180 *baro)
{
	int32_t UP;
	int32_t X1, X2;
	uint8_t data[3] = {BMP180_REG_RESULT,};
	double press_diff;

	HAL_I2C_Master_Transmit(baro->hi2c, baro->addr,  data, 1, 1);
	HAL_I2C_Master_Receive( baro->hi2c, baro->addr, data, 3, 1);

	UP = (((int32_t) data[0]) << 16 | data[1] << 8
	      | data[2]) >> (8 - baro->oss);
	
	baro->B4 = (int32_t)(baro->AC4 * (uint32_t)(baro->X3 + 0x8000)) >> 15;
	baro->B7 = ((uint32_t) UP - baro->B3) * (50000 >> baro->oss);

	if ( baro->B7 < 0x80000000 ) {
		baro->p = (baro->B7 << 1) / baro->B4;
	} else {
		baro->p = ( baro->B7 / baro->B4) << 1;
	}
	
	X1 = (baro->p>>8)*(baro->p>>8);
	X1 = (int32_t)(X1 * 3038)>>16;
	X2 = (-7357 * baro->p)>>16;
	baro->p +=  (int32_t) (X1 + X2 + 3791) >> 4;

	
	if ( ! baro->rot_men_full ) {
		baro->press_rot_mem[baro->rot_mem_pos] = baro->p;
		baro->press_total_avg += baro->p;
		if ( baro->rot_mem_pos == 0 ) baro->p_slow = baro->p;
		baro->rot_mem_pos++;
		baro->p_fast = baro->press_total_avg / baro->rot_mem_pos;
		if (baro->rot_mem_pos == 20) {
			baro->rot_men_full = 1;
			baro->rot_mem_pos = 0;
		}
	} else {
		
		baro->press_total_avg -= baro->press_rot_mem[baro->rot_mem_pos];
		baro->press_rot_mem[baro->rot_mem_pos] = baro->p;
		baro->press_total_avg += baro->press_rot_mem[baro->rot_mem_pos];
		baro->rot_mem_pos++;
		if ( baro->rot_mem_pos == 20 ) baro->rot_mem_pos = 0;
		baro->p_fast = baro->press_total_avg / 20.0;
	}

	
	baro->p_slow = baro->p_slow * 0.99 + baro->p_fast * 0.01;
	press_diff = baro->p_slow - baro->p_fast;
	if ( press_diff > 8 ) press_diff = 8;
	if ( press_diff < -8 ) press_diff = -8;
	if ( press_diff > 1 || press_diff < -1 )
		baro->p_slow -=  press_diff / 6.0;

	
	return baro->p_slow;
}


/* baro_loop: call every 4 ms */
int baro_loop(BMP180 *baro)
{
	int ret = 0;

	if (baro == NULL ) return -1;
	if (baro->loop == 2 ) {
		bmp180_get_temperature(baro);
		ret = bmp180_start_pressure(baro);
	} else if ( baro->loop == (2 + baro->cicle) ) {
		bmp180_get_pressure(baro);
		ret = bmp180_start_temperature(baro);
		baro->loop = 0;
		if ( ret == 0 )
			ret = 1;
	} else if ( baro->loop == 0 ) {
		ret = bmp180_start_temperature(baro);
	}
	baro->loop++;

	return ret; /* 1: new presure calculated */
}

