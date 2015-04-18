
/*
 * deduplib.h
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

#ifndef SOLIDLIB_H
#define SOLIDLIB_H

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SDEDUP_DONE = 101,
    SDEDUP_NOT_DONE = 102,
    SDEDUP_NULL_POINTER = 103,
    SDEDUP_ERROR = 104,
    SDIFF_DONE = 201,
    SDIFF_NOT_DONE = 202,
    SDIFF_NULL_POINTER = 203,
    SDIFF_ERROR = 204,
    
    SDUP_DONE = 105,
    SDUP_NOT_DONE = 106,
    SDUP_NULL_POINTER = 107,
    SDUP_ERROR = 108,
    
    SPATCH_DONE = 205,
    SPATCH_NOT_DONE = 206,
    SPATCH_ERROR = 207
} solid_result;

typedef struct {
    char *in_buffer; 
    char *out_buffer; // NULL buffer
    size_t in_len;
    size_t out_len;     
}t_solid_data;

typedef t_solid_data* SOLID_DATA;

extern char errorMsg[100];

#ifndef SEG_S
#define SEG_S 200000000
#endif

solid_result de_dup(SOLID_DATA buffer);
//solid_result duplicate(SOLID_DATA buffer);

solid_result diff(SOLID_DATA buffer);
//solid_result patch(SOLID_DATA buffer);

solid_result stream_compress(SOLID_DATA buffer);
//solid_result stream_decomp(SOLID_DATA buffer);

solid_result solid_compress(char* inbuffer, char *outbuffer, size_t in_len, size_t * out_len);
solid_result solid_compress_file(FILE *in_file, FILE *out_file);



#ifdef __cplusplus
}
#endif

#endif