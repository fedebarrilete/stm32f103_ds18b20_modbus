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


#ifndef _MB_ADDRS_H
#define _MB_ADDRS_H

//#include <stdint.h>

/* Input register (read as input or holding): */
#define MB_FIRMWARE_ADDR (1)                   // 0001 len 1
#define MB_THER_NUM      (MB_FIRMWARE_ADDR + 1)// 0002 len 1
#define MB_UNIT_ID       (MB_THER_NUM + 1)     // 0003 len 1
#define MB_CFG_SERIAL_ID (MB_UNIT_ID + 1)      // 0004 len 2
#define MB_UPTIME_ADDR   (MB_CFG_SERIAL_ID + 2)// 0006 len 2
#define MB_TIME_ADQ_ADDR (MB_UPTIME_ADDR + 2)  // 0008 len 2
#define MB_THER_ADDR     (MB_TIME_ADQ_ADDR + 2)    // 0010 len 1 * THERM_NUM
/* 0087 - 0099 free */
#define MB_PRESSURE      (MB_THER_ADDR + 100)  // 0110 len 2 only 17 bits
#define MB_HUMIDITY      (MB_PRESSURE    + 2)  // 0112 len 1
#define MB_H_TEMPERATURE (MB_HUMIDITY    + 1)  // 0113 len 1
/* 0114 - 0119 free */
/* Events register on switches: */
#define MB_SWITCH0_ON    (MB_THER_ADDR + 110)  // 0120 len 2
#define MB_SWITCH0_OFF   (MB_SWITCH0_ON  + 2)  // 0122 len 2
#define MB_SWITCH1_ON    (MB_SWITCH0_OFF + 2)  // 0124 len 2
#define MB_SWITCH1_OFF   (MB_SWITCH1_ON  + 2)  // 0126 len 2
#define MB_SWITCH2_ON    (MB_SWITCH1_OFF + 2)  // 0128 len 2
#define MB_SWITCH2_OFF   (MB_SWITCH2_ON  + 2)  // 0130 len 2
#define MB_SWITCH3_ON    (MB_SWITCH2_OFF + 2)  // 0132 len 2
#define MB_SWITCH3_OFF   (MB_SWITCH3_ON  + 2)  // 0134 len 2
#define MB_SWITCH4_ON    (MB_SWITCH3_OFF + 2)  // 0136 len 2
#define MB_SWITCH4_OFF   (MB_SWITCH4_ON  + 2)  // 0138 len 2
#define MB_SWITCH5_ON    (MB_SWITCH4_OFF + 2)  // 0140 len 2
#define MB_SWITCH5_OFF   (MB_SWITCH5_ON  + 2)  // 0142 len 2
#define MB_SWITCH6_ON    (MB_SWITCH5_OFF + 2)  // 0144 len 2
#define MB_SWITCH6_OFF   (MB_SWITCH6_ON  + 2)  // 0146 len 2
#define MB_SWITCH7_ON    (MB_SWITCH6_OFF + 2)  // 0148 len 2
#define MB_SWITCH7_OFF   (MB_SWITCH7_ON  + 2)  // 0150 len 2
#define MB_SWITCH8_ON    (MB_SWITCH7_OFF + 2)  // 0152 len 2
#define MB_SWITCH8_OFF   (MB_SWITCH8_ON  + 2)  // 0154 len 2
#define MB_SWITCH9_ON    (MB_SWITCH8_OFF + 2)  // 0156 len 2
#define MB_SWITCH9_OFF   (MB_SWITCH9_ON  + 2)  // 0158 len 2
#define MB_SWITCH10_ON   (MB_SWITCH9_OFF + 2)  // 0160 len 2
#define MB_SWITCH10_OFF  (MB_SWITCH10_ON  + 2) // 0162 len 2
#define MB_SWITCH11_ON   (MB_SWITCH10_OFF + 2) // 0164 len 2
#define MB_SWITCH11_OFF  (MB_SWITCH11_ON  + 2) // 0166 len 2
#define MB_SWITCH12_ON   (MB_SWITCH11_OFF + 2) // 0168 len 2
#define MB_SWITCH12_OFF  (MB_SWITCH12_ON  + 2) // 0170 len 2
#define MB_SWITCH13_ON   (MB_SWITCH12_OFF + 2) // 0172 len 2
#define MB_SWITCH13_OFF  (MB_SWITCH13_ON  + 2) // 0174 len 2
#define MB_SWITCH14_ON   (MB_SWITCH13_OFF + 2) // 0176 len 2
#define MB_SWITCH14_OFF  (MB_SWITCH14_ON  + 2) // 0178 len 2
#define MB_SWITCH15_ON   (MB_SWITCH14_OFF + 2) // 0180 len 2
#define MB_SWITCH15_OFF  (MB_SWITCH15_ON  + 2) // 0182 len 2
/* 0183 - 0199 free */
#define MB_ID_TABLE      (MB_THER_ADDR + 200)  // 0200 len 1 * THERM_NUM
/* 0277 - 0300 free */

