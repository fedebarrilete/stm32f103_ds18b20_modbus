/** 
 *   @file encoder_sw.c
 *   @brief Encoder and switch
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 14/04/2021
 * 
 *   @copyright (c) Copyright 2021 Federico Bareilles <fede@fcaglp.unlp.edu.ar>.
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

#ifndef _ENCODER_SW_H
#define _ENCODER_SW_H


#include <stdint.h>
#include "gpio.h"

#include "my_config.h" /* only for ENC_TYPE_X configuration */

#ifndef ENC_TYPE_HW40_30P 
#  define ENC_TYPE_HW40_30P 1
#  define ENC_TYPE_EN11_20P 0
#endif

typedef volatile struct _encoder_rot {
	int32_t min;
	int32_t max;
	int32_t current;
	int32_t raw_last;
	int16_t inc;   /* inc active */
	int16_t inc_f; /* inc fine */
	int16_t inc_c; /*inc coarse */
	uint8_t sw1; /* One touch */
	uint8_t sw2; /* Large touch */
	uint8_t sw_toggle; /* sw for coarse/fine 0: none, 1: sw1, 2: sw2*/
	TIM_HandleTypeDef *htim;
	GPIO_TypeDef *sw_p;
	uint16_t sw_pin;

	int8_t sw_state;
	uint32_t int_time;
	
} ENCODERROT;


#if MY_CFG_ENCODER_ENABLE
extern ENCODERROT enc;
#endif

void enc_init(ENCODERROT *_rot, TIM_HandleTypeDef *htim,
	      GPIO_TypeDef *sw_p,  uint16_t sw_pin );

int32_t enc_get_current(ENCODERROT *en);
void enc_set_current(ENCODERROT *en, int32_t val);
uint8_t enc_get_sw(ENCODERROT *en, uint8_t num);

void enc_switch_callback(ENCODERROT *en);

void enc_setup(ENCODERROT *en, int32_t min, int32_t max,
	       int32_t current, int16_t inc,
	       int8_t sw1, int8_t sw2 );

void enc_setup_fc(ENCODERROT *en,int32_t min, int32_t max, int32_t current,
		  int16_t inc_f, int16_t inc_c, uint8_t sw_toggle, 
		  int8_t sw1, int8_t sw2);



#endif /*  _ENCODER_SW_H */
