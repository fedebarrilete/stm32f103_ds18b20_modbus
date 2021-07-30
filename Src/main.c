/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/*
  https://seriot.ch/ntp.php:
  echo $(( 0x`printf c%47s|nc -uw1 ntp.aggo-conicet.gob.ar 123|xxd -s40 -l4 -p`-64#23GDW0))

echo $(( 0x$(printf c%47s | nc -uw1 ntp.aggo-conicet.gob.ar 123|xxd -s40 -l4 -p) - 2208988800 ))

 Number of seconds between 1900 and 1970: 2208988800

*/

/* Add on stm32f1xx_it.c in function SysTick_Handler():

	if(ticks == 1000) {
		DHCP_time_handler();
		DNS_time_handler();
		ticks = 0;
	}  

*/

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "my_config.h"
#include "my_w5500.h"
#include "my_tcp.h"
#include "my_rtc_time.h"
#include "huart_tools.h"

#if MY_CFG_18B20_ENABLE
#include "my_ds18b20.h"
#include "ds18b20.h"
#endif

#include "cmd.h"
#include "eeprom.h"
#include "my_menu.h"
#include "modbus.h"

#if MY_CFG_LCD_ENABLE
#include "lcd.h"
#include "my_lcd.h"
#endif

#include "Ethernet/socket.h"
#include "DHCP/dhcp.h"
#include "DNS/dns.h"

#ifndef MY_CFG_NTP_ENABLE
# define MY_CFG_NTP_ENABLE  0L
#endif

#if MY_CFG_NTP_ENABLE		  
#include "SNTP/sntp.h"
#endif

#if MY_CFG_ENCODER_ENABLE		  
#include "encoder_sw.h"
#endif

#if MY_CFG_BAROMETER_ENABLE
#include "bmp180.h"
#endif

#if MY_CFG_DHT_ENABLE
#include "dht.h"
#endif


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define UNIXT_OFFSET 1577836800U /* date --date="2020-01-01 00:00:00" -u +%s */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

volatile uint32_t sec_ticks = 0;
volatile uint32_t day_ticks = 0;
#if ! MY_CFG_RTC_ENABLE
volatile uint32_t skip_ms = 0;
#endif

uint8_t wdt_active = 0;
uint32_t conf_buff[SIZE_MY_CONF32] = {0,};
MY_CONF *my_conf = (MY_CONF *) conf_buff;
wiz_NetInfo *net_info_ee = NULL;
const uint32_t *eeprom_blk;
uint32_t cicle_break = 0;
uint32_t state = 0;
uint32_t adq_time = 0;
#if MY_CFG_NTP_ENABLE
int32_t ntp_diff = 0;
#endif

uint32_t door_sw_open_time = 0;
uint32_t door_sw_close_time = 0;

#if MY_CFG_BAROMETER_ENABLE
BMP180 *barometer = NULL;
uint32_t pressure = 0;
#endif

#if MY_CFG_NTP_ENABLE
static uint8_t ntp_data[MAX_SNTP_BUF_SIZE]={0};
#endif

#if MY_CFG_ENCODER_ENABLE
ENCODERROT enc;
#endif

#if MY_CFG_MODBUS_ENABLE
uint16_t coil_register = 0;
uint16_t dout_register;
#endif

#if MY_CFG_DHT_ENABLE
DHT dht22;
uint32_t dht_humidity;
int32_t dht_temperature;
#endif


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void my_time_handler(void)
{

	sec_ticks++;
	DHCP_time_handler();
#if MY_CFG_DNS_ENABLE
	DNS_time_handler();
#endif
	if ( (sec_ticks % 86400) == 0) day_ticks++;
}


static void leds_run(void)
{
	static uint32_t sec_on = 0;
	static uint32_t count = 0;
	if ( state & ST_IP_ASSIGNED ) {
		if (sec_on != sec_ticks ) {
			LED1_ON;
			LED0_ON;
			sec_on = sec_ticks;
			count = 0;
		} else if ( count == 12 ) {
			LED1_OFF;
			LED0_OFF;
		}
	} else {
		if (count % 30 == 0) {
			LED1_TOGGLE;
			LED0_TOGGLE;
		}
	}
	count++;
	
	return;
}
	

uint16_t wdt_get_counter(void)
{
	uint32_t count;

	EE_READ_BLK(1, &count);
	if ( count == 0xffffffff )
		count = 0;
	
	return (uint16_t) count;
}


