/** 
 *   @file my_menu.c
 *   @brief  menu implementation.
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 02/03/2020
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


#include "my_config.h"
#if MY_CFG_MENUCFG_ENABLE
#include "main.h"
#include "usart.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "rtc.h"
#include "my_tcp.h"
#include "my_rtc_time.h"
#include "huart_tools.h"
#include "cmd.h"
#include "eeprom.h"
#include "my_menu.h"
#if MY_CFG_LCD_ENABLE
#include "my_lcd.h"
#include "lcd.h"
#endif
#include "my_ds18b20.h"
#include "modbus.h"

char msg[160] = {0,};

/*
------------------------------------------------------------
                Institute AGGO - CONICET
            MODBUS Themometer Logger (ThLog)
                 (c) Federico Bareilles
------------------------------------------------------------
*/

/*
static const char wellcome[] = {
	"\r\n\------------------------------------------------------------\r\n\t\tInstitute AGGO - CONICET\r\n\t    MODBUS Thermometer Logger (ThLog)\r\n\t         (c) Federico Bareilles\r\n" };
*/

static const char *wellcome[] = {
	"------------------------------------------------------------\r\n",
	"\t\tInstitute AGGO - CONICET\r\n",
#if MY_CFG_MODBUS_ENABLE
	"\t    MODBUS Thermometer Logger (ThLog)\r\n",
#endif
	"\t         (c) Federico Bareilles\r\n",
	NULL };


//static int com_test(uint8_t, void *);
static int com_date(uint8_t, void *);
static int com_time(uint8_t, void *);
#if  MY_CFG_NTP_ENABLE
static int com_ntpset(uint8_t, void *);
static int com_ntpdiff(uint8_t, void *);
#endif
static int com_uptime(uint8_t, void *);
//static int com_read_bk_reg(uint8_t, void *);
static int com_wdt(uint8_t sn, void *arg);
#if MY_CFG_RTC_ENABLE
static int com_rtccr(uint8_t sn, void *arg);
#else
static int com_cfg_skip_ms(uint8_t sn, void *arg);
#endif
static int com_reboot(uint8_t, void *);
static int com_status(uint8_t, void *);
static int com_clear_err(uint8_t, void *);
static int com_version(uint8_t, void *);
#if MY_CFG_18B20_ENABLE
static int com_list_th(uint8_t, void *);
#endif

static int com_cfg(uint8_t, void *);
static int com_cfg_quit(uint8_t, void *);
static int com_cfg_dhcp(uint8_t, void *);
static int com_cfg_ip(uint8_t, void *);
static int com_cfg_gw(uint8_t, void *);
static int com_cfg_mask(uint8_t, void *);
#if MY_CFG_DNS_ENABLE
static int com_cfg_dns(uint8_t, void *);
#endif
#if  MY_CFG_NTP_ENABLE
static int com_cfg_ntp_host(uint8_t, void *);
static int com_cfg_ntpenable(uint8_t, void *);
#endif
static int com_cfg_id(uint8_t, void *);
static int com_cfg_passwd(uint8_t, void *);
static int com_cfg_commit(uint8_t, void *);
#if MY_CFG_MODBUS_ENABLE
static int com_cfg_mbport(uint8_t, void *);
static int com_cfg_mbreset(uint8_t, void *);
#endif
static int com_cfg_cfgport(uint8_t, void *);

#if MY_CFG_LCD_ENABLE
static int com_cfg_lcd_to(uint8_t, void *);
#endif
static int com_format_ee(uint8_t, void *);
static int com_door_buzz(uint8_t, void *);

#if MY_CFG_18B20_ENABLE
static int com_th_cfg(uint8_t, void *);
static int com_th_quit(uint8_t, void *);
static int com_th_list(uint8_t, void *);
static int com_th_set_offset(uint8_t, void *);
static int com_th_rm(uint8_t, void *);
static int com_th_recover(uint8_t, void *);
static int com_th_auto_add(uint8_t, void *);
static int com_th_auto_stop(uint8_t, void *);
static int com_th_save(uint8_t, void *);
static int com_th_rmv_all(uint8_t, void *);
#endif
//static int com_(uint8_t, void *);





