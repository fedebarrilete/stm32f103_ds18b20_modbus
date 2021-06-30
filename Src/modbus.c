/** 
 *   @file modbus.c
 *   @brief ModBus TCP implementation.
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 05/03/2020
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
 *   05/03/2020: Created.
 **/
/*
  Documentation:
  https://www.modbus.org/specs.php
  Modbus_Application_Protocol_V1_1b3.pdf
*/

#include <stdio.h>
#include <string.h>
#include "modbus.h"
#include "my_tcp.h"
#include "huart_tools.h"
#include "my_config.h"
#include "my_ds18b20.h"

#if MY_CFG_MODBUS_ENABLE

#define SWAP16(n) ( ((((uint16_t)(n) & 0xff))  << 8) |\
		    (((uint16_t)(n)  & 0xff00) >> 8) )

/* Big endian word order for 32-bit integer: */
#define SWAP32(n) (((((uint32_t)(n) & 0xff))       << 24) | \
                  ((((uint32_t)(n)  & 0xff00))     <<  8) | \
                  ((((uint32_t)(n)  & 0xff0000))   >>  8) | \
                  ((((uint32_t)(n)  & 0xff000000)) >> 24))

/*
  -0: First reference is 0 (PDU addressing) instead 1
  modpoll  10.10.10.167 -p 502  -r 4000 -c 5
  00 01 00 00 00 06 01 03 0f 9f 00 05 
  00 02 00 00 00 06 01 03 0f 9f 00 05 
  00 03 00 00 00 06 01 03 0f 9f 00 05 
  modpoll  10.10.10.167 -p 502  -r 4000 -c 5 -0

  |00 01|00 00|00 06|01||03|0f a0|00 05 
  |00 02|00 00|00 06|01||03|0f a0|00 05 
  |00 03|00 00|00 06|01||03|0f a0|00 05
  |     |     |     |  ||   |     +-     
  |     |     |     |  ||   +-------     
  |     |     |     |  |+-----------     
  |     |     |     |  +------------ End Header MBAP     
  |     |     |     +--------------- Unit Identifier
  |     |     +--------------------- Length: Number of following bytes    
  |     +--------------------------- Protocol Identifier [0:MODBUS]
  +--------------------------------- Transaction Identifier

  3999 = 0x0f9f
  4000 = 0x0fa0

*/


static uint8_t mb_buff[sizeof(MBAP)];
static uint8_t mb_buff_out[MB_BUFF_OUT_LEN];
static MBAP *mb = (MBAP *) mb_buff;
static uint8_t socket_enables[MAX_SOCKS] = {0,};