void wdt_increment_counter(void)
{
	int ret;
	int retry = 0;
	uint32_t count = wdt_get_counter() + 1;
	
	do {
		ret = EE_WRITE_BLK(1, &count);
	} while ( ret == 0 && retry++ < 3 );
}


static void main_ip_client_loop(void)
{
	int8_t ret = my_dhcp_run();
	
	if ( ret == DHCP_IP_LEASED || net_info_ee->dhcp == NETINFO_STATIC ) {
#if MY_CFG_NTP_ENABLE 
		if ( !(state & ST_IP_ASSIGNED ) ) {
#  if MY_CFG_DNS_ENABLE
			wiz_NetInfo net_info;
			wizchip_getnetinfo(&net_info);
			uprintf(&huart1,"# [%ld] Resolving NTP server \"%s\"...\r\n", sec_ticks, my_conf->ntp_name);
			if( DNS_run(net_info.dns, my_conf->ntp_name, my_conf->ntp_ip) == 1) {
				uprintf(&huart1,"# [%ld] Result: %d.%d.%d.%d\r\n",
					sec_ticks,
					my_conf->ntp_ip[0],
					my_conf->ntp_ip[1],
					my_conf->ntp_ip[2],
					my_conf->ntp_ip[3]);
			}
#  endif
			/* 18: UTC-03:0, 22: UTC */
			SNTP_init(NTP_SOCKET, my_conf->ntp_ip, MY_TZ, ntp_data);
			
			uprintf(&huart1,"# [%ld] NTP server: %d.%d.%d.%d\r\n",
				sec_ticks, my_conf->ntp_ip[0],
				my_conf->ntp_ip[1], my_conf->ntp_ip[2],
				my_conf->ntp_ip[3]);
			
			cicle_break = 1;
			state |= ST_IP_ASSIGNED;
		}
#else
		if ( !(state & ST_IP_ASSIGNED) ) cicle_break = 1;
		state |= ST_IP_ASSIGNED;
#endif
	} else {
		cicle_break = 1;
		state &= ~(ST_IP_ASSIGNED);
	}
	return;
}


#if MY_CFG_NTP_ENABLE
static void main_ntp_loop(void)
{
	static uint32_t seconds_old = 0L;
	static uint32_t ntp_count = 0L;
	datetime time;
  
	if ( SNTP_run(&time) == 1 && (MNC_NTP_ENABLE() || state & ST_NTP_GET) ) {
		uint32_t seconds = 0U;
		ntp_count = 0;
		for (uint8_t _i = 40; _i < 44; _i++) {
			seconds = (seconds << 8) | ntp_data[_i];
		}
		/* Number of seconds between 1900 and 1970: */
		seconds -= 2208988800U;
		if ( (seconds - seconds_old) == 1 ){
			if ( !(state & (ST_NTP_SET | ST_NTP_FAIL)) &&
			     MNC_NTP_ENABLE() ) {
				my_HAL_Set_Unix_Time(&hrtc, seconds);
				ntp_diff = 0;
				state |= ST_NTP_SET;
			} else {
				uint32_t seconds_local = my_time(NULL);
				if ( seconds_local >= seconds ) {
					ntp_diff = seconds_local - seconds;
				} else {
					ntp_diff = seconds - seconds_local;
					ntp_diff *= -1;
				}
				state &= ~(ST_NTP_GET);
			}
			uprintf(&huart1,
				"# [%ld] %4d-%02d-%02d %02d:%02d:%02d [%lu] %s\r\n",
				sec_ticks, time.yy, time.mo, time.dd,
				time.hh, time.mm, time.ss, seconds,
				my_tz_str() );
		}
		seconds_old = seconds;
	} else if ( MNC_NTP_ENABLE() ) {
		if (ntp_count++ == 100)
			state |= ST_NTP_FAIL;
	}

	return;
}
#endif


#if MY_CFG_ENCODER_ENABLE || MY_CFG_DHT_ENABLE
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
#if MY_CFG_ENCODER_ENABLE
	if ( GPIO_Pin == enc.sw_pin ) {
		enc_switch_callback(&enc);
	}
#endif
#if MY_CFG_DHT_ENABLE
	if(GPIO_Pin == dht22.pin) {
		DHT_pin_change_callback(&dht22);
	}
#endif	
	return;
}
#endif


