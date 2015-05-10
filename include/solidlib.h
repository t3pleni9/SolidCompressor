
/*
 * solidlib.h
 * 
 * Copyright 2015 Justin Jose <justinjose999@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#ifndef __SOLIDLIB_H
#define __SOLIDLIB_H

#include "scons.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char errorMsg[100];

SOLID_RESULT solid_compress(char* inbuffer, char *outbuffer, size_t in_len, size_t * out_len);
SOLID_RESULT solid_compress_fd(int in_fd, int dump_fd);

#ifdef __cplusplus
}
#endif

#endif