/* Input Registers | 16-bit word | Read-Only */
static int mb_f4(uint8_t sn)
{
	int ret = 0;
	MBAP_SEND v;
	uint8_t *bop = mb_buff_out;
#if MY_CFG_18B20_ENABLE
	THER18B20DATA *thd = my_18b20_get_data_p();
#endif
	uint16_t addr, addr_to;

	v.fc = MBAP_FUNC_CODE(mb);
	v.num_bytes = 0;
	addr = MBAP_ADDR(mb);
	addr_to = addr + MBAP_NUM_REG(mb);
	if ( addr == MB_FIRMWARE_ADDR ) {
		*((int16_t *)bop) = SWAP16(VERSION << 8 | SUBVERSION);
		bop += 2;
		addr += 1;
	}
	if ( addr == MB_THER_NUM  && addr < addr_to ) {
#if MY_CFG_18B20_ENABLE
		*((int16_t *)bop) = SWAP16(my_18b20_get_number());
#else
		*((int16_t *)bop) = 0;
#endif
		bop += 2;
		addr += 1;
	}
	if ( addr == MB_UNIT_ID && addr < addr_to ) {
		*((int16_t *) bop) = SWAP16(MNC_GET_ID());
		bop += 2;
		addr += 1;
	}
	if ( addr == MB_CFG_SERIAL_ID && addr < addr_to ) {
		*((uint32_t *) bop)= SWAP32(MY_CFG_SERIAL_ID);
		bop += 4;
		addr += 2;
	}
	if ( addr == MB_UPTIME_ADDR && addr < addr_to ) {
		*((uint32_t *) bop) = SWAP32(sec_ticks);
		bop += 4;
		addr += 2;
	}
	if ( addr == MB_TIME_ADQ_ADDR && addr < addr_to ) {
		*((uint32_t *) bop)= SWAP32(adq_time);
		bop += 4;
		addr += 2;
	}
#if MY_CFG_18B20_ENABLE
	if( addr >= MB_THER_ADDR &&  addr_to <= (MB_THER_ADDR + THERM_NUM )) {
		int id;
		uint8_t *idt =  my_18b20_get_id_table();
		while ( addr < (MBAP_ADDR(mb) + MBAP_NUM_REG(mb)) && !ret) {
			if ( addr >= MB_THER_ADDR && (state & ST_ADQ_ENABLE) ) {
				id = addr - MB_THER_ADDR;
				if ( thd[idt[id]].valid ) {
					*((int16_t *) bop) = SWAP16(
						(int16_t)
						(thd[idt[id]].value_avg * 100));
				} else {
					*((int16_t *) bop) = SWAP16(-8500);
				}
				bop += 2;
				addr += 1;
			} else {
				ret = 6; /* Server Busy */
				addr_to = 0;
			}
		}
	}
#endif
#if MY_CFG_BAROMETER_ENABLE
	if ( addr == MB_PRESSURE && addr < addr_to ) {
		*((uint32_t *) bop)= SWAP32(pressure);
		bop += 4;
		addr += 2;	
	}
#endif
       	if ( addr >= MB_SWITCH0_ON && addr < addr_to &&
		    addr + MBAP_NUM_REG(mb) <= MB_SWITCH15_OFF + 2 ) {
		while ( addr < (MBAP_ADDR(mb) + MBAP_NUM_REG(mb)) ) {
			if ( addr == MB_SWITCH0_ON ) {
				*((uint32_t *) bop) = SWAP32(door_sw_open_time);
			} else if ( addr == MB_SWITCH0_OFF ) {
				*((uint32_t *) bop)= SWAP32(door_sw_close_time);
			} else {
				/* not defined yet */
				*((uint32_t *) bop) = 0;
			}
			bop += 4;
			addr += 2;
		}
	}
#if MY_CFG_18B20_ENABLE
	if ( addr >= MB_ID_TABLE &&
	     addr + MBAP_NUM_REG(mb) <= (MB_ID_TABLE + THERM_NUM) ) {
		uint8_t *idt =  my_18b20_get_id_table();
		while ( addr < (MBAP_ADDR(mb) + MBAP_NUM_REG(mb)) ) {
			*((int16_t *) bop) =
				SWAP16((int16_t) (idt[addr - MBAP_ADDR(mb)]));
			bop += 2;
			addr += 1;
		}
	}
#endif
	v.num_bytes = (addr - MBAP_ADDR(mb)) << 1;
	mb->h.len_l = v.num_bytes + 3;
	mb->h.len_h = 0;
	
	if (addr == MBAP_ADDR(mb) ) {
		ret = 2; /* ILLEGAL DATA ADDRESS */
	}

	if ( ret == 0 ) {
		if ( MBAP_NUM_REG(mb) != (v.num_bytes >> 1) ) 
			ret = 3; /* ILLEGAL DATA VALUE */
	}
	if ( ret ) {
		v.fc |= 0x80;
		v.num_bytes = ret; /* ERROR CODE */
		mb->h.len_l = 3;
		mb->h.len_h = 0;
	}
	send(sn, mb_buff, sizeof(MBAP_H) );
	send(sn, (uint8_t*) &v, sizeof(MBAP_SEND) );
	if ( !(v.fc & 0x80) )
		send(sn, mb_buff_out, v.num_bytes );

	return ret;
}


/* Read Coils | Single bit | Read-Write */
static int mb_f1(uint8_t sn) 
{
	int ret = 0;
	uint16_t *p16 = (uint16_t *) mb_buff_out;
	MBAP_SEND v;

	v.fc = MBAP_FUNC_CODE(mb);
	if ( MBAP_ADDR(mb) >= MB_COILS &&
	     (MBAP_ADDR(mb) + MBAP_NUM_REG(mb)) <= (MB_COIL16 + 1) ) {
		int mask = (1<<MBAP_NUM_REG(mb)) - 1;
		v.num_bytes = (MBAP_NUM_REG(mb) + 7) >> 3;
		*p16 = coil_register;
		*p16 = *p16 >> (MBAP_ADDR(mb) - MB_COILS);
		*p16 &= mask;
		mb->h.len_l = v.num_bytes + 3;
		mb->h.len_h = 0;
	} else {
		v.fc |= 0x80;
		v.num_bytes = 2; /* ERROR CODE: 2 -> Illegal data address */
		mb->h.len_l = 3;
		mb->h.len_h = 0;
	}

	send(sn, mb_buff, sizeof(MBAP_H) );
	send(sn, (uint8_t*) &v, sizeof(MBAP_SEND) );
	if ( !(v.fc & 0x80) )
		send(sn, mb_buff_out, v.num_bytes );

	return ret;
}



/* Discrete Inputs | Single bit | Read-Only */
static int mb_f2(uint8_t sn) 
{
	int ret = 0;
	uint16_t *p16 = (uint16_t *) mb_buff_out;
	MBAP_SEND v;

	v.fc = MBAP_FUNC_CODE(mb);
	if ( MBAP_ADDR(mb) >= MB_DI_IN &&
	     (MBAP_ADDR(mb) + MBAP_NUM_REG(mb)) <= (MB_DI_IN16 + 1) ) {
		int mask = (1<<MBAP_NUM_REG(mb)) - 1;
		v.num_bytes = (MBAP_NUM_REG(mb) + 7) >> 3;
		*p16 = dout_register;
		*p16 = *p16 >> (MBAP_ADDR(mb) - MB_DI_IN);
		*p16 &= mask;
		mb->h.len_l = v.num_bytes + 3;
		mb->h.len_h = 0;
	} else {
		v.fc |= 0x80;
		v.num_bytes = 2; /* ERROR CODE: 2 -> Illegal data address */
		mb->h.len_l = 3;
		mb->h.len_h = 0;
	}

	send(sn, mb_buff, sizeof(MBAP_H) );
	send(sn, (uint8_t*) &v, sizeof(MBAP_SEND) );
	if ( !(v.fc & 0x80) )
		send(sn, mb_buff_out, v.num_bytes );

	return ret;
}