#if MY_CFG_DHT_ENABLE
void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef * htim)
{

	if ( htim->Instance == DHT_timer->Instance ) DHT_timer_overflow ^= 1;

	return;
}
#endif




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint32_t *id = (uint32_t *) UID_BASE;
  int i = 0;
  uint32_t tickstart;
  const uint32_t wait = 4;
  uint32_t cicle_delta;
#if MY_CFG_NTP_ENABLE
  uint32_t day_now = 0;
#endif
  int8_t ret = 0;
  int rom_size = 0;
  int door_sw_open = -1;
#if MY_CFG_LCD_ENABLE
  uint32_t switch_state = 0;
  int clear_pin = 0;  
#  if MY_CFG_ENCODER_ENABLE
  int esw1 = 0;
#  endif
#endif
  
  const MY_CONF my_conf_def = {
	  .ni = {
		  .mac  = { 0xfe, 0xde, MAC_MAGIC_PRJ,
			    (uint8_t)id[2], (uint8_t)id[1], (uint8_t)id[0] },
		  .dhcp = NETINFO_DHCP,
		  //.dhcp = NETINFO_STATIC,
	  },
	  /* ntp_name: Need MY_CFG_DNS_ENABLE: */
#if MY_CFG_DNS_ENABLE
	  .ntp_name = {"ntp.aggo-conicet.gob.ar"},
#else
	  .ntp_ip = {168,96,251,196}, /* ntp.aggo-conicet.gob.ar */
#endif
	  .mbport = 502,
	  .cfgport = 500,
	  .passwd = {0,},
	  .id = 0,
	  .ntp_enable = 1,
	  .lcd_to = 0, /* default disable display off */
  };
#if MY_CFG_18B20_ENABLE
  THER18B20ROM *th = my_18b20_get_rom_p();
#endif
  const uint32_t _eeprom_blk[] = {/* 4 bytes uinits: */
	  /* BLK 0 */ SIZE_MY_CONF32,
	  /* BLK 1 */ _eeprom_blk[i++] + 1, /* WDT counter */
	  /* BLK 2 */ _eeprom_blk[i++] + (sizeof(THER18B20ROM) >> 2),
	  /* BLK 3 */ 0,
  };
  eeprom_blk = _eeprom_blk;
  for(i=0; eeprom_blk[i] != 0; i++) rom_size+= eeprom_blk[i];

  
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
#if 0 /* 0: disable all auto MX initialization. */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_IWDG_Init();
  MX_RTC_Init();
  MX_USART3_UART_Init();
  MX_SPI2_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
#else
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
#if MY_CFG_UPRINTF_ENABLE
  MX_USART1_UART_Init();
#endif
#if  MY_CFG_WDT_ENABLE
  MX_IWDG_Init();
#endif
#if MY_CFG_RTC_ENABLE
  MX_RTC_Init();
#endif
#if MY_CFG_18B20_ENABLE
  MX_USART3_UART_Init();
#endif
#if MY_CFG_LCD_ENABLE
  MX_SPI2_Init();
#endif
//  MX_ADC1_Init();
#if MY_CFG_ENCODER_ENABLE
  MX_TIM3_Init();
#endif
#if MY_CFG_BAROMETER_ENABLE
  MX_I2C1_Init();
#endif
#if MY_CFG_DHT_ENABLE
  MX_TIM4_Init();
#endif
#endif
  
  /* Test for 74HC595: */
  /*
  uint8_t coil_val[] = {0xff,0xff};
  HAL_SPI_Transmit(&hspi2, coil_val, 2, 1);
  HAL_GPIO_WritePin(LCD_LATCH_GPIO_Port, LCD_LATCH_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_LATCH_GPIO_Port, LCD_LATCH_Pin, GPIO_PIN_SET);
  */

  
  /* Hard Reset W5500 start: */
  HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(1); /* RESET should be held low at least 500 us for W5500: */
  HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);

  tickstart = HAL_GetTick();

  uprintf(&huart1,
	  "\r\n# [%ld] STM32 %d KB FLASH, ID: %08X-%08X-%08X\r\n",
	  sec_ticks,
	  *((unsigned short *) FLASHSIZE_BASE), /* Flash in kB */
	  id[0], id[1], id[2]);
  uprintf(&huart1,"# [%ld] ROM %d B\r\n", sec_ticks, rom_size << 2);
  uprintf(&huart1,"# [%ld] CLK Freq: %ld Hz\r\n", sec_ticks,
	  HAL_RCC_GetHCLKFreq() );
#if MY_CFG_18B20_ENABLE
  my_18b20_init(&huart3);
