/** 
 *   @file my_ds18b20.h
 *   @brief Acquisition of 18b20 thermometer set data.
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 29/02/2020
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
 *   29/02/2020: Created.
 **/

 /*
    QT:
    0x28, 0xff, 0x77, 0x67, 0x50, 0x18, 0x83, 0xed, 
    0x28, 0xff, 0x93, 0x1e, 0x51, 0x18, 0x81, 0x04, 
    0x28, 0xff, 0x03, 0x22, 0x51, 0x18, 0x81, 0xe9, 
    0x28, 0xff, 0x40, 0x68, 0x50, 0x18, 0x83, 0xa2, 
    0x28, 0xff, 0xdf, 0x6c, 0x50, 0x18, 0x83, 0x74,
    DS:
    0x28, 0xa8, 0x9c, 0xb3, 0x05, 0x00, 0x00, 0x59,

    Sala Relojes:
    Old:
    Current Table:                                              offset
    ID: [00][00] 0x28, 0x9b, 0xf5, 0x79, 0x97, 0x18, 0x03, 0x32, 26
    ID: [00][01] 0x28, 0x66, 0xa1, 0x79, 0x97, 0x19, 0x03, 0x44,  6 
    ID: [00][02] 0x28, 0xfb, 0x95, 0x75, 0xd0, 0x01, 0x3c, 0x18, -34
    ID: [00][03] 0x28, 0xf9, 0xb4, 0x79, 0x97, 0x18, 0x03, 0xbb, -73
    ID: [00][04] 0x28, 0xde, 0x58, 0x79, 0x97, 0x16, 0x03, 0xaf, -4 
    ID: [00][05] 0x28, 0x09, 0x41, 0x79, 0x97, 0x18, 0x03, 0xbd, -8 
    ID: [00][06] 0x28, 0xee, 0xd9, 0x79, 0x97, 0x17, 0x03, 0x80, 32 
    ID: [00][07] 0x28, 0xc7, 0x7f, 0x46, 0x92, 0x02, 0x02, 0xa5, 6  
    ID: [00][08] 0x28, 0xdb, 0xcb, 0x46, 0x92, 0x10, 0x02, 0xae, -93
    ID: [00][09] 0x28, 0x56, 0xef, 0x46, 0x92, 0x0d, 0x02, 0x8c, 84 
    ID: [00][10] 0x28, 0xba, 0xc7, 0x75, 0xd0, 0x01, 0x3c, 0x4c, -17
    ID: [00][11] 0x28, 0x42, 0x6d, 0x79, 0x97, 0x19, 0x03, 0x2d, 12 
    ID: [00][12] 0x28, 0x1c, 0x4e, 0x46, 0x92, 0x0a, 0x02, 0xa6, -1 
    ID: [00][13] 0x28, 0xe9, 0xa9, 0x79, 0x97, 0x18, 0x03, 0x70, 66 
    ID: [00][14] 0x28, 0xc7, 0x1a, 0x79, 0x97, 0x18, 0x03, 0xd4, -21
    ID: [00][15] 0x28, 0xb7, 0x13, 0x75, 0xd0, 0x01, 0x3c, 0x47, -15
    ID: [00][16] 0x28, 0x1a, 0x23, 0x79, 0x97, 0x18, 0x03, 0x2d, 0  
    ID: [00][17] 0x28, 0x08, 0x7e, 0x07, 0xd6, 0x01, 0x3c, 0xe7, 42 
    ID: [00][18] 0x28, 0x4a, 0x43, 0x79, 0x97, 0x19, 0x03, 0xd6, 35 
    ID: [00][19] 0x28, 0xfd, 0xf4, 0x79, 0x97, 0x17, 0x03, 0x96, -46 

    New:
    ID: [01][00] 0x28, 0x9b, 0xf5, 0x79, 0x97, 0x18, 0x03, 0x32, offset:  26
    ID: [01][01] 0x28, 0x66, 0xa1, 0x79, 0x97, 0x19, 0x03, 0x44, offset:   5
    ID: [01][02] 0x28, 0xfb, 0x95, 0x75, 0xd0, 0x01, 0x3c, 0x18, offset: -40
    ID: [01][03] 0x28, 0xf9, 0xb4, 0x79, 0x97, 0x18, 0x03, 0xbb, offset: -72
    ID: [01][04] 0x28, 0xde, 0x58, 0x79, 0x97, 0x16, 0x03, 0xaf, offset:  -4
    ID: [01][05] 0x28, 0x09, 0x41, 0x79, 0x97, 0x18, 0x03, 0xbd, offset:  -2
    ID: [01][06] 0x28, 0xee, 0xd9, 0x79, 0x97, 0x17, 0x03, 0x80, offset:  31
  X ID: [01][07] 0x28, 0xc7, 0x7f, 0x46, 0x92, 0x02, 0x02, 0xa5, offset:  -1
    ID: [01][08] 0x28, 0xdb, 0xcb, 0x46, 0x92, 0x10, 0x02, 0xae, offset: -96
    ID: [01][09] 0x28, 0x56, 0xef, 0x46, 0x92, 0x0d, 0x02, 0x8c, offset:  78
    ID: [01][10] 0x28, 0xba, 0xc7, 0x75, 0xd0, 0x01, 0x3c, 0x4c, offset: -14
    ID: [01][11] 0x28, 0x42, 0x6d, 0x79, 0x97, 0x19, 0x03, 0x2d, offset:  13
    ID: [01][12] 0x28, 0x1c, 0x4e, 0x46, 0x92, 0x0a, 0x02, 0xa6, offset:  -9
    ID: [01][13] 0x28, 0xe9, 0xa9, 0x79, 0x97, 0x18, 0x03, 0x70, offset:  70
    ID: [01][14] 0x28, 0xc7, 0x1a, 0x79, 0x97, 0x18, 0x03, 0xd4, offset: -26
    ID: [01][15] 0x28, 0xb7, 0x13, 0x75, 0xd0, 0x01, 0x3c, 0x47, offset: -18
    ID: [01][16] 0x28, 0x1a, 0x23, 0x79, 0x97, 0x18, 0x03, 0x2d, offset:   0
    ID: [01][17] 0x28, 0x08, 0x7e, 0x07, 0xd6, 0x01, 0x3c, 0xe7, offset:  39
  X ID: [01][18] 0x28, 0x4a, 0x43, 0x79, 0x97, 0x19, 0x03, 0xd6, offset:  36
 07 ID: [01][19] 0x28, 0xfd, 0xf4, 0x79, 0x97, 0x17, 0x03, 0x96, offset: -45

ID 07 no funciona, ID 18 quedo sin uso.

  */

