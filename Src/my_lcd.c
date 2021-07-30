/** 
 *   @file my_lcd.c
 *   @brief  lcd 16x2 implementation.
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 24/09/2020
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


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "my_config.h"
#if MY_CFG_LCD_ENABLE
#include "my_rtc_time.h"
#include "my_ds18b20.h"
#include "lcd.h"

#if MY_CFG_ENCODER_ENABLE		  
#include "encoder_sw.h"
#endif

enum { /* LCD Page: */
	LCDP_MAC = 0,
	LCDP_IP,
	LCDP_TIME,
#if MY_CFG_18B20_ENABLE
#if   ! MY_CFG_ENCODER_ENABLE
	LCDP_TEMP0,
	LCDP_TEMP1,
#  endif
#  if MY_CFG_ENCODER_ENABLE
	//LCDP_ENC_DEMO,
	LCDP_LIST_TEMP,
#  endif
	LCDP_EXTREME,
#endif
#if MY_CFG_BAROMETER_ENABLE
	LCDP_PRESS,
#endif
#if MY_CFG_DHT_ENABLE
	LCDP_HUMIDITY,
#endif
	LCDP_ST,
	LCDP_NUM /* it is not a page. */
};


#if MY_CFG_18B20_ENABLE || MY_CFG_BAROMETER_ENABLE || MY_CFG_DHT_ENABLE
static uint8_t int_to_str(uint8_t *str, uint16_t val, uint8_t digits)
{
	int i = 0;

	if ( digits >= 6 ) str[i++] = (val % 1000000) / 100000 + 0x30;
	if ( digits >= 5 ) str[i++] = (val % 100000)  / 10000  + 0x30;
	if ( digits >= 4 ) str[i++] = (val % 10000)   / 1000   + 0x30;
	if ( digits >= 3 ) str[i++] = (val % 1000)    / 100    + 0x30;
	if ( digits >= 2 ) str[i++] = (val % 100)     / 10     + 0x30;
	str[i++] = (val % 10)                                  + 0x30;

	return i;
}

	
static uint8_t float_to_str(uint8_t *str, float val, uint8_t width, uint8_t point)
{
	uint8_t len = int_to_str(str, (uint16_t) val, width -1 -point);
	if ( point ) {
		float power[] = {1.,10.,100.,1000.,10000.,100000};
		uint16_t decimal = (val * power[point]) -
			((uint16_t) val) * power[point];
		str[len++] = '.';
		len += int_to_str(str+len, decimal, point);
	}

	return len;
}
#endif


#if MY_CFG_18B20_ENABLE
static int _calc_extremes(int *M, int *m)
{
	int id;
	float max=-85.0,min=180.0;
	THER18B20DATA *thd = my_18b20_get_data_p();
	int num = my_18b20_get_number();
	uint8_t *idt =  my_18b20_get_id_table();
	
	for (id = 0; id < num; id++) {
		if (thd[idt[id]].valid ) {
			float v = thd[idt[id]].value_avg;
			if ( v > max ) {
				*M = id;
				max = v;
			}
			if (v < min) {
				*m = id;
				min = v;
			}
		}
	}
	
	return 0;
}
#endif