#endif
  
  /*## Check if the system has resumed from IWDG reset ##*/
#if MY_CFG_WDT_ENABLE
  if(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET) {
	  wdt_active = 1;
	  /* Clear reset flags */
	  __HAL_RCC_CLEAR_RESET_FLAGS();
	  wdt_increment_counter();
	  
  } else {
	  /* IWDGRST flag is not set. */
	  wdt_active = 0;
  }
#endif
  
  net_info_ee = &(my_conf->ni);
  
  if ( HAL_GPIO_ReadPin(Clear_GPIO_Port, Clear_Pin) == 0 ) {
	  int _i;
	  int retry = 0;
	  /* Reset configuration: */
	  for ( _i = 0; _i < SIZE_MY_CONF32;_i++)
		  conf_buff[_i] = 0xffffffff;

	  do {
		  ret = EE_WRITE_BLK(0, conf_buff);
	  } while ( ret == 0 && retry++ < 3 );
	  
  } else {
	  EE_READ_BLK(0, conf_buff);
  }
  if ( conf_buff[0] == 0xffffffff ) {
	  memcpy(my_conf, &my_conf_def, sizeof(MY_CONF) );
  }

  if ( net_info_ee->dhcp != 1 )
	  net_info_ee->dhcp = NETINFO_DHCP;

  /* Hard Reset W5500 end: */
  while ((HAL_GetTick() - tickstart) < 1);
  my_w5500_set_loggin_uart(&huart1);
  if ( my_w5500_init(&hspi1, net_info_ee) != 0 ) {
	  LED1_ON;
	  while(1);
  }
  LED0_OFF;
  
#if MY_CFG_18B20_ENABLE
  EE_READ_BLK(2, (uint32_t *) th);
  for(i = THERM_NUM -1; i && th->rom[i][0] == 0xff; i--) {
	  th->rom[i][0] = 0;
	  th->offset[i] = 0;
  }

  my_18b20_make_index_table();
#endif
#if MY_CFG_RTC_ENABLE
  /* Cofigure RTCCR register: (need only if battery is removed) */
  if ( MNC_RTCCR_ROM > 0 && MNC_RTCCR_ROM < 0x7f ) {
	  uint8_t bkpccr = READ_REG(BKP->RTCCR);
	  if ( (bkpccr & 0x7f) == 0 ) {
		  bkpccr = (bkpccr&0xff80) | (MNC_RTCCR_ROM & 0x7f);
		  WRITE_REG(BKP->RTCCR,  bkpccr);
	  }
  }
#else
  if ( MNC_RTCCR_ROM )
	  skip_ms = MNC_RTCCR_ROM * 1000;
#endif
  
//#if MY_CFG_NTP_ENABLE /* set anyway */
  SNTP_set_TZ(MY_TZ);
//#endif
#if MY_CFG_LCD_ENABLE
# if MY_CFG_LCD_I2C
  lcd_init_i2c (&hi2c1, LCD_ADDR);
# endif

# if MY_CFG_LCD_SPI
  //lcd_init_spi(&hspi2, HC595_LATCH_GPIO_Port, HC595_LATCH_Pin);
  lcd_init_spi(&hspi2, LCD_LATCH_GPIO_Port, LCD_LATCH_Pin);
# endif
  lcd_light_set(1);
  my_lcd_set_time_out(my_conf->lcd_to);
#endif

#if MY_CFG_WDT_ENABLE
  uprintf(&huart1,"# [%ld] WDT: %d\r\n", sec_ticks, wdt_active );
#endif

#if MY_CFG_MENUCFG_ENABLE  
  set_prompt(TCP1_SOCKET, prompt_def);
  set_prompt(TCP2_SOCKET, prompt_def);
#endif
//  state |= ST_ADQ_ENABLE;

#if MY_CFG_ENCODER_ENABLE
  enc_init(&enc, &htim3,
	   ENC_SW_GPIO_Port,
	   ENC_SW_Pin
	  );
  enc_setup(&enc, 0, 1, 0, 1, 0, 0);
  //enc_setup_fc(&enc, 0, 100, 0, 1, 5, 1, 0, 0);
#endif
  
#if MY_CFG_BAROMETER_ENABLE
  barometer = bmp180_init(&hi2c1, BMP180_ADDR, BMP180_CMD_PRESSURE3);
#endif

#if MY_CFG_DHT_ENABLE
  DHT_init_timer(&htim4);
  DHT_init(&dht22, DHT22_GPIO_Port, DHT22_Pin); 
