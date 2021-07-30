/** 
 *   @file dht.h
 *   @brief Humidity and temperture adquisition from DHT22 sensor
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 24/07/2021
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
 *   24/07/2021: Created.
 * 
 **/


#ifndef _DHT_H
#define _DHT_H

#include "tim.h"

extern TIM_HandleTypeDef *DHT_timer;
volatile extern uint8_t DHT_timer_overflow;

typedef struct
{
	GPIO_TypeDef      *gpio;
	uint16_t          pin;
	volatile uint32_t time;
	volatile uint16_t last_cnt;
	int16_t           temperature;
	uint16_t          humidity;
	volatile uint8_t count;  
	uint8_t           data_valid;
	volatile uint8_t  data[84]; 
} DHT;

void  DHT_pin_change_callback(DHT *dht);
/* A single timer for many sensors: */
void  DHT_init_timer(TIM_HandleTypeDef *tim);
/* One gpio for each sensors: */
void  DHT_init(DHT *dht, GPIO_TypeDef *gpio, uint16_t pin);
uint8_t  DHT_read_data(DHT *dht);

#endif