#include "main.h"

#include <stdio.h>
#include <string.h>
#include "my_config.h"
#include "my_ds18b20.h"
#include "ds18b20.h"
#include "my_rtc_time.h"
#include "my_menu.h"
#if MY_CFG_UPRINTF_ENABLE
#  include "huart_tools.h"
#  include "usart.h"
#endif

#if MY_CFG_18B20_ENABLE

#define CICLE2CONVERT 188 //188

static THER18B20DATA thd[THERM_NUM + 1];
static uint8_t idt[THERM_NUM];
static int number_off_thermometers = 0;

#if 1
static THER18B20ROM th;
#elif 0 
static THER18B20ROM th = {
	/* Alarm Table: */
	{
		/* up */ {28, 30, 40, 50, 60, 70, 80, 90, 100, 0, },
		/* low*/ {0,},
	},
	/* Thermometer ROMs: */
	{
		/* 0 */ {0x28, 0xff, 0x77, 0x67, 0x50, 0x18, 0x83, 0xed},
		/* 1 */ {0x28, 0xff, 0x93, 0x1e, 0x51, 0x18, 0x81, 0x04},
		/* 2 */ {0x28, 0xff, 0x03, 0x22, 0x51, 0x18, 0x81, 0xe9},
		/* 3 */ {0x28, 0xff, 0x40, 0x68, 0x50, 0x18, 0x83, 0xa2}, 
		/* 4 */ {0x28, 0xff, 0xdf, 0x6c, 0x50, 0x18, 0x83, 0x74},
		/* 5 */ {0x28, 0xa8, 0x9c, 0xb3, 0x05, 0x00, 0x00, 0x59},
		/* 6 */ {0xff,}, /* free slot */
		/* 7 */ {0,},    /* last element */
	},
		
	/* Thermometer alarms: */
	//{1,2,3,4,5,0x88,-50,127,0xaa,-100,0x80,}
};
#else