#endif

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  tickstart = HAL_GetTick();
#if MY_CFG_WDT_ENABLE
	  HAL_IWDG_Refresh(&hiwdg);
#endif
	  
#if MY_CFG_LCD_ENABLE
	  if ( HAL_GPIO_ReadPin(Clear_GPIO_Port, Clear_Pin) == 0
#  if MY_CFG_ENCODER_ENABLE   
	       || (enc_get_sw(&enc, 1) != esw1)
#  endif
		  ) {
		  if ( clear_pin == 0 ) {
			  //HAL_StatusTypeDef st;
			  switch_state |= 0x11; // 0x12 for (-) sw.
			  clear_pin = 1;
			  /*
			    st = HAL_ADC_Start (&hadc1);
			    if ( st == HAL_OK ) {
			    st = HAL_ADC_PollForConversion (&hadc1, 2);
			    #if MY_CFG_UPRINTF_ENABLE
			    if ( st == HAL_OK ) {
			    uint32_t val;
			    val = HAL_ADC_GetValue (&hadc1);
			    uprintf(&huart1,"# [%ld] %ld\r\n",
			    sec_ticks, val);
			    }
			    #endif
			    }
			    HAL_ADC_Stop (&hadc1);
			  */
#  if MY_CFG_ENCODER_ENABLE   
			  esw1 = enc_get_sw(&enc, 1);
#  endif

		  }
	  } else {
		  clear_pin = 0;
	  }
#endif
	  if ( !(state & ST_ERR) ) {
		  leds_run();
#if MY_CFG_LCD_ENABLE
		  if ( switch_state & 0x20 ) {
			  switch_state &= ~0x20;
			  my_lcd_set_time_out(my_conf->lcd_to);
		  }
#endif		  
	  } else {
		  LED1_ON;
		  LED0_ON;		  
#if MY_CFG_LCD_ENABLE
		  if ( !(switch_state & 0x20) ) {
			  switch_state = (0x20|0x04);
			  my_lcd_set_time_out(0);
			  lcd_light_set(1);
		  }
#endif		  
	  }
	  if ( sec_ticks == 3 ) {
		  state |= ST_ADQ_ENABLE;
	  }
	  if ( sec_ticks > 8 ) { /* W5500 need ~9 secs to start */
		  main_ip_client_loop();
	  }
#if MY_CFG_NTP_ENABLE
	  if( (state & ST_IP_ASSIGNED)  && 
	      (!(state & (ST_NTP_SET | ST_NTP_FAIL) ) ||
	       (state & ST_NTP_GET) )     	           ) {
		  main_ntp_loop();
	  }
	  if ( day_ticks != day_now ) {
		  day_now = day_ticks;
		  if ( MNC_NTP_ENABLE() ) state &= ~ST_NTP_SET;
	  }
#endif	       
	  /* ALL: */
	  
	  /* ModBus digital registers: */
#if MY_CFG_MODBUS_ENABLE
	  dout_register = 0;
	  if ( HAL_GPIO_ReadPin(DOOR_SW_GPIO_Port, DOOR_SW_Pin) == 1 )
			dout_register |= 0x01;
	  if ( HAL_GPIO_ReadPin(Clear_GPIO_Port, Clear_Pin) == 0 )
			dout_register |= 0x0100;


	  /*
	    coil_val[0] = ~ (coil_register);
	    coil_val[1] = ~ (coil_register >> 8);
	    HAL_SPI_Transmit(&hspi2, coil_val, 1, 1);
	    HAL_GPIO_WritePin(LCD_LATCH_GPIO_Port, LCD_LATCH_Pin, GPIO_PIN_RESET);
	    HAL_GPIO_WritePin(LCD_LATCH_GPIO_Port, LCD_LATCH_Pin, GPIO_PIN_SET);
	    //HAL_GPIO_WritePin(HC595_LATCH_GPIO_Port, HC595_LATCH_Pin, GPIO_PIN_RESET);
	    //HAL_GPIO_WritePin(HC595_LATCH_GPIO_Port, HC595_LATCH_Pin, GPIO_PIN_SET);
	    */

	  
