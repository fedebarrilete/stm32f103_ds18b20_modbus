/** 
 *   @file my_config.h
 *   @brief Some initialization parameters
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 27/01/2020
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
 *   27/01/2020: Created.
 **/

/* 
   Interesting note about NTP data client:
   https://seriot.ch/ntp.php

   About tcp clients and DNS:
   https://eax.me/stm32-w5500/
   http://www.nazim.ru/2386
*/


#ifndef _MY_CONFIG_H
#define _MY_CONFIG_H

#include <stdint.h>
#include "my_w5500.h"
#define MAC_MAGIC_PRJ 0x02 /* for MAC conformation: { 0xfe, 0xde,
			    * MAC_MAGIC_PRJ, ...} */

#define VERSION    0
#define SUBVERSION 5


#define MY_CFG_UPRINTF_ENABLE 1L
#define UPRINTF_BUFFER_SIZE 160
/* MY_CFG_UPRINTF_USE_TO:
   0: disable
   > 0: time out in ms.
*/
#define MY_CFG_UPRINTF_USE_TO 4L 

/*  MY_CFG_DEBUG :
    0: printf output is supresed.
    1: All printf output is route to huart1 device (DMA). 
    2: All printf output is route to WDO.
*/
#define MY_CFG_DEBUG 0L

/*
  MY_CFG_WDT_ENABLE:
  0: WDT is disable.
  1: WDT is enable.  
*/
#define MY_CFG_WDT_ENABLE       0L
#define MY_CFG_NTP_ENABLE       1L 
#define MY_CFG_RTC_ENABLE       0L /* FIXME: make long test in disable state. */
#define MY_CFG_DNS_ENABLE       0L /* It is not convenient to enable it. */
#define MY_CFG_LCD_ENABLE       0L 
#define MY_CFG_18B20_ENABLE     0L 
#define MY_CFG_ENCODER_ENABLE   0L /* Specify the type of encoder in
				    * encoder_sw.h */
#define MY_CFG_BAROMETER_ENABLE 0L /* BMP 180 */
#define MY_CFG_MODBUS_ENABLE    1L

#if MY_CFG_ENCODER_ENABLE
#  define ENC_TYPE_HW40_30P 1L
#  define ENC_TYPE_EN11_20P 0L
#else
#  define ENC_TYPE_HW40_30P 0L
#  define ENC_TYPE_EN11_20P 0L
#endif

#define MY_CFG_SERIAL_ID			        \
	(uint32_t) (					\
		MY_CFG_DEBUG /* bits 0,1 */|		\
		MY_CFG_UPRINTF_ENABLE  << 2|		\
		MY_CFG_UPRINTF_USE_TO  << 3|		\
		MY_CFG_WDT_ENABLE      << 4|		\
		MY_CFG_NTP_ENABLE      << 5|		\
		MY_CFG_DNS_ENABLE      << 6|		\
		MY_CFG_LCD_ENABLE      << 7|		\
		/* bits 8,9 an 10 for enc type */	\
		ENC_TYPE_HW40_30P      << 8|		\
		ENC_TYPE_EN11_20P      << 9|		\
		/* 10 for future encoder type */	\
		MY_CFG_18B20_ENABLE    <<11|		\
		MY_CFG_ENCODER_ENABLE  <<12|		\
		MY_CFG_BAROMETER_ENABLE<<13|		\
		MY_CFG_RTC_ENABLE      <<14|		\
		MY_CFG_MODBUS_ENABLE   <<15             )

/* MY_CFG_LCD_SPI: Usa SPI para comnicarse con un 74HC595 y controlar
   el display, el bit1 controla el LED de estado. No se usa CS, sino
   un LATCH para el resgistro del 595.

   MY_CFG_LCD_I2C: Usa I2C para el display, el cual debe habilitarse
   desde CubeMX. El LED de estado puede conectarse al pint de LATCH
   que esta sin uso en este caso.
 */
#if MY_CFG_LCD_ENABLE
#  define MY_CFG_LCD_SPI   1L
#  if !MY_CFG_LCD_SPI
#    define MY_CFG_LCD_I2C 1L
#    if MY_CFG_LCD_I2C
#      define LCD_ADDR 0x4e
#    endif
#  endif
#endif

/* TZ Data in Lib/ioLibrary_Driver/Internet/SNTP/sntp.c 
   18: UTC-03:0, 22: UTC 
*/
#define MY_TZ 18 //  UTC-03:0
//#define MY_TZ 22 //  UTC
#if (MY_TZ == 18)
#define MY_TZ_SRT " ar"
#elif  (MY_TZ == 22)
#define MY_TZ_SRT "utc"
#endif

#define my_tz_str() (char *) MY_TZ_SRT 

#define LED0_ON      HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET)
#define LED0_OFF     HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET)
#define LED0_TOGGLE  HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin)