static THER18B20ROM th = {
	/* Alarm Table: */
	{
		/* up */ {28, 30, 40, 50, 60, 70, 80, 90, 100, 0, },
		/* low*/ {0,},
	},
	/* Thermometer ROMs: */
	{
		/* 00 */ {0x28, 0x9b, 0xf5, 0x79, 0x97, 0x18, 0x03, 0x32},
		/* 01 */ {0x28, 0x66, 0xa1, 0x79, 0x97, 0x19, 0x03, 0x44},
		/* 02 */ {0x28, 0xfb, 0x95, 0x75, 0xd0, 0x01, 0x3c, 0x18},
		/* 03 */ {0x28, 0xf9, 0xb4, 0x79, 0x97, 0x18, 0x03, 0xbb},
		/* 04 */ {0x28, 0xde, 0x58, 0x79, 0x97, 0x16, 0x03, 0xaf},
		/* 05 */ {0x28, 0x09, 0x41, 0x79, 0x97, 0x18, 0x03, 0xbd},
		/* 06 */ {0x28, 0xee, 0xd9, 0x79, 0x97, 0x17, 0x03, 0x80},
		/* 07 */ {0x28, 0xc7, 0x7f, 0x46, 0x92, 0x02, 0x02, 0xa5},
		/* 08 */ {0x28, 0xdb, 0xcb, 0x46, 0x92, 0x10, 0x02, 0xae},
		/* 09 */ {0x28, 0x56, 0xef, 0x46, 0x92, 0x0d, 0x02, 0x8c},
		/* 10 */ {0x28, 0xba, 0xc7, 0x75, 0xd0, 0x01, 0x3c, 0x4c},
		/* 11 */ {0x28, 0x42, 0x6d, 0x79, 0x97, 0x19, 0x03, 0x2d},
		/* 12 */ {0x28, 0x1c, 0x4e, 0x46, 0x92, 0x0a, 0x02, 0xa6},
		/* 13 */ {0x28, 0xe9, 0xa9, 0x79, 0x97, 0x18, 0x03, 0x70},
		/* 14 */ {0x28, 0xc7, 0x1a, 0x79, 0x97, 0x18, 0x03, 0xd4},
		/* 15 */ {0x28, 0xb7, 0x13, 0x75, 0xd0, 0x01, 0x3c, 0x47},
		/* 16 */ {0x28, 0x1a, 0x23, 0x79, 0x97, 0x18, 0x03, 0x2d},
		/* 17 */ {0x28, 0x08, 0x7e, 0x07, 0xd6, 0x01, 0x3c, 0xe7},
		/* 18 */ {0x28, 0x4a, 0x43, 0x79, 0x97, 0x19, 0x03, 0xd6},
		/* 19 */ {0x28, 0xfd, 0xf4, 0x79, 0x97, 0x17, 0x03, 0x96},
		/* 20 */ {0,},    /* last element */
	},
	/* Offsets: */
	{
		/* 00 */ 26,
		/* 01 */ 6, 
		/* 02 */ -34, 
		/* 03 */ -73, 
		/* 04 */ -4, 
		/* 05 */ -8, 
		/* 06 */ 32, 
		/* 07 */ 6, 
		/* 08 */ -93, 
		/* 09 */ 84, 
		/* 10 */ -17, 
		/* 11 */ 12, 
		/* 12 */ -1, 
		/* 13 */ 66, 
		/* 14 */ -21, 
		/* 15 */ -15, 
		/* 16 */ 0, 
		/* 17 */ 42, 
		/* 18 */ 35, 
		/* 19 */ -46,
		/* 20 */ 0,
	},
};