/* Discrete Inputs -> DIgital INput (switch state): */
#define MB_DI_IN         (1)                   // 0001 len 1 (16 digital inputs)
#define MB_DI_IN1        (MB_DI_IN)            // 0001
#define MB_DI_IN2        (MB_DI_IN1 + 1)       // 0002
#define MB_DI_IN3        (MB_DI_IN2 + 1)       // 0003
#define MB_DI_IN4        (MB_DI_IN3 + 1)       // 0004
#define MB_DI_IN5        (MB_DI_IN4 + 1)       // 0005
#define MB_DI_IN6        (MB_DI_IN5 + 1)       // 0006
#define MB_DI_IN7        (MB_DI_IN6 + 1)       // 0007
#define MB_DI_IN8        (MB_DI_IN7 + 1)       // 0008 
/**/
#define MB_DI_IN9        (MB_DI_IN8 + 1)       // 0009
#define MB_DI_IN10       (MB_DI_IN9 + 1)       // 0010
#define MB_DI_IN11       (MB_DI_IN10 + 1)      // 0011
#define MB_DI_IN12       (MB_DI_IN11 + 1)      // 0012
#define MB_DI_IN13       (MB_DI_IN12 + 1)      // 0013
#define MB_DI_IN14       (MB_DI_IN13 + 1)      // 0014
#define MB_DI_IN15       (MB_DI_IN14 + 1)      // 0015
#define MB_DI_IN16       (MB_DI_IN15 + 1)      // 0016
/* last for protocol 0200 */

/* Coils -> digital input/output: */
#define MB_COILS        (1)                  // 0001 len 1 (16 digital inputs)
#define MB_COIL1        (MB_COILS)           // 0001
#define MB_COIL2        (MB_COIL1 + 1)       // 0002
#define MB_COIL3        (MB_COIL2 + 1)       // 0003
#define MB_COIL4        (MB_COIL3 + 1)       // 0004
#define MB_COIL5        (MB_COIL4 + 1)       // 0005
#define MB_COIL6        (MB_COIL5 + 1)       // 0006
#define MB_COIL7        (MB_COIL6 + 1)       // 0007
#define MB_COIL8        (MB_COIL7 + 1)       // 0008 
/**/
#define MB_COIL9        (MB_COIL8 + 1)       // 0009
#define MB_COIL10       (MB_COIL9 + 1)       // 0010
#define MB_COIL11       (MB_COIL10 + 1)      // 0011
#define MB_COIL12       (MB_COIL11 + 1)      // 0012
#define MB_COIL13       (MB_COIL12 + 1)      // 0013
#define MB_COIL14       (MB_COIL13 + 1)      // 0014
#define MB_COIL15       (MB_COIL14 + 1)      // 0015
#define MB_COIL16       (MB_COIL15 + 1)      // 0016
/* last for protcol 0200 */

#endif  /* END _MB_ADDRS_H */
