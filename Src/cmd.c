/** 
 *   @file cmd.c
 *   @brief  Command line implementation.
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 28/01/2020
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
 *   28/01/2020: Reimplemente for tcp server on STM32 and W5500.
 *   2007: Porter to libiarcontrol on version 0.1.8.
 *   2004: Initial verson on "Camera Control for STAR I."
 **/


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "socket.h" /* for send() */
#include "cmd.h"


__attribute__((weak)) int com_help(uint8_t sn, void *arg)
{
	return CMD_HELP;

}

__attribute__((weak)) int com_quit(uint8_t sn, void *arg)
{
	send(sn, (uint8_t*)"bye.\r\n\n", 7);
	return CMD_QUIT;
}

static int send_help(uint8_t sn, COMMAND *cmd)
{
#define buff_len 512
	char msg[buff_len];// = {0,};
	const uint8_t dash_len = 60;
	int i;
	int32_t len = 0;

	memset(msg,'-',dash_len);
	*(msg+dash_len) = '\r';
	*(msg+dash_len + 1) = '\n';
	len = dash_len+2;
	for (i = 0; cmd[i].name; i++) {
		len+=sprintf(msg+len, "%-12s:%s.\r\n",
			     cmd[i].name, cmd[i].doc );
		if ( len > (buff_len - 80) ) {
			send(sn, (uint8_t*)msg, len);
			len = 0;
		}
	}
	if ( len > (buff_len - 80) ) {
		send(sn, (uint8_t*)msg, len);
		len = 0;
	}
	memset(msg+len,'-',dash_len);
	*(msg+len+dash_len) = '\r';
	*(msg+len+dash_len + 1) = '\n';
	len += dash_len+2;
	send(sn, (uint8_t*)msg, len);

	return 0;
}


/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
static COMMAND *find_command (COMMAND *cmd, char *name)
{
        int i;

        for (i = 0; cmd[i].name; i++)
                if (strcmp (name, cmd[i].name) == 0)
                        return (&cmd[i]);

        return ((COMMAND *)NULL);
}



/* Execute a command line. */
int execute_line (uint8_t sn, COMMAND *cmd, char *line)
{
        int i;
        COMMAND *command;
        char *word;
	int ret = CMD_FAIL;
	char msg[TCP_CMD_BUFFER_LEN] = {0,};
	
        /* Isolate the command word. */
        i = 0;
        while (line[i] && WHITESPACE(line[i]))  i++;
        word = line + i;

        while (line[i] && !WHITESPACE (line[i])) i++;

        if (line[i]){
                line[i++] = '\0';
        }
        command = find_command (cmd , word);
        if ( !command ) {
		if ( *(word) != 0 ) {
			sprintf(msg, "Unknow [%s]\r\n", word);
			send(sn, (uint8_t*)msg, strlen(msg));
		}
                return CMD_FAIL;
        }

        /* Get argument to command, if any. */
        while (WHITESPACE (line[i])) i++;

        word = line + i;

        /* Call the function. */
        //return ((*(command->func)) (sn) (word));
	ret = command->func(sn, word);
	if ( ret == CMD_HELP ) {
		send_help(sn, cmd);
	}
	
	return ret;
}