#endif


static uint8_t FunctionBuffer[9]; /* 9 */
static int thc = 0; /* thermometer counter */
static UART_HandleTypeDef *_con;
//static int error_18b20 = 0;

#if 0
void OnComplete(void)
{
	//printf("complete\r\n");
	//error_18b20 = 0;
	return;
}
#endif


void OnErr(void)
{
	//error_18b20 = 1;
	printf("18b20 error\r\n");
	return;
}


void my_18b20_init(UART_HandleTypeDef *huart)
{
	_con = huart;
	OneWire_Init(huart);
	OneWire_SetCallback(NULL/*OnComplete*/, OnErr );
	memset(thd, 0, sizeof(THER18B20DATA) * (THERM_NUM + 1) );
	for (int i = 0; i < THERM_NUM; i++) {
		thd[i].max = -5500;
		thd[i].min = 12500;
	}
}


void my_18b20_make_index_table(void)
{
	int t = 0;
	int id;

	memset(idt, 0, THERM_NUM );	
	for (id = 0; id < THERM_NUM && th.rom[id][0]; id++) {
		if ( th.rom[id][0] != 0xff ) {
			idt[t] = id;
			t++;
		}
	}
	number_off_thermometers = t;
	
	return;
}


int my_18b20_get_number(void)
{
	return number_off_thermometers;
}

uint8_t *my_18b20_get_id_table(void)
{
	return idt;
}



THER18B20DATA *my_18b20_get_data_p(void)
{
	return thd;
}


THER18B20ROM *my_18b20_get_rom_p(void)
{
	return &th;
}


static inline int if_id_ther(int id)
{
	return (th.rom[id][0] == 0x28 ? 1 : 0 );
}


static uint8_t crc8( uint8_t *addr, uint8_t len)
{
	uint8_t crc = 0;

	while (len--) {
		uint8_t inbyte = *addr++;
		for (uint8_t i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8c;
			inbyte >>= 1;
		}
	}

	return crc;
}


#if ! THERM_CALC_FLOAT
/* awk 'BEGIN{for(i=0;i<16;i++)printf "%2.0f,", 6.25*i;}' */
static const int dec[] = {0, 6,12,19,25,31,38,44,50,56,62,69,75,81,88,94};

static int16_t my_ds18b20_buff2int(uint8_t *buff)
{
	int16_t temp;

	temp = (*(buff) >> 4 | *(buff+1) << 4) & 0x7f;
	temp *= 100;
	temp += dec[*(buff) & 0x0f];
	if ( (*(buff+1) >> 3) != 0 ) { /* negative */
		temp -= 12800;
	}

	return temp;
}
#endif


static int my_18b20_buff_proc(THER18B20DATA *th, uint8_t *buff, int8_t offset)
{
	th->valid = 0;
	
	if ( crc8(buff, 8) == buff[8] && buff[4] ) {
		if ( ! th->valid ) {  /* 0x0550 (85.0C) */
			if ( !(*(buff+1) == 0x05 && *(buff) == 0x50) )
				th->valid = 1;
		}
	} else {
		th->valid = 0;
	}
	if( th->valid ) {
#if THERM_CALC_FLOAT
		th->value = (*(buff) >> 4 | *(buff+1) << 4) & 0x7f;
		th->value +=  (float) ((*(buff) & 0x0f) * 625) / 10000.0;
		if ( (*(buff+1) >> 3) != 0 ) { /* negative */
			th->value -= 128.0;
		}
		th->value += (float) offset / 100.0;
		if (th->value_avg == 0
		    || (th->value_avg - th->value > 0.3)
		    || (th->value_avg - th->value < -0.3) )
			th->value_avg = th->value;
		th->value_avg = th->value_avg * 0.95 + th->value * 0.05;
		if ( th->value_avg > th->max ) th->max = th->value_avg;
		if ( th->value_avg < th->min ) th->min = th->value_avg;
#else
		th->value_raw[0] = buff[0];
		th->value_raw[1] = buff[1];
		if ( (*(buff+1) >> 3) != 0 ) { /* negative */ /* FIXME: TBC */
			th->val = my_ds18b20_buff2int(th->value_raw) - offset;
		} else {
			th->val = my_ds18b20_buff2int(th->value_raw) + offset;
		}
		if ( th->val > th->max ) th->max = th->val;
		if ( th->val < th->min ) th->min = th->val;
#endif
	}

	return th->valid;
}