COMMAND cmd_p500[] = {
        //{ "test",      com_test,   "Test" },
//#if  MY_CFG_NTP_ENABLE 
        { "date",      com_date,   "Get current date" },
//#endif
        { "time",      com_time,   "Get/Set current Unix time" },
#if MY_CFG_NTP_ENABLE		
        { "ntpset",    com_ntpset,  "Set time via NTP server" },
        { "ntpdiff",   com_ntpdiff, "Show (local_time - NTP_time)" },
#endif
	{"uptime",     com_uptime, "System uptime"},
//        { "r_bk_reg",  com_read_bk_reg,"Reads data from the RTC Backup data Register. See AN2604" },
#if MY_CFG_RTC_ENABLE
	{ "rtccr",     com_rtccr,   "Get/Set clock calibration reg" },
#endif
	{ "reboot",    com_reboot, "Reboot" },
	{ "wdt",       com_wdt,    "Get WDT activations"},
	{ "st",        com_status, "Get status"},
	{ "clear_err", com_clear_err, "Clear error status flag"},
	{ "version",   com_version, "Show firmware version and others"},

	{ "config", com_cfg, "General Configuration: net and others" },
#if MY_CFG_18B20_ENABLE	
	{ "th_config", com_th_cfg, "Configure DS/QT18b20 array"},
	{ "r",         com_list_th,"Read current temperatures and pressure"},
#endif
//        { "help",      com_help,   "Display this text" },
        { "?",         com_help,   "Help" },
//        { "quit",      com_quit,   "Quit ..." },
        { "q",         com_quit,   "Quit" },
        { (char *)NULL, (cmd_callback *)NULL, (char *)NULL }
};


COMMAND cmd_p500_cfg[] = {
	//{ "test",      com_test,   "Test" },
	{ "dhcp",      com_cfg_dhcp, "Conf. DHCP mode. 0: STAIC, 1: DINAMYC" },
	{ "ip",        com_cfg_ip,  "Conf. IP"}, 
	{ "gw",        com_cfg_gw,  "Conf. GateWay"}, 
	{ "mask",      com_cfg_mask,"Conf. net mask"},
#if MY_CFG_DNS_ENABLE
	{ "dns",       com_cfg_dns, "Conf. DNS"},
#endif
	{ "mac",       com_version, "Show MAC addr"},
#if MY_CFG_MODBUS_ENABLE
	{ "mb_port",   com_cfg_mbport,  "Conf. MODBUS port"},
	{ "mb_reset",  com_cfg_mbreset, "Reset current MODBUS connection"},
#endif
	{ "cfg_port",  com_cfg_cfgport, "Conf. configuration port"},
#if MY_CFG_NTP_ENABLE
#  if MY_CFG_DNS_ENABLE
	{ "ntp_server",com_cfg_ntp_host,"Conf. canonical NTP server name"},
#  else
	{ "ntp_server",com_cfg_ntp_host,"Conf. IP NTP server"},
#  endif	
        { "ntpenable", com_cfg_ntpenable,"Enable disble NTP set time" },
#endif
#if ! MY_CFG_RTC_ENABLE
	{ "skip_ms",     com_cfg_skip_ms,   "Skip 1 ms every X seconds" },
#endif
	{ "unit_id",   com_cfg_id,"Get/Set Unit ID number [0-ff]"},
#if MY_CFG_LCD_ENABLE
	{ "lcdto",     com_cfg_lcd_to,"G/S LCD light off [min]: 0:dis,1:5,2:10,3: 30"},
#endif
	{ "passwd",    com_cfg_passwd,"Set new password"},
	{ "door_buzz", com_door_buzz,"Enable/Disable Door buzzer."}, 
	{ "commit",    com_cfg_commit,"Commit (save)  Currente configuration"},
	{ "Erase_EE", com_format_ee, "Erase EEPROM; all data will be lost"},
	//{ "reboot",    com_reboot, "Reboot" },
	{ "?",         com_help,   "Help" },
	{ "q",         com_cfg_quit,   "Back to previous" },
	//{ "qq",        com_quit,   "Close connection" },
        { (char *)NULL, (cmd_callback *)NULL, (char *)NULL }
};

uint8_t prompt_cfg[] = {"config> "};


#if MY_CFG_18B20_ENABLE
COMMAND cmd_th[] = {
	{ "ls",        com_th_list,  "List current thermometers" },
	{ "rm",        com_th_rm,    "Remove thermometer by ID"},
	{ "recover",   com_th_recover,"Recover thermometer by ID"},
	{ "so",        com_th_set_offset,"Set thermometer offset in hundredths"},
	{ "rm_all",    com_th_rmv_all, "Clear all list"},
	{ "auto_add",  com_th_auto_add,"Scan and auto add th."}, 
	{ "auto_stop",  com_th_auto_stop,"Stop auto add Th."}, 
	{ "save",      com_th_save,"Save Currente configuration"},
	//{ "reboot",    com_reboot, "Reboot" },
	{ "?",         com_help,   "Help" },
	{ "q",         com_th_quit,   "Back to previous" },
        { (char *)NULL, (cmd_callback *)NULL, (char *)NULL }
};


uint8_t prompt_th[] = {"18b20_config> "};
#endif

//COMMAND *current_p500 = cmd_p500;
COMMAND *current_p500[MAX_SOCKS] = {cmd_p500,cmd_p500,cmd_p500};
uint8_t prompt_def[] = {"[--] ThLog> "};
static uint8_t clien_auto_add[MAX_SOCKS] = {0,};
#if  MY_CFG_NTP_ENABLE
static uint8_t clien_get_ntp_diff[MAX_SOCKS] = {0,};
#endif