#if MY_CFG_LCD_SPI
#define LED1_ON      lcd_bit1_set(1)
#define LED1_OFF     lcd_bit1_set(0)
#define LED1_TOGGLE  lcd_bit1_toggle()
#else
#define LED1_ON      HAL_GPIO_WritePin (LCD_LATCH_GPIO_Port, LCD_LATCH_Pin, GPIO_PIN_SET)
#define LED1_OFF     HAL_GPIO_WritePin (LCD_LATCH_GPIO_Port, LCD_LATCH_Pin, GPIO_PIN_RESET)
#define LED1_TOGGLE  HAL_GPIO_TogglePin(LCD_LATCH_GPIO_Port, LCD_LATCH_Pin)	
#endif

/*
#define LED1_ON      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET)
#define LED1_OFF     HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET)
#define LED1_TOGGLE  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin)
*/

#define BUZZER_ON   HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET)
#define BUZZER_OFF  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET)



#if MY_CFG_DEBUG == 0
#  define printf(...);
#endif


typedef struct _my_conf {
	wiz_NetInfo ni;
	uint8_t ntp_ip[4];
	uint8_t ntp_name[29];
	uint16_t mbport;
	uint16_t cfgport;  /* len 60 */
	uint8_t passwd[8]; /* len 68 */
	uint8_t id;        /* len 69 */
	uint8_t ntp_enable:1;
	uint8_t lcd_to:2; /* Time Out for LCD back light */
	uint8_t diin0_buzzer_enable:1; /* DIgital INput */
	uint8_t unusedbits:4; /* free for flags */
	uint8_t rtccr; /* Clock calibration */
	/* Memory reserved for future use: */
	uint8_t dummy; /* len 72 */
} MY_CONF;

#define SIZE_MY_CONF32 ((sizeof(MY_CONF) + 3) >> 2)


#define EE_BLK_LEN(n) (eeprom_blk[n] - (n > 0? eeprom_blk[n-1]:0))
#define EE_BLK_ADDR(n) (n > 0? eeprom_blk[n-1]:0)
#define EE_WRITE_BLK(n, a) EE_Writes(EE_BLK_ADDR(n), EE_BLK_LEN(n), a)
#define EE_READ_BLK(n, a)  EE_Reads(EE_BLK_ADDR(n), EE_BLK_LEN(n), a)
#define SIZE_NetInfo32 ((sizeof(wiz_NetInfo) + 3) >> 2)

#define MNC_GET_ID() my_conf->id
#define MNC_SET_ID(n) my_conf->id = n
#define MNC_PASSWD_ENABLE() my_conf->passwd[0]
#define MNC_GET_PASSWD() (char *)(my_conf->passwd)
#define MNC_NTP_ENABLE() my_conf->ntp_enable
#define MNC_RTCCR_ROM my_conf->rtccr

#define DOOR_BUZZER_ENABLE my_conf->diin0_buzzer_enable

#define ST_IP_ASSIGNED 0x0001
#define ST_ADQ_ENABLE  0x0002
#define ST_NTP_SET     0x0004
#define ST_NTP_FAIL    0x0008
#define ST_NTP_GET     0x0010
#define ST_TH_SCAN     0x0020
#define ST_LOOP_ERR    0x0100
#define ST_OW_ERR      0x0200
//#define ST_ERR (ST_NTP_FAIL|ST_LOOP_ERR|ST_OW_ERR)
#define ST_ERR (ST_NTP_FAIL|ST_OW_ERR)

extern volatile uint32_t sec_ticks;
extern volatile uint32_t day_ticks;
#if ! MY_CFG_RTC_ENABLE
extern volatile uint32_t skip_ms;
#endif
extern uint32_t state;
extern uint32_t conf_buff[];
extern MY_CONF *my_conf;
extern wiz_NetInfo *net_info_ee;
extern const uint32_t *eeprom_blk;
extern uint8_t wdt_active;
extern uint32_t cicle_break;
extern uint32_t adq_time;
#if MY_CFG_NTP_ENABLE	
extern int32_t ntp_diff;
#endif

/* Door info: */
extern uint32_t door_sw_open_time;
extern uint32_t door_sw_close_time;

/* Barometer BMP180: */
#if MY_CFG_BAROMETER_ENABLE
extern uint32_t pressure; /* calculated pressure in Pa */
#endif

/* modbus digital i/o: */
#if MY_CFG_MODBUS_ENABLE
extern uint16_t coil_register; /* Digital In/Out Register (fc = 1)*/
extern uint16_t dout_register; /* Digital Output Register (fc = 2) */
#endif

uint16_t wdt_get_counter(void);
void wdt_increment_counter(void);
void my_time_handler(void);

#endif /*  _MY_CONFIG_H */
