/** 
 *   @file modbus.h
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
 **/


#ifndef _MODBUS_H
#define _MODBUS_H


#include <stdint.h>
#include "my_ds18b20.h"
#include "mb_addrs.h"

#define PDU_START_ADDR 1
#define UNIT_IDENTIFIER 1

#define MB_BUFF_IN_LEN 8
#define MB_BUFF_OUT_LEN 256

//#pragma pack(push)  /* push current alignment to stack */
//#pragma pack(1)     /* set alignment to 1 byte boundary */
typedef struct _mbap_header {
	uint8_t tr_id_h; /* Transaction Identifier */
	uint8_t tr_id_l;
	uint8_t pt_id_h; /* Protocol Identifier [0:MODBUS] */
	uint8_t pt_id_l;
	uint8_t len_h;   /* Length: Number of following bytes */
	uint8_t len_l;
	uint8_t unit_id; /* Unit Identifier */
} MBAP_H;
//#pragma pack(pop)   /* restore original alignment from stack */


typedef struct _mbap_pdu {
	uint8_t f_code;     /* Function Code */
	uint8_t addr_h;     /* Starting Address */
	uint8_t addr_l;
       	uint8_t num_reg_h;  /* Quantity of Registers */
       	uint8_t num_reg_l;
} MBAP_PDU;


typedef struct _mbap {
	MBAP_H h;
	MBAP_PDU f;
	uint8_t d[MB_BUFF_IN_LEN];
} MBAP;


#define MBAP_FUNC_CODE(m) (m->f.f_code)
#define MBAP_ADDR(m)     ((m->f.addr_h    << 8 | m->f.addr_l) + PDU_START_ADDR)
#define MBAP_NUM_REG(m)   (m->f.num_reg_h << 8 | m->f.num_reg_l)

#define MBAP_TR_ID(m)   (m->h.tr_id_h << 8 | m->h.tr_id_l)
#define MBAP_PT_ID(m)   (m->h.pt_id_h << 8 | m->h.pt_id_l)
#define MBAP_LEN(m)     (m->h.len_h   << 8 | m->h.len_l)
#define MBAP_UNIT_ID(m) (m->h.unit_id)


typedef struct _pdu_send {
	uint8_t fc;
	uint8_t num_bytes;
} MBAP_SEND;


int mb_close_clients(void);
int mb_run(uint8_t sn, uint16_t port);

#endif  /* END _MODBUS_H */