int my_menu_send_to_client(uint8_t *msg, int len)
{
	int i;

	for ( i = 0; i < MAX_SOCKS; i++) {
		if (clien_auto_add[i] ) {
			if ( send(i, msg, len ) <= 0 )
				clien_auto_add[i] = 0;
		}
	}

	return 0;
}



static char * ok_fail(int v)
{
	return  (v == 1?"ok":"fail");
}

#if MY_CFG_18B20_ENABLE
static int com_th_cfg(uint8_t sn, void *arg)
{	
	current_p500[sn] = cmd_th;
	set_prompt(sn, prompt_th);
	execute_line (sn, current_p500[sn], "?");

	sprintf(msg, "WARNING: Acquisition of thermometer deactivated.\r\n");
	send(sn, (uint8_t*)msg, strlen(msg) );
	state &= ~(ST_ADQ_ENABLE|ST_TH_SCAN);

	return CMD_OK;
}


static int com_th_quit(uint8_t sn, void *arg)
{
	current_p500[sn] = cmd_p500;
	set_prompt(sn, prompt_def);
	execute_line (sn, current_p500[sn], "?");
	my_18b20_make_index_table();
	state |= ST_ADQ_ENABLE;
	
	return CMD_OK;
}


static void _show_rom_id(uint8_t sn, int id)
{
	THER18B20ROM *th = my_18b20_get_rom_p();
	int len = sprintf(msg,"ID: [%02x][%02d] ", MNC_GET_ID(),id);
	if ( th->rom[id][0] != 0xff ) {
		for(int i=0;i<8;i++)
			len += sprintf(msg+len,"0x%02x, ",
				       th->rom[id][i] );
		len += sprintf(msg+len,"offset: %3d",
			       th->offset[id] );
	} else
		len += sprintf(msg+len,"deleted");
	sprintf(msg+len,"\r\n");
	send(sn, (uint8_t*)msg, strlen(msg) );

	return;
}


static int com_th_list(uint8_t sn, void *arg)
{
	THER18B20ROM *th = my_18b20_get_rom_p();

	sprintf(msg, "Current Table:\r\n");
	send(sn, (uint8_t*)msg, strlen(msg) );
	cicle_break = 1;
	for (int id = 0; id < THERM_NUM && th->rom[id][0]; id++) {
		_show_rom_id(sn, id);
	}
	
	return CMD_OK;
}


static int com_th_set_offset(uint8_t sn, void *arg)
{
	THER18B20ROM *th = my_18b20_get_rom_p();
	int len1,len2;
	int id;

	if ( *((char *) arg) != 0 ) {
		len2 = strlen(arg);
		for(len1=0;*((char *)arg+len1)!= ' ' && len1 < len2; len1++);
		if (len1 < len2) *((char *)arg+len1) = 0;
		id = atoi(arg);
		
		if ( id >= 0 && id < THERM_NUM && th->rom[id][0] != 0xff) {
			if (len1 < len2)
				th->offset[id] = atoi((char *)arg+len1+1);
			else
				_show_rom_id(sn, id);
		}
	}
	
	return CMD_OK;
}


/*
static int com_th_add(uint8_t sn, void *arg)
{
	THER18B20ROM *th = my_18b20_get_rom_p();
	THER18B20DATA *thd = my_18b20_get_data_p();

	if ( *((char *) arg) != 0 ) {
		int id = atoi(arg);
		if ( id >= 0 && id < THERM_NUM ) {
			th->rom[id][0] = 0xff;
			thd[id].valid = 0;
		}
	}
	
	return CMD_OK;
}
*/



static int com_th_rm(uint8_t sn, void *arg)
{
	THER18B20ROM *th = my_18b20_get_rom_p();
	THER18B20DATA *thd = my_18b20_get_data_p();
	int ret = CMD_FAIL;

	if ( *((char *) arg) != 0 ) {
		int id = atoi(arg);
		if ( id >= 0 && id < THERM_NUM ) {
			if ( th->rom[id][0] == 0x28 ) {
				th->rom[id][0] = 0xff;
				thd[id].valid = 0;
				ret = CMD_OK;
			}
		}
	}
	
	return ret;
}


static int com_th_rmv_all(uint8_t sn, void *arg)
{
	THER18B20ROM *th = my_18b20_get_rom_p();

	for (int id = 0; id < THERM_NUM && th->rom[id][0]; id++) {
		if ( th->rom[id][0] == 0x28 || th->rom[id][0] == 0xff ) {
			th->rom[id][0] = 0;
			sprintf(msg,"ID: [%02d] remove\r\n", id);
			send(sn, (uint8_t*)msg, strlen(msg) );
		}
	}
	
	return CMD_OK;
}



