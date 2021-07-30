/** 
 *   @file my_w5500.c
 *   @brief initialization ethernet w5500 via ioLibrary_Driver.
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 26/01/2020
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
 *   26/01/2020: Created.
 **/

#include "my_config.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "Ethernet/socket.h"
#include "DHCP/dhcp.h"
#if MY_CFG_DNS_ENABLE
#include "DNS/dns.h"
#endif
#include "my_w5500.h"
#include "my_tcp.h"
#include "huart_tools.h"
#include "cmd.h" 

static SPI_HandleTypeDef *_w5500_spi = NULL;
static wiz_NetInfo *_net_info = NULL;
static UART_HandleTypeDef *_con = NULL; /* Loggin console */

volatile uint8_t new_ip_assigned = 0;

// 1K should be enough, see https://forum.wiznet.io/t/topic/1612/2
static uint8_t dhcp_buffer[236+312];
#if MY_CFG_DNS_ENABLE
// 1K seems to be enough for this buffer as well
static uint8_t dns_buffer[MAX_DNS_BUF_SIZE];
static uint8_t ip_dns[4];
#endif

				       
void my_w5500_set_loggin_uart(UART_HandleTypeDef *u)
{
	_con = u;
}


void W5500_Select(void) {
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_RESET);
}


void W5500_Unselect(void) {
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);
}


void W5500_ReadBuff(uint8_t* buff, uint16_t len) {
    HAL_SPI_Receive(_w5500_spi, buff, len, HAL_MAX_DELAY);
}


void W5500_WriteBuff(uint8_t* buff, uint16_t len) {
    HAL_SPI_Transmit(_w5500_spi, buff, len, HAL_MAX_DELAY);
}


uint8_t W5500_ReadByte(void) {
    uint8_t byte;
    W5500_ReadBuff(&byte, sizeof(byte));
    return byte;
}


void W5500_WriteByte(uint8_t byte) {
    W5500_WriteBuff(&byte, sizeof(byte));
}


void Callback_IPAssigned(void) {
	uprintf(_con,"# [%ld] Callback: IP assigned! Leased time: %ld sec\r\n",
		sec_ticks, getDHCPLeasetime());
	new_ip_assigned = 1;
}


void Callback_IPConflict(void) {
	uprintf(_con,"[%ld] Callback: IP conflict!\r\n",sec_ticks);
}


static void print_net_data(wiz_NetInfo *n_i)
{
	uprintf(_con,"# [%ld] DHCP:%s\r\n# \tIP:  %d.%d.%d.%d\r\n# \tGW:  %d.%d.%d.%d\r\n# \tMask:%d.%d.%d.%d\r\n# \tDNS: %d.%d.%d.%d\r\n", sec_ticks,
		n_i->dhcp== 1?"STATIC":"DINAMIC",
		n_i->ip[0], n_i->ip[1], n_i->ip[2], n_i->ip[3],
		n_i->gw[0], n_i->gw[1], n_i->gw[2], n_i->gw[3],
		n_i->sn[0], n_i->sn[1], n_i->sn[2], n_i->sn[3],
		n_i->dns[0],n_i->dns[1],n_i->dns[2],n_i->dns[3]
		);
	
	return;
}


int my_w5500_init(SPI_HandleTypeDef *hspi, wiz_NetInfo *net_info)
{
	int ret = -1;
	char tmpstr[6] = {0,};

	_net_info = net_info;
	_w5500_spi = hspi;
	reg_wizchip_cs_cbfunc(W5500_Select, W5500_Unselect);
	reg_wizchip_spi_cbfunc(W5500_ReadByte, W5500_WriteByte);
	reg_wizchip_spiburst_cbfunc(W5500_ReadBuff, W5500_WriteBuff);

	//uprintf(_con,"\r\n# STM32 ID: %lx %lx %lx\r\n", id[0], id[1], id[2]);
	
	//printf("Calling wizchip_init()...\r\n");
	/* rx_tx_buff_sizes dimension is number of sockets: 8 sockets. */
	uint8_t rx_tx_buff_sizes[] = {2, 2, 2, 2, 2, 2, 2, 2};
	wizchip_init(rx_tx_buff_sizes, rx_tx_buff_sizes);

	/* Get WIZCHIP name: */
	ctlwizchip(CW_GET_ID,(void*)tmpstr); 
	printf("# WIZnet chip: %s\r\n", tmpstr);
	
/*
  STM32 ID: 671ff52 49558250 87120839
  STM32 ID: 66dff55 54526750 87252339
*/
	ret =  (strncmp(tmpstr, "W5500", 5) == 0?0:-1);
	if ( ret != 0 )
		return ret;

	if ( _net_info->dhcp == NETINFO_DHCP ) {
		setSHAR(_net_info->mac);
		DHCP_init(DHCP_SOCKET, dhcp_buffer);
		reg_dhcp_cbfunc(
			Callback_IPAssigned,
			Callback_IPAssigned,
			Callback_IPConflict
			);
	} else {
		wizchip_setnetinfo(_net_info);
	}

	uprintf(_con,"# [%ld] MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
		sec_ticks,
		_net_info->mac[0],_net_info->mac[1],_net_info->mac[2],
		_net_info->mac[3],_net_info->mac[4],_net_info->mac[5] );

	if ( _net_info->dhcp != NETINFO_DHCP ) 
		print_net_data(_net_info);

#if MY_CFG_DNS_ENABLE
	DNS_init(DNS_SOCKET, dns_buffer);
	if ( _net_info->dhcp == NETINFO_DHCP )
		getDNSfromDHCP(ip_dns);
#endif
	return 0;	
}


int my_dhcp_run(void)
{
	int ret = DHCP_IP_LEASED;

	if ( _net_info->dhcp != NETINFO_DHCP ) 
		return ret;

	ret = DHCP_run();

#if MY_DEBUG
	switch( ret ) {
	case DHCP_IP_ASSIGN:
		printf("# dhcp: ASSIGN   \r");
		break;
	case DHCP_IP_CHANGED:
		printf("# dhcp: CHANGED  \r");
		break;
	case DHCP_IP_LEASED: /* Normal working state */
		//printf("# dhcp: LEASED  \r");
		break;
	case DHCP_FAILED:
		printf("# dhcp: FAILED  \r");
		/* pseudocode:
		   my_dhcp_retry++;
		   if(my_dhcp_retry > MY_MAX_DHCP_RETRY) {
		   my_dhcp_retry = 0;
		   DHCP_stop();
		   }
		*/	  
		break;
	case DHCP_RUNNING:
		printf("# dhcp: RUNNING \r");
		break;
	case DHCP_STOPPED:
		printf("# dhcp: STOPPED \r");
		break;  
	default:
		printf("# dhcp: UNKNOW  \r");
		break;
	}
#endif
	if ( new_ip_assigned == 1) {
		new_ip_assigned = 0;
		wizchip_getnetinfo(_net_info);
		getIPfromDHCP(_net_info->ip);
		getGWfromDHCP(_net_info->gw);
		getSNfromDHCP(_net_info->sn);
		getDNSfromDHCP(_net_info->dns);
		wizchip_setnetinfo(_net_info);

		printf("# dhcp: LEASED  \r\n");
		print_net_data(_net_info);

		
	}

	return ret;
}