static int my_lcd_page(int page)
{
	static int wcount = 0;
	static int chcount;// = 0;
	static char chpoint;// = 0xa5;
	static int lcd_step = 0;
	static int pege_curr = -1;
	static char lcd_buff[17]={0};
	static datetime *dt;
#if MY_CFG_18B20_ENABLE || MY_CFG_BAROMETER_ENABLE || MY_CFG_DHT_ENABLE
	int len;
#endif
#if MY_CFG_18B20_ENABLE
	THER18B20DATA *thd = my_18b20_get_data_p();
	uint8_t *idt =  my_18b20_get_id_table();
	static int M = 0, m = 0;
#  if MY_CFG_ENCODER_ENABLE
	static int cur = 0;
	static int num_th = 0;
#  else
	int i, ofs;
#  endif
#endif	
	if ( pege_curr != page ) {
		lcd_step = 0;
		pege_curr = page;
	}

	switch(page) {
	case LCDP_MAC:
		switch(lcd_step) {
		case 0: lcd_clear();
			break;
		case 1:	lcd_send_string("MAC:");
			break;
		case 2:	sprintf(lcd_buff, "%02x%02x%02x%02x%02x%02x",
				net_info_ee->mac[0], net_info_ee->mac[1],
				net_info_ee->mac[2], net_info_ee->mac[3],
				net_info_ee->mac[4], net_info_ee->mac[5] );
			lcd_send_string(lcd_buff);
			break;
		case 3:
			lcd_put_cur(1, 2);
			chcount = 0;
			chpoint = 0xa5;
			break;
		case 4:
			lcd_send_string(">");
			break;
		case 5:
			lcd_put_cur(1, 13);
			break;
		case 6:
			lcd_send_string("<");
			break;
		case 7:
			lcd_put_cur(1, 3);
			break;
		case 8:
			if ( sec_ticks > wcount || (wcount == -1)) {
				lcd_send_string(&chpoint);
				wcount = sec_ticks;
				chcount++;
			}
			if ( chcount == 10 ) {
				chpoint = '*';
				lcd_step = 6;
			} else if ( chcount == 20 ) {
				chpoint = 0xa5;
				lcd_step = 6;
				chcount = 0;
			} else 
				lcd_step = 7;
			break;
		default:
			lcd_step = -1;
			break;
		}
		break;
	case LCDP_IP:
		switch(lcd_step) {
		case 0:	lcd_clear();
			break;
		case 1:
			lcd_put_cur(0, 0);
			break;
		case 2:
			sprintf(lcd_buff,"UID: %02x - IP:", MNC_GET_ID());
			lcd_send_string(lcd_buff);
			break;
		case 3: lcd_put_cur(1, 1);
			break;
		case 4:	sprintf(lcd_buff,"%d.%d.%d.%d",
				net_info_ee->ip[0], net_info_ee->ip[1],
				net_info_ee->ip[2], net_info_ee->ip[3] );	
			lcd_send_string(lcd_buff);
			break;
		default:
			lcd_step = -1;
			break;
		}
		break;
#if MY_CFG_18B20_ENABLE
#  if ! MY_CFG_ENCODER_ENABLE
	case LCDP_TEMP0:
	case LCDP_TEMP1:
		ofs = (page - LCDP_TEMP0) << 2;
		switch(lcd_step) {
		case 0:
			lcd_clear();
			break;	
		case 1:
			lcd_put_cur(0, 0);
			break;
		case 2:			
			len = 0;
			for(i=ofs;i<2+ofs;i++) {
				if (thd[idt[i]].valid ) {
					/*
					len += sprintf(lcd_buff+len,"T%02d ", idt[i]);
					len += float_to_str((uint8_t *)lcd_buff+len,
							    thd[idt[i]].value_avg,
							    6,2);
					*((uint8_t *)lcd_buff+len) = 0;
					*/
					
					len += sprintf(
						lcd_buff+len,"T%d%6ld", idt[i],
						(int32_t) (thd[idt[i]].value_avg * 100.));
					
				} else {
					len += sprintf(lcd_buff+len,
						       "T%d  null",idt[i]);
				}
			} 
			lcd_send_string(lcd_buff);
			break;
		case 3:	lcd_put_cur(1, 0);
			break;
		case 4:
			len = 0;
			for(i=2+ofs;i<4+ofs;i++) {
				if (thd[idt[i]].valid ) {
					/*
					len += sprintf(lcd_buff+len,"T%02d", idt[i]);
					len += float_to_str((uint8_t *)lcd_buff+len,
							    thd[idt[i]].value_avg,
							    6,2);
					*((uint8_t *)lcd_buff+len) = 0;
					*/
					
					len += sprintf(lcd_buff+len,"T%d%6ld",
						       idt[i],
						       (int32_t) (thd[idt[i]].value_avg * 100.));
					
				} else {
					len += sprintf(lcd_buff+len,
						       "T%d  null",idt[i]);
				}
			} 
			lcd_send_string(lcd_buff);
			break;
		default:
			lcd_step = 0;
			break;
		}
		break;
# endif
#endif
	case LCDP_TIME:
		switch(lcd_step) {
		case 0: lcd_clear();
			break;	
		case 1: lcd_send_string("Date: ");
			break;
		case 2: lcd_put_cur(1, 0);
			break;
		case 3: lcd_send_string("Time:");
			break;			
		case 4:	dt = my_localtime(my_time(NULL));
			lcd_put_cur(0, 6);
			break;
		case 5:	sprintf(lcd_buff,"%4d/%02d/%02d",
				dt->yy, dt->mo, dt->dd );
			lcd_send_string(lcd_buff);
			break;
		case 6:	lcd_put_cur(1, 5);
			break;
		case 7: sprintf(lcd_buff,"%02d:%02d:%02d%s",
				dt->hh, dt->mm, dt->ss, my_tz_str());
			lcd_send_string(lcd_buff);
			break;
		default:
			lcd_step = 3;
			break;
		}
		break;
#if MY_CFG_18B20_ENABLE
	case LCDP_EXTREME:
		switch(lcd_step) {
		case 0: lcd_clear();
			break;	
		case 1: lcd_put_cur(0, 0);
			_calc_extremes(&M, &m);
			break;
		case 2:
			len = 0;
			len += sprintf(lcd_buff,"Max%2d ", M);
			len += float_to_str((uint8_t *)lcd_buff+len,
					    thd[M].value_avg, 6,2);
			*((uint8_t *)lcd_buff+len) = 0;
			lcd_send_string(lcd_buff);
			break;
		case 3: lcd_put_cur(1, 0);
			break;
		case 4:
			len = 0;
			len += sprintf(lcd_buff,"Min%2d ", m);
			len += float_to_str((uint8_t *)lcd_buff+len,
					    thd[m].value_avg, 6,2);
			*((uint8_t *)lcd_buff+len) = 0;
			lcd_send_string(lcd_buff);
			break;
		default:
			lcd_step = 0;
			break;
		}	
		break;
#endif
#if MY_CFG_BAROMETER_ENABLE
	case LCDP_PRESS:
		switch(lcd_step) {
		case 0: lcd_clear();
			break;	
		case 1: lcd_put_cur(0, 0);
			break;
		case 2:
			sprintf(lcd_buff,"Pressure [hPa]:");
			lcd_send_string(lcd_buff);
			break;
		case 3: lcd_put_cur(1, 4);
			break;
		case 4:
			if ( pressure ) {
				len = float_to_str((uint8_t *)lcd_buff,
						   (float) pressure/100.0, 7,2);
				*((uint8_t *)lcd_buff+len) = 0;
			} else {
				sprintf(lcd_buff,"-------");	
			}
			lcd_send_string(lcd_buff);
			break;
		default:
			lcd_step = 2;
			break;
		}	
		break;
#endif


#if MY_CFG_DHT_ENABLE
	case LCDP_HUMIDITY:
		switch(lcd_step) {
		case 0: lcd_clear();
			break;	
		case 1: lcd_put_cur(0, 0);
			break;
		case 2:
			sprintf(lcd_buff,"H: --.- %%");
			lcd_send_string(lcd_buff);
			break;

		case 3: lcd_put_cur(1, 0);
			break;
		case 4:
			sprintf(lcd_buff,"T: ---.- C");
			lcd_send_string(lcd_buff);
			break;
		case 5: lcd_put_cur(0, 3);
			break;
		case 6:
			if ( dht_humidity ) {
				len = float_to_str((uint8_t *)lcd_buff,
						   (float) dht_humidity/10.0, 4,1);
				*((uint8_t *)lcd_buff+len) = 0;
			} else {
				sprintf(lcd_buff,"--.-");
			}
			lcd_send_string(lcd_buff);
			break;

		case 7: lcd_put_cur(1, 3);
			break;
		case 8:
			if ( dht_temperature != -85 ) {
				len = float_to_str((uint8_t *)lcd_buff,
						   (float) dht_temperature/10.0, 5,1);
				*((uint8_t *)lcd_buff+len) = 0;

			} else {
				sprintf(lcd_buff,"---.-");
			}
			lcd_send_string(lcd_buff);
			
			break;


			
		default:
			lcd_step = 4;
			break;
		}	
		break;
#endif

		
	case LCDP_ST:
		switch(lcd_step) {
		case 0: lcd_clear();
			break;	
		case 1: lcd_send_string("NTP:");
			break;
		case 2: lcd_put_cur(0, 9);
			break;
		case 3: lcd_send_string("OW:");
			break;		
		case 4: lcd_put_cur(1, 0);
			break;
		case 5: lcd_send_string("LOOP:");
			break;
		case 6: lcd_put_cur(1, 10);
			break;
		case 7: lcd_send_string("ST");
			break;
		case 8:	lcd_put_cur(0, 4);
			break;
		case 9: 
			sprintf(lcd_buff,"%s", (state&ST_IP_ASSIGNED)?
				(state&ST_NTP_FAIL)?"FAIL":"OK  ": "----");
			lcd_send_string(lcd_buff);
			break;
		case 10:lcd_put_cur(0, 12);
			break;
		case 11: 
			sprintf(lcd_buff,"%s",
				(state&ST_OW_ERR)?"FAIL":"OK  ");
			lcd_send_string(lcd_buff);
			break;
		case 12:lcd_put_cur(1, 5);
			break;
		case 13: 
			sprintf(lcd_buff,"%s",
				(state&ST_LOOP_ERR)?"FAIL":"OK  ");
			lcd_send_string(lcd_buff);
			break;
		case 14: lcd_put_cur(1, 12);
			break;
		case 15: 
			sprintf(lcd_buff,"%04lx", state);
			lcd_send_string(lcd_buff);
			break;
		default:
			lcd_step = 7;
			break;
		}
		break;
#if MY_CFG_ENCODER_ENABLE
#  if 0
	case LCDP_ENC_DEMO:
		switch(lcd_step) {
		case 0: enc_setup_fc(&enc, 0, 100, 0, 1, 5, 1, 0, 0);
			lcd_clear();
			break;	
		case 1: lcd_put_cur(0, 0);
			break;
		case 2: 
			sprintf(lcd_buff,"Val:%4ld",
				enc_get_current(&enc) );
			lcd_send_string(lcd_buff);
			break;
		case 3: lcd_put_cur(1, 0);
			break;
		case 4: 
			sprintf(lcd_buff,"SW1:%1d SW2:%1d",
				enc_get_sw(&enc, 1), enc_get_sw(&enc, 2));
			lcd_send_string(lcd_buff);
			break;
		default:
			lcd_step = 0;
			break;

		}
		break;
#  endif
#  if MY_CFG_18B20_ENABLE
	case LCDP_LIST_TEMP:
		switch(lcd_step) {
		case 0:
			num_th = my_18b20_get_number();
			enc_setup(&enc, 0, num_th - 1, cur, 1, -1, -1);
			lcd_clear();
			break;	
		case 1:
			lcd_put_cur(0, 0);
			break;
		case 2:			
			len = 0;
			cur = enc_get_current(&enc);
			if (thd[idt[cur]].valid ) {
				len += sprintf(lcd_buff+len,"T%02d ", idt[cur]);
				len += float_to_str((uint8_t *)lcd_buff+len,
						    thd[idt[cur]].value_avg,
						    6,2);
				*((uint8_t *)lcd_buff+len) = 0;
			} else {
				len += sprintf(lcd_buff+len,
					       "T%02d  null   ", idt[cur]);
			}
			
			lcd_send_string(lcd_buff);
			break;

		case 3:
			lcd_put_cur(1, 0);
			break;
		case 4:
			if (thd[idt[cur]].valid ) {
				len = 1;
				*((uint8_t *)lcd_buff) = 'M';
				len += float_to_str((uint8_t *)lcd_buff+len,
						    thd[idt[cur]].max, 6,2);
				*((uint8_t *)lcd_buff+len++) = ' ';
				*((uint8_t *)lcd_buff+len++) = ' ';
				*((uint8_t *)lcd_buff+len++) = 'm';
				len += float_to_str((uint8_t *)lcd_buff+len,
						    thd[idt[cur]].min, 6,2);
				*((uint8_t *)lcd_buff+len) = 0;
			} else {
				for(len=0;len<16;len++)
					*((uint8_t *)lcd_buff+len) = ' ';	
			}
			lcd_send_string(lcd_buff);
			break;
			
		default:
			lcd_step = 0;
			break;
		}
		break;
#  endif
#endif
	}
	if ( lcd_step >= 0 )
		lcd_step++;
	
	return 0;
}