static int com_th_recover(uint8_t sn, void *arg)
{
	THER18B20ROM *th = my_18b20_get_rom_p();


	if ( *((char *) arg) != 0 ) {
		int id = atoi(arg);
		if ( id >= 0 && id < THERM_NUM && th->rom[id][0] == 0xff) {
			th->rom[id][0] = 0x28;
			//thd[id].valid = 0;
		} else {
			sprintf(msg, "Fail\r\n");
			send(sn, (uint8_t*)msg, strlen(msg) );
		}
	}
	
	return CMD_OK;
}


static int com_th_auto_add(uint8_t sn, void *arg)
{
	state |= ST_TH_SCAN;
	clien_auto_add[sn] = 1;
	
	return CMD_OK;
}


static int com_th_auto_stop(uint8_t sn, void *arg)
{
	state &= ~ST_TH_SCAN;
	clien_auto_add[sn] = 0;
	
	return CMD_OK;
}


static int com_th_save(uint8_t sn, void *arg)
{
	int ret;
	int retry = 0;
	THER18B20ROM *th = my_18b20_get_rom_p();

	cicle_break = 1;
	do {
		ret = EE_WRITE_BLK(2, (uint32_t *) th);
	} while ( ret == 0 && retry++ < 3 );
	
	sprintf(msg, "Write EEPROM: %s\r\n", ok_fail(ret) );
	send(sn, (uint8_t*)msg, strlen(msg) );

	return CMD_OK;
}
#endif /* MY_CFG_18B20_ENABLE */

static int com_cfg(uint8_t sn, void *arg)
{	
	current_p500[sn] = cmd_p500_cfg;
	set_prompt(sn, prompt_cfg);
	execute_line (sn, current_p500[sn], "?");
	
	return CMD_OK;
}


static int com_cfg_quit(uint8_t sn, void *arg)
{
	current_p500[sn] = cmd_p500;
	set_prompt(sn, prompt_def);
	execute_line (sn, current_p500[sn], "?");
	
	return CMD_OK;
}


static int com_cfg_dhcp(uint8_t sn, void *arg)
{
	if ( *((char *) arg) == 0 ) {
		sprintf(msg, "DHCP: %d - %s\r\n",
			net_info_ee->dhcp==1?0:1,
			net_info_ee->dhcp == 1?"STATIC":"DINAMIC");
		send(sn, (uint8_t*)msg, strlen(msg) );
	} else {
		net_info_ee->dhcp = (atoi(arg) == 0 ?
				    NETINFO_STATIC:0);
		com_cfg_dhcp(sn, NULL);
	}
	
	return CMD_OK;
}


static int set_get_ip_number(uint8_t *ip, uint8_t sn, void *arg)
{
	if ( *((char *) arg) == 0 ) {
		sprintf(msg, "%d.%d.%d.%d\r\n",
			ip[0],ip[1],ip[2],ip[3]);
		send(sn, (uint8_t*)msg, strlen(msg) );
	} else {
		int i;
		uint8_t *pt = arg;
		uint8_t *pt0;
		for(i=0; i < 4 && pt != NULL; i++) {
			pt0 = pt;
			pt = memchr(pt0, (int) '.', 5);
			if ( pt != NULL ) {
				*(pt++) = 0;
			}
			ip[i] = atoi((char *) pt0);	
		}

		set_get_ip_number(ip, sn, NULL);
	}
	
	return CMD_OK;
}


static int com_cfg_ip(uint8_t sn, void *arg)
{	
	return set_get_ip_number(net_info_ee->ip, sn, arg);
}


static int com_cfg_gw(uint8_t sn, void *arg)
{	
	return set_get_ip_number(net_info_ee->gw, sn, arg);
}


static int com_cfg_mask(uint8_t sn, void *arg)
{	
	return set_get_ip_number(net_info_ee->sn, sn, arg);
}


#if MY_CFG_DNS_ENABLE
static int com_cfg_dns(uint8_t sn, void *arg)
{	
	return set_get_ip_number(net_info_ee->dns, sn, arg);
}
#endif


static int com_cfg_id(uint8_t sn, void *arg)
{
	if ( *((char *) arg) == 0 ) {
		sprintf(msg,"Unit ID: %02x\r\n", MNC_GET_ID());
		send(sn, (uint8_t*)msg, strlen(msg) );
	} else {
		MNC_SET_ID(strtol(arg, NULL, 16));
		sprintf(msg,"New unit ID is: %02x\r\n",
			MNC_GET_ID() );
		send(sn, (uint8_t*)msg, strlen(msg) );
	}

	return CMD_OK;
}


#if MY_CFG_LCD_ENABLE
static int com_cfg_lcd_to(uint8_t sn, void *arg)
{
	uint8_t val;
	
	if ( *((char *) arg) == 0 ) {
		sprintf(msg,"LDC time out: %d\r\n",my_conf->lcd_to );
		send(sn, (uint8_t*)msg, strlen(msg) );
	} else {
		val = strtol(arg, NULL, 10);
		if ( val < 4 ) {
			my_conf->lcd_to = val;
			my_lcd_set_time_out(my_conf->lcd_to);
			sprintf(msg,"New val is: %d\r\n", my_conf->lcd_to);
		} else {
			sprintf(msg,"Out of range: %d\r\n", val);
		}
			send(sn, (uint8_t*)msg, strlen(msg) );
	}

	return CMD_OK;
}
#endif

