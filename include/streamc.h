/*
 * streamc.h
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

#include <zlib.h>
#include <stdio.h>
#include <assert.h>

#include "scons.h"

#ifndef __STREAMC_H
#define __STREAMC_H

#ifdef __cplusplus
extern "C" {
#endif

extern int level;
extern char errorMsg[100];

void* zlib_compress(void* _args);
SOLID_RESULT bzip2_compress(char *in_buffer, char **out_buffer, size_t in_len, size_t *out_len); // TODO: to be done bzip2
SOLID_RESULT _7z_compress(char *in_buffer, char **out_buffer, size_t in_len, size_t *out_len); // TODO: to be done 7z

void* zlib_decompress(void* _args);

#ifdef __cplusplus
}
#endif

#endif