static int mb_f_illegal(uint8_t sn)
{
	MBAP_SEND v;

	v.fc = MBAP_FUNC_CODE(mb) | 0x80;
	v.num_bytes = 1; /* ERROR CODE: 1 -> Illegal function */
	mb->h.len_l = 3;
	mb->h.len_h = 0;

	send(sn, mb_buff, sizeof(MBAP_H) );
	send(sn, (uint8_t*) &v, sizeof(MBAP_SEND) );

	return 0;
}


/* Read Coils | Single bit | Read-Write */
static int mb_f5(uint8_t sn) 
{
	int ret = 0;
	uint16_t *p16 = (uint16_t *) (mb_buff_out + 1);
	MBAP_SEND v;

	v.fc = MBAP_FUNC_CODE(mb);
	if ( MBAP_ADDR(mb) >= MB_COILS && MBAP_ADDR(mb) <= MB_COIL16  ){
		v.num_bytes = 2;
		p16[0] = mb->f.addr_l << 8 | mb->f.addr_h;
		mb_buff_out[0] = 5;
		mb->h.len_l = 5;
		mb->h.len_h = 0;
	
		if ( mb->f.num_reg_h == 0xff ) { /* Set coil ON */
			coil_register |= 1 << (MBAP_ADDR(mb) - MB_COILS);
			p16[1] = 0x00ff;
		} else if ( mb->f.num_reg_h == 0 ) { /* Set coil OFF */
			coil_register &=
				~(1 << (MBAP_ADDR(mb) - MB_COILS));
			p16[1] = 0x0000;
		} else {
			v.fc |= 0x80;
			v.num_bytes = 3; /* ERROR: Illegal Data Value */
			mb->h.len_l = 3;
			mb->h.len_h = 0;
		}
	} else {
		v.fc |= 0x80;
		v.num_bytes = 2; /* ERROR CODE: 2 -> Illegal data address */
		mb->h.len_l = 3;
		mb->h.len_h = 0;
	}

	send(sn, mb_buff, sizeof(MBAP_H) );
	if ( v.fc & 0x80 ) /* On err */
		send(sn, (uint8_t*) &v, sizeof(MBAP_SEND) );
	else {
		send(sn,mb_buff_out , 5 );
	}
	return ret;
}


static int mb_f15(uint8_t sn)
{
	/* FIXME: ALL */

	return mb_f_illegal(sn);
}


int mb_close_clients(void)
{
	int i;
	for ( i = 0; i < MAX_SOCKS; i++ ) {
		if (socket_enables[i]) {
			socket_enables[i] = 0;
			//disconnect(i);
			close(i);
		}
	}

	return 0;
}


int mb_run(uint8_t sn, uint16_t port)
{
	int ret = -1;
	uint16_t len;

	if ( tcp_server_run(sn, port) > 0 ) {
		socket_enables[sn] = 1;
		len = recv(sn, mb_buff, sizeof(MBAP) );
		if ( len != (6+MBAP_LEN(mb) ) )
			return -1;
		if ( MBAP_PT_ID(mb) != 0 ||
		     MBAP_UNIT_ID(mb) !=  UNIT_IDENTIFIER )
			return -2;
		if ( len ) {
			if ( MBAP_FUNC_CODE(mb) == 1 )
				 /* Read Coils */
				ret = mb_f1(sn);
			else if ( MBAP_FUNC_CODE(mb) == 2 )
				/* Read Discrete Inputs */
				ret = mb_f2(sn);
			else if ( MBAP_FUNC_CODE(mb) == 3 )
				 /* Read Holding Registers */
				ret = mb_f4(sn);
			else if ( MBAP_FUNC_CODE(mb) == 4 )
				 /* Read Input Registers */
				ret = mb_f4(sn);
			else if ( MBAP_FUNC_CODE(mb) == 5 )
				 /* Write Single Coil */
				ret = mb_f5(sn);
			else if ( MBAP_FUNC_CODE(mb) == 6 )
				/* Write Single Holding Register */
				ret = mb_f_illegal(sn); 
			else if ( MBAP_FUNC_CODE(mb) == 15 )
				 /* Write Multiple Coils */
				ret = mb_f15(sn);
			else if ( MBAP_FUNC_CODE(mb) == 16 )
				/* Write Mulriple Holding Register */
				ret = mb_f_illegal(sn);
			else {
				/* Illegal function */
				ret = mb_f_illegal(sn);
			}
#if MY_CFG_DEBUG
			if ( ret < 0 ) {
				printf("Invalid pk.\r\n");
			}
#endif
		}
	}

	return ret;
}


#endif