static int com_door_buzz(uint8_t sn, void *arg)
{
	uint8_t val;
	
	if ( *((char *) arg) == 0 ) {
		sprintf(msg,"Door Buzzer enable: %d\r\n", DOOR_BUZZER_ENABLE );
		send(sn, (uint8_t*)msg, strlen(msg) );
	} else {
		val = strtol(arg, NULL, 10);
		if ( val ) {
			DOOR_BUZZER_ENABLE = 1;
		} else {
			DOOR_BUZZER_ENABLE = 0;	
		}
		sprintf(msg,"New val is: %d\r\n", DOOR_BUZZER_ENABLE);
		send(sn, (uint8_t*)msg, strlen(msg) );
	}

	return CMD_OK;
}

static int com_cfg_passwd(uint8_t sn, void *arg)
{
	if ( *((char *) arg) == 0 ) {
		//my_conf->passwd[0] = 0;
		*(MNC_GET_PASSWD()) = 0;
		sprintf(msg,"password disable.\r\n");
		send(sn, (uint8_t*)msg, strlen(msg) );
	} else {
		int len = strlen(arg);
		memset(MNC_GET_PASSWD(), 0, 8);
		if (len > 8) len = 8;
		strncpy( MNC_GET_PASSWD(), arg, strlen(arg));
		sprintf(msg,"password changed\r\n");
		send(sn, (uint8_t*)msg, strlen(msg) );
	}

	return CMD_OK;
}



static int com_cfg_commit(uint8_t sn, void *arg)
{
	//char msg[80];
	int ret, retry = 0;
	cicle_break = 1;

	do {
		//ret = EE_WRITE_BLK(0, (uint32_t *) net_info_ee);
		ret = EE_WRITE_BLK(0, conf_buff);
	} while ( ret == 0 && retry++ < 3 );
	sprintf(msg,"eeprom save: %s\r\n", ok_fail(ret) );
	send(sn, (uint8_t*)msg, strlen(msg) );
	
	return CMD_OK;
}


static int set_get_uint16(uint16_t *v, uint8_t sn, void *arg)
{
	if ( *((char *) arg) == 0 ) {
		sprintf(msg, "Curent: %d\r\n", *v );
		send(sn, (uint8_t*)msg, strlen(msg) );
	} else {
		*v = atoi(arg);
	}
	
	return CMD_OK;
}

#if MY_CFG_MODBUS_ENABLE
static int com_cfg_mbport(uint8_t sn, void *arg)
{
	return set_get_uint16(&my_conf->mbport, sn, arg);;
}


static int com_cfg_mbreset(uint8_t sn, void *arg)
{
	cicle_break = 1;
	//return disconnect(TCP0_SOCKET); /* FIXME: test it. */
	////close(TCP0_SOCKET);
	return mb_close_clients();
}
#endif

static int com_cfg_cfgport(uint8_t sn, void *arg)
{
	return set_get_uint16(&my_conf->cfgport, sn, arg);
}


#if  MY_CFG_NTP_ENABLE
#  if MY_CFG_DNS_ENABLE
static int com_cfg_ntp_host(uint8_t sn, void *arg)
{
	int len = strlen(arg);
	if ( *((char *) arg) == 0 ) {
		sprintf(msg, "Curent: %s\r\n", my_conf->ntp_name );
		send(sn, (uint8_t*)msg, strlen(msg) );
	} else if (len < 29 ) {
		memcpy (my_conf->ntp_name, arg, len );
		memset(my_conf->ntp_name+len,0, 29-len);
	} else {
		sprintf(msg, "Error: The name is to long.\r\n");
		send(sn, (uint8_t*)msg, strlen(msg) );		
	}
	
	return CMD_OK;
}
#  else
static int com_cfg_ntp_host(uint8_t sn, void *arg)
{
	return set_get_ip_number(my_conf->ntp_ip, sn, arg);
}
#  endif
#endif

static int com_format_ee(uint8_t sn, void *arg)
{
	int ret, retry = 0;
	do {
		ret = EE_Format();
	} while ( ret == 0 && retry++ < 3 );
	sprintf(msg,"eeprom erased: %s\r\n", ok_fail(ret) );
	send(sn, (uint8_t*)msg, strlen(msg) )
		;
	return CMD_OK;
}


