/** 
 *   @file cmd.h
 *   @brief  Command line implementation.
 *   @author Bareilles Federico fede@fcaglp.unlp.edu.ar>
 *   @date 28/01/2020
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
 *   28/01/2020: Reimplemente for tcp server on STM32 and W5500.
 *   2007: Port to libiarcontrol on version 0.1.8.
 *   2004: Initial verson on "Camera Control for STAR I."
 **/

#ifndef _CMD_H
#define _CMD_H

#include <stdint.h>

#ifndef WHITESPACE
#  define WHITESPACE(c) (((c) == ' ') || ((c) == '\t'))
#endif

#define CMD_QUIT 0x10
#define CMD_HELP 0x11
#define CMD_OK   0x00
#define CMD_FAIL   -1

#define TCP_CMD_BUFFER_LEN 80

typedef int cmd_callback (uint8_t, void *);

typedef struct {
        char *name;             /* User printable name of the function. */
        cmd_callback *func;     /* Function to call to do the job. */
        char *doc;              /* Documentation for this function.  */
} COMMAND;

__attribute__((weak)) int com_help(uint8_t sn, void *arg);
__attribute__((weak)) int com_quit(uint8_t sn, void *arg);


int execute_line (uint8_t sn, COMMAND *cmd, char *line);

#endif /* _CMD_H */