/**
  * @brief Main 18b20 adq. loop. call every 4 ms.
  * @retval 1: New data set complet, 0: otherwise.
  */
int my_18b20_loop(void) 
{
	static uint32_t ther_cicle = 0;
	static uint32_t next_read = CICLE2CONVERT;
	static int _err = 0;
#if MY_CFG_UPRINTF_ENABLE
	static int _err_count = 0;
#endif
	int complete = 0;


	if ( _err ){
		for(int i = 0; i < THERM_NUM; i++) {
			thd[i].valid = 0;
		}
		ther_cicle = 0;
		_err = 0;
	}
	
	if ( ther_cicle == 0 ) {
		thc = 0;
		adq_time = my_time(NULL);
		/* start to Convert T: */
		OneWire_Execute(0xcc, NULL, 0x44, NULL);
	} else if ( ther_cicle == CICLE2CONVERT ) {
		next_read = CICLE2CONVERT;
		while ( !if_id_ther(thc) && thc < THERM_NUM ) thc++;
		if ( if_id_ther(thc) ){
			//LED0_ON;
			OW_adq(th.rom[thc], FunctionBuffer);
		}
	} else if ( ther_cicle == (next_read + 4) ) { //4
		if ( if_id_ther(thc) && thc < THERM_NUM ) {
			if ( !OneWire_complete() ) {
				state |= ST_OW_ERR;
				_err = 1;
#if MY_CFG_UPRINTF_ENABLE
				uprintf(&huart1,
					"# [%ld] OneWire Fail, count: %d\r\n",
					sec_ticks, ++_err_count);
#endif
			} else {
				my_18b20_buff_proc(&thd[thc], FunctionBuffer, th.offset[thc]);
				next_read = ther_cicle;
				thc++;
				while ( !if_id_ther(thc) && thc < THERM_NUM )
					thc++;
				if ( if_id_ther(thc) ) {
					OW_adq(th.rom[thc], FunctionBuffer);
					
				} else 
					complete = 1;
			}
		}
	}
	
	ther_cicle++;
	if (ther_cicle == THERM_END_CICLE ) ther_cicle = 0;

	
	return complete;
}


void my_18b20_scan(void)
{
	//static uint32_t cicle = 0;
	static uint8_t ROMBuffer[8]= {0,};
	int count = 0;
	static int rom_count = 0;
	static uint64_t *romp = (uint64_t *) ROMBuffer;
	static uint64_t	rom_old = 0L;
	static uint64_t *th_rom;
	int id;

	//if ( cicle++ < 10 ) return;
	//cicle = 0;

	OneWire_Execute(0x33, ROMBuffer, 0, NULL);
	//LED0_ON;
	while(!OneWire_complete() && count++ < 25000);

	//LED0_OFF;
	if ( count < 25000 && crc8(ROMBuffer, 7) == ROMBuffer[7] ) {
		if ( *romp != rom_old ) {
			rom_count = 0;
			rom_old = *romp;
		}
		if ( rom_count == 10 ) {
			int len;
			int add = -1;
			int find = 0;
			for (id = 0; id < THERM_NUM && th.rom[id][0]; id++) {
				th_rom =  (uint64_t *) th.rom[id];
				if ( *th_rom == *romp ) {
					find = 1;
					break;
				}
			}
			if ( !find && id <THERM_NUM ) {
				add = id;
				for (id = 0; id < THERM_NUM &&
					     th.rom[id][0] != 0xff ; id++);
				if ( id > add ) id = add;
				th.offset[id] = 0;
				th_rom =  (uint64_t *) th.rom[id];
				*th_rom = *romp;
			}
			len = sprintf(msg,"\rID: [%02d] ", id);
			for(int i=0;i<8;i++)
				len+=sprintf(msg+len,"0x%02x, ", ROMBuffer[i] );
			sprintf(msg+len,"\r\n");
			//send(sn, (uint8_t*)msg, strlen(msg) );
			my_menu_send_to_client((uint8_t*)msg, strlen(msg) );
			
		} 
		rom_count++;

	} else {
		rom_old = 0L;
		HAL_Delay(500);
		
	}
	
	return;
}