#if 0
static int com_test(uint8_t sn, void *arg)
{
	//char msg[80] = {0,};
	
	sprintf(msg, "%lu\r\n",  my_time(NULL) );
	send(sn, (uint8_t*)msg, strlen(msg) );
	
	sprintf(msg, "RTC Prescaler: %lu\r\n",
		READ_REG(hrtc.Instance->PRLH) << 16 |
		READ_REG(hrtc.Instance->PRLL)
		);
	send(sn, (uint8_t*)msg, strlen(msg) );
	
	return CMD_OK;
}
#endif


#if MY_CFG_18B20_ENABLE
static int com_list_th(uint8_t sn, void *arg)
{
	THER18B20DATA *thd = my_18b20_get_data_p();
	uint8_t *idt =  my_18b20_get_id_table();
	int num_th = my_18b20_get_number();
	int len;
	
	sprintf(msg, "Current Temperatures:\r\n");
	send(sn, (uint8_t*)msg, strlen(msg) );
	cicle_break = 1; /* FIXME */
	for (int id = 0; id < num_th; id++) {
		int id_name = idt[id];
		len = sprintf(msg,"ID: [%02x][%02d] -> ", MNC_GET_ID(),id_name);
		if (thd[id_name].valid ) {
			len += sprintf(msg+len, "%03d",
				       (int) (thd[id_name].value_avg*100));
			*(msg+len)   = *(msg+len-1);
			*(msg+len-1) = *(msg+len-2);
			*(msg+len-2) = '.';
			len++;
			len += sprintf(msg+len, "\t%03d",
				       (int) (thd[id_name].max*100));
			*(msg+len)   = *(msg+len-1);
			*(msg+len-1) = *(msg+len-2);
			*(msg+len-2) = '.';
			len++;
			len += sprintf(msg+len, "\t%03d",
				       (int) (thd[id_name].min*100));
			*(msg+len)   = *(msg+len-1);
			*(msg+len-1) = *(msg+len-2);
			*(msg+len-2) = '.';
			len++;
			len += sprintf(msg+len, "\t%03d",
				       (int) ((thd[id_name].max -
					       thd[id_name].min)*100));
			*(msg+len)   = *(msg+len-1);
			*(msg+len-1) = *(msg+len-2);
			*(msg+len-2) = '.';
			len++;
		} else {
			len += sprintf(msg+len, "(null)");
		}
		sprintf(msg+len,"\r\n");
		send(sn, (uint8_t*)msg, strlen(msg) );
	}
#  if MY_CFG_BAROMETER_ENABLE
	if ( pressure != 0 ) {
		sprintf(msg,"ID: [%02x] pressure -> %ld Pa\r\n",
			      MNC_GET_ID(), pressure);
		send(sn, (uint8_t*)msg, strlen(msg) );	
	}
#  endif
#  if MY_CFG_DHT_ENABLE
	if (  dht_humidity != 0 ) {
		len = sprintf(msg,"ID: [%02x] humidity -> %3ld  %%, ",
			      MNC_GET_ID(), dht_humidity);
		*(msg+len-4)   = *(msg+len-5);
		*(msg+len-5) = *(msg+len-6);
		*(msg+len-6) = '.';
		len += 	sprintf(msg+len, "T -> %4ld \r\n", dht_temperature);
		*(msg+len-2)   = *(msg+len-3);
		*(msg+len-3) = *(msg+len-4);
		*(msg+len-4) = '.';
		send(sn, (uint8_t*)msg, strlen(msg) );	
	}
#  endif
	return CMD_OK;
}
#endif

#if MY_CFG_RTC_ENABLE
static int com_rtccr(uint8_t sn, void *arg)
{
	//char msg[80] = {0,};
	uint8_t bkpccr;

	/* Calibration: AN2604.pdf
	   /home/fede/STM32Cube/Repository/AN2604.pdf
	   Know values:
	   Nominal values: prescaler = 32768, rtccr = 0.
	   |                 ID                |prescaler|rtccr|sec/mon
	   STM32 ID: 66dff55 54526750 87252339     32766    33    81.65
	   STM32 ID: 46f78dc 18003129 160707e2     32766    32    78.74
	   STM32 ID: 671ff52 49558250 87120839  (Bad RTC)
	*/
	bkpccr = READ_REG(BKP->RTCCR);
	if ( *((char *) arg) == 0 ) {
		sprintf(msg, "RTCCR: %u\r\n", bkpccr );
		send(sn, (uint8_t*)msg, strlen(msg) );
	} else {
		bkpccr = (bkpccr&0xff80) | (atoi(arg) & 0x7f);
		WRITE_REG(BKP->RTCCR,  bkpccr);
		MNC_RTCCR_ROM = atoi(arg) & 0x7f;
	}
	return CMD_OK;
}

#else

static int com_cfg_skip_ms(uint8_t sn, void *arg)
{
	if ( *((char *) arg) == 0 ) {
		sprintf(msg, "skip every: %u sec.\r\n", MNC_RTCCR_ROM);
		send(sn, (uint8_t*)msg, strlen(msg) );
	} else {
		MNC_RTCCR_ROM = atoi(arg);
	}
	return CMD_OK;
}
#endif