#endif

	  /* Door monitor: */
	  if ( HAL_GPIO_ReadPin(DOOR_SW_GPIO_Port, DOOR_SW_Pin) == 1
	       //HAL_GPIO_ReadPin(Clear_GPIO_Port, Clear_Pin) == 0
		  ) {
		  if ( door_sw_open != 1) {
			  /* Door open */
			  if ( door_sw_open == 0 ) {
				  door_sw_open_time = my_time(NULL);
			  }
			  door_sw_open = 1;
			  if (DOOR_BUZZER_ENABLE) BUZZER_ON;
		  }
	  } else if (door_sw_open != 0) {
		  if ( door_sw_open == 1 ) {
			  door_sw_close_time = my_time(NULL);
		  }
		  door_sw_open = 0;
		  /*if (DOOR_BUZZER_ENABLE)*/ BUZZER_OFF;
	  }

	  /* TCP servers loops: */
	  if ( state & ST_IP_ASSIGNED ) {
#if MY_CFG_MODBUS_ENABLE
		  mb_run(TCP0_SOCKET, my_conf->mbport);
		  mb_run(TCP7_SOCKET, my_conf->mbport);
#endif
#if MY_CFG_MENUCFG_ENABLE
		  my_menu_p500_run(TCP1_SOCKET, my_conf->cfgport);
		  my_menu_p500_run(TCP2_SOCKET, my_conf->cfgport);
#endif
	  }

	  /* Thermometers loop: */
#if MY_CFG_18B20_ENABLE
	  ret = -1;
	  if ( state & ST_ADQ_ENABLE ) {
		  ret = my_18b20_loop();
	  } else if ( state & ST_TH_SCAN ) {
		  my_18b20_scan();
		  cicle_break = 1;
	  }
#endif
	  
	  /* LCD loop: */
#if MY_CFG_LCD_ENABLE
	  if ( sec_ticks == 14 ) {
		  /* switch to TIME in LCD: */
		  my_lcd_run(0x08);
	  } else {
		  my_lcd_run(switch_state & 0x07);
		  switch_state &= ~0x07;
	  }
#endif
	  
	  /* Barometer loop: */
#if MY_CFG_BAROMETER_ENABLE
	  if ( baro_loop(barometer) ) {
		  pressure = (uint32_t) BMP180_GET_PRESSURE(barometer);
	  }
#endif
	  /* DHT22 (Hydrometer) loop: */
#if MY_CFG_DHT_ENABLE
	  if ( DHT_read_data(&dht22) == 0 ) {
		  dht_humidity = dht22.humidity;
		  dht_temperature = dht22.temperature;
	  } else if (dht22.data_valid == 0 && dht_humidity) {
		  dht_humidity = 0;
		  dht_temperature = -85;
		  state |= ST_DHT_ERR;
	  }
#endif
	  /* CHK main loop integrity: */
	  cicle_delta = HAL_GetTick() - tickstart;
	  if ( cicle_delta > wait ) {
		  if ( cicle_break ) {
			  cicle_break = 0;
		  } else {
			  state |= ST_LOOP_ERR;
			  uprintf(&huart1,
				  "# [%ld] Error: cicle time: %ld ms\r\n",
				  sec_ticks, cicle_delta);
		  }
	  }
	  while ((HAL_GetTick() - tickstart) < wait);

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

#if MY_CFG_DEBUG
/*
int __io_putchar(int ch)
{
	ITM_SendChar(ch);
	return(ch);
}

int _write(int file, char *ptr, int len)
{
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		__io_putchar(*ptr++);
	}
	return len;
}
*/

int _write(int file, char *ptr, int len)
{
	/*
	  http://eeblog.co.uk/2018/11/29/swo-with-cubemx-using-st-link-clones/
	*/
#if (MY_CFG_DEBUG == 2)
	int DataIdx;
	
	for(DataIdx=0; DataIdx<len; DataIdx++)
	{
		ITM_SendChar(*ptr++);
	}
#else
	static char buff[80];
	if ( len > 80 ) len = 80;
	while( huart1.gState != HAL_UART_STATE_READY);
	memcpy(buff,ptr,len);
	HAL_UART_Transmit_DMA(&huart1, (uint8_t *) buff, len);
	
#endif
	return len;
}
#endif


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *uarth){
#if MY_CFG_18B20_ENABLE
	if (uarth->Instance == OneWire_UART_Instance() /*USART3*/){
		OneWire_TxCpltCallback();
	}
#endif
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uarth){
#if MY_CFG_18B20_ENABLE
	if (uarth->Instance == OneWire_UART_Instance() /*USART3*/){
			OneWire_RxCpltCallback();
		}
#endif
}




/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
