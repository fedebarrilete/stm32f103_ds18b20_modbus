/** 
 *   @file dht.c
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

#include "dht.h"
#include "my_config.h"

#define DHT_output(d) DHT_gpio_set(d, GPIO_MODE_OUTPUT_OD)
#define DHT_input(d)  DHT_gpio_set(d, GPIO_MODE_IT_RISING_FALLING)

#define TIMER_PERIOD 0xffff /* for 16 bit timers */

TIM_HandleTypeDef *DHT_timer = NULL;
volatile uint8_t DHT_timer_overflow = 0;


void  DHT_pin_change_callback(DHT *dht)
{
	uint16_t cnt = __HAL_TIM_GET_COUNTER(DHT_timer);
	dht->time = HAL_GetTick();
	if(dht->count < sizeof(dht->data) -1 ) {

		if ( cnt > dht->last_cnt )
			dht->data[dht->count] = cnt - dht->last_cnt;
		else
			dht->data[dht->count] = TIMER_PERIOD - dht->last_cnt +
				cnt;
		dht->last_cnt = cnt;
		dht->count++;
	}

	return;
}


static void  DHT_delay_us(uint16_t delay_us)
{
	int32_t delay;
	uint8_t over;
	__disable_irq();
	delay = __HAL_TIM_GET_COUNTER(DHT_timer) + delay_us;
	over = DHT_timer_overflow;
	__enable_irq();
	
	if ( delay > TIMER_PERIOD ) { /* timer overflow in the delay period */ 
		delay -= TIMER_PERIOD;
		while ( over == DHT_timer_overflow ); /* wait for overflow it*/
	}
	while ( __HAL_TIM_GET_COUNTER(DHT_timer) < delay );

	return;
}


static void DHT_gpio_set(DHT *dht, int mode)
{
	GPIO_InitTypeDef  gpio;
	if ( mode == GPIO_MODE_OUTPUT_OD )
		dht->gpio->BSRR = dht->pin;
	gpio.Mode = mode;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pin = dht->pin;
	gpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(dht->gpio,&gpio);    
	
}


static uint8_t DHT_decode(DHT *dht, uint8_t *decode)
{
	int8_t bit = 7;
	uint8_t *dp;
	
	if( (dht->data[0] < 40) || (dht->data[0] > 210) ||
	    (dht->data[1] < 60) || (dht->data[1] > 100) )
		return 1;      

	for (uint8_t i = 0; i < 80; i += 2) {
		dp = (uint8_t *) (dht->data + i + 2);
		if( *dp >= 35 && *dp <= 85 ) {
			dp++;
			if( *dp >= 10 &&  *dp <= 45 )
				*decode &= ~(1 << bit); 
			else if( *dp >= 55 && *dp <= 95 )
				*decode |= 1 << bit; 
			else
				return 1;
			bit--;
			if(bit == -1) {
				bit = 7;
				decode++;
			}
		} else
			return 1;  
	}
      
	return 0;  
}


void  DHT_init_timer(TIM_HandleTypeDef *tim)
{
	if ( DHT_timer == NULL ) {
		DHT_timer = tim;
		DHT_timer->Init.Prescaler = (HAL_RCC_GetHCLKFreq()/1000000L)-1;
		DHT_timer->Init.CounterMode = TIM_COUNTERMODE_UP;
		DHT_timer->Init.Period = TIMER_PERIOD;
		DHT_timer->Init.AutoReloadPreload =
			TIM_AUTORELOAD_PRELOAD_DISABLE;
		HAL_TIM_Base_Init(DHT_timer);
		HAL_TIM_Base_Start_IT(DHT_timer);
	}

	return;
}


void  DHT_init(DHT *dht, GPIO_TypeDef *gpio, uint16_t pin)
{
	dht->gpio = gpio;
	dht->pin = pin;
	dht->count = sizeof(dht->data); /* prenvet to callback run */
	dht->data_valid = 0;
	DHT_output(dht);
}


enum {
	DHT_OUT_LOW = 0,
	DHT_OUT_PUSE = 2,
	DHT_DECODE = 4,
	DHT_END,
	DHT_MAX = 500,
};


uint8_t  DHT_read_data(DHT *dht)
{
 	static uint32_t start_time;
	static uint32_t step = DHT_OUT_LOW;
	
	if ( step == DHT_OUT_LOW ) {
		DHT_output(dht);
		HAL_GPIO_WritePin(dht->gpio, dht->pin, GPIO_PIN_RESET);
	} else if ( step == DHT_OUT_PUSE ) {
		HAL_GPIO_WritePin(dht->gpio, dht->pin, GPIO_PIN_SET);
		DHT_delay_us(20);
		HAL_GPIO_WritePin(dht->gpio, dht->pin, GPIO_PIN_RESET);    
		DHT_delay_us(5);
		dht->count = 0;
		dht->last_cnt = __HAL_TIM_GET_COUNTER(DHT_timer);
		start_time = HAL_GetTick();
		DHT_input(dht);
	} else if ( step == DHT_DECODE ) {  
		if( HAL_GetTick() - start_time > 8 ) {
			step = DHT_END;
			return 2;
		}	
		if( HAL_GetTick() - dht->time > 1 ) {
			uint8_t data[5];
			dht->data_valid = 0;
			if( DHT_decode(dht, data) ) {
				step = DHT_END;
				return 3;
			}
			if( ((data[0] + data[1] + data[2] +
			      data[3]) & 0x00FF) != data[4] ) {
				step = DHT_END;
				return 3;
			}
			dht->temperature = (data[2] << 8) | data[3];
			dht->humidity = (data[0] << 8) | data[1];
			dht->data_valid = 1;
			DHT_output(dht);
			/*
			plot "ht01.log" u 1 w l lw 2, '' u 2 w l lw 2 axes x1y2, "<awk 'BEGIN{s=0}{if(s==0){a=$1;s=1}else{ if(($1-a)>0.5||(a-$1)>0.5){a=$1}else{a=(0.97 * a) + ($1 * 0.03)}}; print a;}' ht01.log" u 1 w l lw 2 lt 7
			*/
			return 0;
		} else {
			return 2;
		}
	} else if ( step == DHT_END ) {
		DHT_output(dht);
	}
	step++;
	if ( step == DHT_MAX ) step = DHT_OUT_LOW;
		
	return 1;
}