#if 0
static int com_read_bk_reg(uint8_t sn, void *arg)
{
	//char msg[256] = {0,};
	uint32_t reg;
	int32_t len = 0;

	for ( reg = 1; reg < 43 && len < 256; reg++ ) {
		len+=sprintf(msg+len, "%04lx ", HAL_RTCEx_BKUPRead(&hrtc, reg ) );
	}
	if ( len < 256 ) {
		len+=sprintf(msg+len, "\r\n");
		send(sn, (uint8_t*)msg, strlen(msg));
		return CMD_OK;
	} else {
		return CMD_FAIL;
	}
}
#endif


static int com_date(uint8_t sn, void *arg)
{
	datetime *dt;

	dt = my_localtime(my_time(NULL));
	sprintf(msg,"%4d/%02d/%02d %02d:%02d:%02d %s\r\n",
		dt->yy, dt->mo, dt->dd, dt->hh, dt->mm, dt->ss,
		my_tz_str()); 
	send(sn, (uint8_t*)msg, strlen(msg));

	return CMD_OK;
}


static int com_time(uint8_t sn, void *arg)
{
	if ( *((char *) arg) == 0 ) {
		sprintf(msg, "%lu\r\n",  my_time(NULL) );
		send(sn, (uint8_t*)msg, strlen(msg) );
	} else {
		my_HAL_Set_Unix_Time(&hrtc, (uint32_t) atol(arg));
	}
	
	return CMD_OK;
}


#if MY_CFG_NTP_ENABLE	
static int com_ntpset(uint8_t sn, void *arg)
{
	int ret = CMD_OK;
	
	if (  MNC_NTP_ENABLE() )
		state &= ~(ST_NTP_SET|ST_NTP_FAIL);
	else {
		sprintf(msg, "NTP not enable: FAIL\r\n");
		send(sn, (uint8_t*)msg, strlen(msg) );
		ret = CMD_FAIL;
	}
	return ret;
}


static int com_ntpdiff(uint8_t sn, void *arg)
{
	state |= ST_NTP_GET;

	clien_get_ntp_diff[sn] = 1;
	set_prompt(sn, (uint8_t *) "diff: ");

	return CMD_OK;
}


static int com_cfg_ntpenable(uint8_t sn, void *arg)
{
	if ( *((char *) arg) == 0 ) {
		sprintf(msg, "NTP: %d\r\n", MNC_NTP_ENABLE() );
		send(sn, (uint8_t*)msg, strlen(msg) );
	} else {
		MNC_NTP_ENABLE() = atoi(arg);
		//com_cfg_commit(sn, arg);
	}

	return CMD_OK;
}
#endif /* MY_CFG_NTP_ENABLE */


static int com_uptime(uint8_t sn, void *arg)
{
	int len;
	len = sprintf(msg, "%lu", sec_ticks);
	if ( day_ticks )
		len+=sprintf(msg+len, " [%lu days]", day_ticks );
	sprintf(msg+len, "\r\n");
	send(sn, (uint8_t*)msg, strlen(msg) );
	
	return CMD_OK;
}


static int com_reboot(uint8_t sn, void *arg)
{
	send(sn, (uint8_t*)"rebooting...\r\n", 14);
	disconnect(sn);
	close(sn);
	wizphy_reset();
	NVIC_SystemReset();
	
	return CMD_OK;
}


static int com_wdt(uint8_t sn, void *arg)
{
#if MY_CFG_WDT_ENABLE
	sprintf(msg, "WDT count: %u, reboot on wdt event: %u\r\n",
		wdt_get_counter(), wdt_active );
#else
	sprintf(msg, "WDT count: %u. WARNING: WDT disable in firmware.\r\n",
		wdt_get_counter());
#endif
	send(sn, (uint8_t*)msg, strlen(msg) );
	
	return CMD_OK;
}


static int com_status(uint8_t sn, void *arg)
{
	uint32_t len = 0;
	
	len=sprintf(msg+len, "NTP set: %d, NTP FAIL: %d, Loop err: %d, OW err: %d, DHT err: %d\r\n",
		    (state&ST_NTP_SET)?1:0, (state&ST_NTP_FAIL)?1:0,
		    (state&ST_LOOP_ERR)?1:0,
		    (state&ST_OW_ERR)?1:0,
		    (state&ST_DHT_ERR)?1:0
		);
	send(sn, (uint8_t*)msg, strlen(msg) );
	
	return CMD_OK;
}


static int com_clear_err(uint8_t sn, void *arg)
{
	state &= ~(ST_ERR);

	return CMD_OK;
}