static uint32_t lcd_to = 0;

int my_lcd_set_time_out(uint8_t index)
{
	uint32_t min = 0;
	
	switch(index) {
	case 1: min = 5; break;
	case 2: min = 10; break;
	case 3: min = 30; break;
	default: break;
	}

	lcd_to = min * 15000; /* 15000:= 60 * 250 */

	return 0;
}



/* FIXME: Hay que reescribirla from scratch: */
int my_lcd_run(int flag) 
{
	static uint32_t show = 0;
	static uint32_t count = 0;
#if MY_CFG_ENCODER_ENABLE
	static int cur_enc_val = 0;
#endif
	if ( flag == 1 && count != lcd_to) {
		show++;
		if ( show == LCDP_NUM ) show = LCDP_MAC;
	} else if ( flag == 2 && count != lcd_to) {
		if ( show != LCDP_MAC &&  show != LCDP_IP) show--;
		else show = LCDP_NUM - 1;
	} else if ( flag == 4 ) { /* on err. */
		show = LCDP_ST;
	} else if ( flag == 8  && show == LCDP_IP) {
		show = LCDP_TIME;
	}
	if ( lcd_to ) {
		if ( flag ) {
			count = 0;
			lcd_light_set(1);
		} else {
			if (count < lcd_to )
				count++;
			else if (count == lcd_to)
				lcd_light_set(0);
		}
	} else count = 1;
		
#if MY_CFG_ENCODER_ENABLE
	if ( cur_enc_val != enc_get_current(&enc) ) {
		if ( count == lcd_to ) {
			enc_set_current(&enc, cur_enc_val);
			lcd_light_set(1);
		} else {
			cur_enc_val = enc_get_current(&enc);
		}
		count = 0;
	}
#endif
	if ( show == LCDP_MAC &&
	     state & ST_IP_ASSIGNED ) {
		show = LCDP_IP;
	} else if (show == LCDP_IP && !(state & ST_IP_ASSIGNED) ) {
		show++;
	}
	
	if ( !lcd_run() )
		my_lcd_page(show);
	
	return show;
}

#endif /* MY_CFG_LCD_ENABLE */



