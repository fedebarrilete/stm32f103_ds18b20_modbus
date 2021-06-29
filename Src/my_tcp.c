/** 
 *   @file my_tcp.c
 *   @brief TCP server implementation.
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 15/09/2020
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
 *   15/09/2020: Created.
 **/


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "my_config.h"
#include "my_tcp.h"


static uint8_t _default_prompt[] = {"> "};
static uint8_t *_prompt[MAX_SOCKS];// = _default_prompt;
			       
void set_prompt(uint8_t sn,uint8_t *pt)
{
	if ( pt == NULL )
		_prompt[sn] = _default_prompt;
	else
		_prompt[sn] = pt;
}


int tcp_server_listen_on(uint8_t sn, uint16_t port)
{
	int ret;

	if((ret = socket(sn, Sn_MR_TCP,port, 0)) != sn)
	{
		printf( "# [%ld] Error open socket\r\n", sec_ticks);
		return ret;
	}

	if((ret = listen(sn)) != SOCK_OK)
	{
		printf("# [%ld] Error listen socket\r\n", sec_ticks);
		return ret;
	}
	printf("# [%ld] Socked listened, wait for input connection on port %d\r\n",
	       sec_ticks, port);

	return 0;
}


int tcp_server_run(uint8_t sn, uint16_t port)
{
	int16_t rmsize = -1;

	if ( getSn_SR(sn) != SOCK_LISTEN ) {
		if ( getSn_SR(sn) == SOCK_ESTABLISHED ) {
			getsockopt(sn, SO_REMAINSIZE, &rmsize); 
		} else {
			disconnect(sn);
			close(sn);
			tcp_server_listen_on(sn, port);
		}
	}

	return rmsize;
}



int tcp_server_menu(uint8_t sn, COMMAND *cmd, uint8_t show_pt )
{
	uint8_t buff[TCP_CMD_BUFFER_LEN];
	int32_t len;
	int ret = 0;
	
	len = recv(sn, buff, TCP_CMD_BUFFER_LEN );
	if ( len > 1) {
		len--;
		while ( buff[len]=='\n' ||  buff[len]=='\r' ||
			buff[len] == 0xff ){
			buff[len] = 0;
			len--;
		}
		ret = execute_line(sn, cmd, (char *) buff);
	}

	if ( ret == CMD_QUIT ) {
		disconnect(sn);
		close(sn);
	}
#if 0
	else if ( ret == -2 ) {
		disconnect(sn);
		close(sn);
		HAL_Delay(1050); /* Wait WDT. */
	}
#endif
	if ( show_pt )
		send(sn, _prompt[sn], strlen( (char *) _prompt[sn]) );
		//send(sn, (uint8_t*) "> ", 2 );
	
	return ret;
}