static int com_version(uint8_t sn, void *arg)
{
	uint32_t *id = (uint32_t *) UID_BASE;
	int i = sprintf(msg,"STM32 ID: %lx %lx %lx\r\n",
			id[0], id[1], id[2]);
	i += sprintf(msg+i, "MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
		     net_info_ee->mac[0], net_info_ee->mac[1],
		     net_info_ee->mac[2], net_info_ee->mac[3],
		     net_info_ee->mac[4], net_info_ee->mac[5]);
	i += sprintf(msg+i, "FW v: %1d.%1d\r\n", VERSION, SUBVERSION);
	i += sprintf(msg+i, "Conf. id: 0x%08lx -> 0b", MY_CFG_SERIAL_ID);
	for(int j = 23; j > -1; j--) /* Max 31 */
		i += sprintf(msg+i, "%01d",
			     (uint8_t) (MY_CFG_SERIAL_ID >> j) & 0x01);
	i += sprintf(msg+i,"\r\n");

	send(sn, (uint8_t*)msg, i/*strlen(msg)*/ );
	return CMD_OK;
}


int validate_client(uint8_t sn)
{
	uint8_t buff[TCP_CMD_BUFFER_LEN];
	int ret = -1;

	recv(sn, buff, TCP_CMD_BUFFER_LEN );
	
	if ( strncmp( (char *) buff, MNC_GET_PASSWD(),
		      strlen( MNC_GET_PASSWD()) ) == 0 ) {
		ret = 0;
	}
	
	return ret;
}


static uint8_t prompt_pass[] = {"\rpassword: "};
static uint8_t echo_off[]={0xff,0xfb,0x01};
static uint8_t echo_on[]= {0xff,0xfc,0x01};

static int8_t start_p500[MAX_SOCKS] = {0,0,0,};
static int8_t count_pass[MAX_SOCKS] = {0,0,0,};

int my_menu_p500_run(uint8_t sn, int port)
{
	//static int8_t start_p500 = 0;
	//static int8_t count_pass = 0;
	int ret;
	
	if ( (ret = tcp_server_run(sn, port)) > 0 ) {
		if (  start_p500[sn] == 2 ) {
			uint8_t buff[ret];
			recv(sn, buff, ret); /* dummy read. */
			start_p500[sn] = 3;
		} else if ( start_p500[sn] == 3 ) {
			tcp_server_menu(sn, current_p500[sn], start_p500[sn]);
		} else {
			start_p500[sn] = 1;
			if ( validate_client(sn) == 0 ) {
				send(sn,echo_on, 3);
				send(sn,(uint8_t *)"\r", 1);
				execute_line (sn, current_p500[sn], "?");
				send(sn, prompt_def,
				     strlen( (char *) prompt_def) );
				start_p500[sn] = 2;
			} else {
				count_pass[sn]++;
				if ( count_pass[sn] == 4 ) {
					disconnect(sn);
					close(sn);
					count_pass[sn] = 0;
				} else {
					if ( count_pass[sn] > 1 )
						send(sn,
						     (uint8_t *)"\r\n", 2);
					send(sn,prompt_pass,
					     strlen( (char *) prompt_pass) );
				}
			}
		}
	} else if ( ret < 0 ) {
		start_p500[sn] = 0;
		count_pass[sn] = 0;
	} else if ( start_p500[sn] == 0 ) {
                int i;
#if MY_CFG_DEBUG != 0
		uint8_t rIP[4];
		uint16_t rPort;

		getsockopt(sn, SO_DESTIP, rIP);
		getsockopt(sn, SO_DESTPORT,(uint8_t*)&rPort);
		uprintf(&huart1,"# [%ld] Connected from %d.%d.%d.%d:%d\r\n",
			sec_ticks,
		       rIP[0],rIP[1],rIP[2],rIP[3],rPort);
#endif

		sprintf((char *) prompt_def,"[%02x] %s", MNC_GET_ID(),
			prompt_def+5);
		for(i=0; wellcome[i] != NULL; i++ )
			send(sn, (uint8_t *) wellcome[i], strlen(wellcome[i]));
		//send(sn, (uint8_t *) wellcome, strlen(wellcome));
		if ( MNC_PASSWD_ENABLE() == 0 ) {
			execute_line (sn, current_p500[sn], "?");
			send(sn, prompt_def,
			     strlen( (char *) prompt_def) );
			start_p500[sn] = 3;
		} else {
			start_p500[sn] = 1;
	 		send(sn,echo_off, 3);
		}		
	}

	/* Send ntp diff if request: */
#if  MY_CFG_NTP_ENABLE
	if ( clien_get_ntp_diff[sn] == 1 && !(state & ST_NTP_GET) ) {
		clien_get_ntp_diff[sn] = 0;
		sprintf(msg, "%ld\r\n", ntp_diff );
		send(sn, (uint8_t*)msg, strlen(msg) );
		set_prompt(sn, prompt_def);
		send(sn, prompt_def, strlen( (char *) prompt_def) );
	}
#endif
	
	return ret;
}

#endif /* MY_CFG_MENUCFG_ENABLE */
