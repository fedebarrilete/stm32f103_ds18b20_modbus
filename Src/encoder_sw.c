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


#include "encoder_sw.h"
#include "my_config.h"

#if ENC_TYPE_EN11_20P
#  define MAX_CNT 0x3fff
#elif ENC_TYPE_HW40_30P
#  define MAX_CNT 0x7fff
#else
# define MAX_CNT 0x7fff
#endif


#if MY_CFG_ENCODER_ENABLE


void enc_init(ENCODERROT *_rot,  TIM_HandleTypeDef *htim,
	      GPIO_TypeDef *sw_p,  uint16_t sw_pin )
{
	enc_setup_fc(_rot, -0x7FFFFFFF, 0x7FFFFFFF, 0, 1, 0, 0, 0, 0);
	_rot->raw_last = 0; /* timer counter start on 0 */
	_rot->htim = htim;
	_rot->sw_p = sw_p;
	_rot->sw_pin = sw_pin;
	_rot->sw_state = 0; 
	_rot->int_time = 0;

	HAL_TIM_Encoder_Start(htim, TIM_CHANNEL_ALL);
	
	return;
}

static int _get_raw(ENCODERROT *_rot)
{
#if ENC_TYPE_EN11_20P
	return __HAL_TIM_GET_COUNTER(_rot->htim) >> 1;
#elif ENC_TYPE_HW40_30P
	return __HAL_TIM_GET_COUNTER(_rot->htim);
#else
	return __HAL_TIM_GET_COUNTER(_rot->htim);
#endif
}


int32_t enc_get_current(ENCODERROT *_rot)
{
	uint32_t raw = _get_raw(_rot);
	int32_t diff = raw - _rot->raw_last;

	if ( diff > (MAX_CNT >> 1) ) diff -= MAX_CNT + 1;
	if ( diff < -(MAX_CNT >> 1) ) diff += MAX_CNT + 1;

#if ENC_TYPE_EN11_20P
	_rot->current += diff * _rot->inc;
#elif ENC_TYPE_HW40_30P
	_rot->current -= diff * _rot->inc;
#else
	_rot->current = raw;
#endif	
	if ( _rot->current > _rot->max ) _rot->current = _rot->max;
	else if ( _rot->current < _rot->min ) _rot->current = _rot->min;
	
	_rot->raw_last = raw;

	return _rot->current;
}


inline void enc_set_current(ENCODERROT *_rot, int32_t val)
{
	_rot->current = val;
}


uint8_t enc_get_sw(ENCODERROT *_rot, uint8_t num)
{
	return (num == 1?_rot->sw1:_rot->sw2);
}


void enc_setup(ENCODERROT *_rot, int32_t min, int32_t max, int32_t current,
		       int16_t inc, int8_t sw1, int8_t sw2)
{
	_rot->min = min;
	_rot->max = max;
        _rot->current = current;
	_rot->inc = inc;
	if ( sw1 > -1 ) _rot->sw1 = sw1;
	if ( sw2 > -1 ) _rot->sw2 = sw2;
	_rot->sw_toggle = 0;
}


void enc_setup_fc(ENCODERROT *_rot, int32_t min, int32_t max, int32_t current,
			  int16_t inc_f, int16_t inc_c, uint8_t sw_toggle, 
			  int8_t sw1, int8_t sw2)
{
	enc_setup(_rot, min, max, current, inc_f, sw1, sw2);
	_rot->inc_f = inc_f;
	_rot->inc_c = inc_c;
	_rot->sw_toggle = sw_toggle;
	
	if ( _rot->sw_toggle == 1 ) {
		if ( _rot->sw1 ) _rot->inc = _rot->inc_c;
		else _rot->inc = _rot->inc_f;
	} else if ( _rot->sw_toggle == 2 ) {
		if ( _rot->sw2 ) _rot->inc = _rot->inc_c;
		else _rot->inc = _rot->inc_f;
	}
}


void enc_switch_callback(ENCODERROT *_rot)
{
	uint32_t _now = HAL_GetTick();
	uint32_t _delta;
	if ( HAL_GPIO_ReadPin(_rot->sw_p, _rot->sw_pin) == 0 ) {
		if ( _rot->sw_state == 0 ) {
			_rot->int_time = _now;
			_rot->sw_state = 1;
		}
	} else if ( _rot->sw_state == 1 ) {
		_delta = _now - _rot->int_time;
		if ( _delta > 10 && _delta < 300 ) { /* ms */
			_rot->sw1 ^= 1;
			if ( _rot->sw_toggle == 1 ) {
				if ( _rot->sw1 ) _rot->inc = _rot->inc_c;
				else _rot->inc = _rot->inc_f;
			}
		} else if ( _delta >= 300 ) { /* ms */
			_rot->sw2 ^= 1;
			if ( _rot->sw_toggle == 2 ) {
				if ( _rot->sw2 ) _rot->inc = _rot->inc_c;
				else _rot->inc = _rot->inc_f;
			}
		}
		_rot->sw_state = 0;
	}

	return;	
}

#endif /* MY_CFG_ENCODER_ENABLE */
