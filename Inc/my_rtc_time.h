/** 
 *   @file my_config.h
 *   @brief Some initialization parameters
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 27/01/2020
 * 
 *   @copyright (c) Copyright 2018 Federico Bareilles <fede@fcaglp.unlp.edu.ar>.
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


#ifndef _MY_RTC_TIME_H
#define _MY_RTC_TIME_H

#include "main.h"
#include "SNTP/sntp.h"
#include "my_config.h"

#if MY_CFG_RTC_ENABLE
uint32_t my_time(uint32_t *tloc);
int my_HAL_Set_Unix_Time(RTC_HandleTypeDef *hrtc, uint32_t time);
#else
extern uint32_t _time_offset_uptime;

inline uint32_t my_time(uint32_t *tloc){
	return _time_offset_uptime + sec_ticks;
}
inline int my_HAL_Set_Unix_Time(RTC_HandleTypeDef *hrtc, uint32_t time){
	_time_offset_uptime = time - sec_ticks;
	return 0;
}
#endif

int my_RTC_Init(RTC_HandleTypeDef *hal_rtc);



#endif /* _MY_RTC_TIME_H */