#if 0
void my_18b20_scan(uint16_t sn)
{
	//static uint32_t cicle = 0;
	static uint8_t ROMBuffer[8]= {0,};
	int count = 0;
	static int rom_count = 0;
	static uint64_t *romp = (uint64_t *) ROMBuffer;
	static uint64_t	rom_old = 0L;
	static uint64_t *th_rom;
	int id;

	//if ( cicle++ < 10 ) return;
	//cicle = 0;

	OneWire_Execute(0x33, ROMBuffer, 0, NULL);
	//LED0_ON;
	while(!OneWire_complete() && count++ < 25000);
	//LED0_OFF;
	if ( count < 25000 && crc8(ROMBuffer, 7) == ROMBuffer[7] ) {
		if ( *romp != rom_old ) {
			rom_count = 0;
			rom_old = *romp;
		}
		if ( rom_count == 10 ) {
			int len;
			for (id = 0; id < THERM_NUM && th.rom[id][0]; id++) {
				th_rom =  (uint64_t *) th.rom[id];
				if ( *th_rom == *romp ) {
					break;
					  
				}
			}
			if ( th.rom[id][0] == 0 && id < THERM_NUM ) {
				th_rom =  (uint64_t *) th.rom[id];
				*th_rom = *romp;
			} else if ( th.rom[id][0] == 0xff && id <= THERM_NUM ) {
				for (id = 0; id < THERM_NUM && th.rom[id][0] != 0xff; id++) {
					th_rom =  (uint64_t *) th.rom[id];
					if ( *th_rom == *romp ) {
						break;
						
					}
					
				}
				if ( th.rom[id][0] == 0xff && id < THERM_NUM ) {
					th_rom =  (uint64_t *) th.rom[id];
					*th_rom = *romp;
				}
			}
			len = sprintf(msg,"\rID: [%02d] ", id);
			for(int i=0;i<8;i++)
				len+=sprintf(msg+len,"0x%02x, ", ROMBuffer[i] );
			sprintf(msg+len,"\r\n");
			send(sn, (uint8_t*)msg, strlen(msg) );
		} 
		rom_count++;

	} else {
		rom_old = 0L;
		HAL_Delay(500);
		
	}
	
	return;
}
#endif 


/****************************************************************************

      SCRATCH:
*/

#if 0
  int j;
  uint8_t *ap;
  uint64_t *rom;
  printf("\r\n");
  for(j=0;j<15;j++) {
	  rom = (uint64_t *) th.rom[j];
	  printf("%2d: %08lx%08lx ", j, (uint32_t) (*rom>>32), (uint32_t) *rom);
	  //for(i=0;i<8;i++)
	  //	  printf("0x%02x, ", th.rom[j][i] );
	  ap = (uint8_t *) &th.a[j];
	  th.a[j].id = j;
	  th.a[j].flag = 1;
	  printf("%d [0x%02x]\r\n", (uint8_t) th.a[j].id,
		 *ap );
  }
  printf("size: %d, round32: %d\r\n", sizeof(THER18B20ROM),
	 (sizeof(THER18B20ROM)+3) >> 2 );

  for(j=0;j<15;j++) {
	  printf("%2d: up: %d\r\n", j, th.t.up[j]);
  }
  while (1);
#endif


#endif /* MY_CFG_18B20_ENABLE */
