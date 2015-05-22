/*
 * constants.h
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
 
#ifndef __SCONS_H
#define __SCONS_H
 
#ifdef __cplusplus
extern "C" {
#endif


#ifndef SEG_S
#define SEG_S 200000000
#endif

#ifndef CHUNK
#define CHUNK 3800
#endif

#define PIPE_SEG 50000

extern char scompressor;
extern char delta;
extern char duplicator;

typedef unsigned long int ulInt;



/* 
 * Add more as pluggins. 
 * Make sure compressor is placed first 
 * and decompressor is (100 + compressor) 
 * Add cases in _s_init for custom added enums.
 * 
 */
typedef enum {
    
    ZLIBC = 101, //Stream compressor
    BZIPC = 102,
    _7Z_C = 103,

    ZLIBD = 201, //Stream de-compressor
    BZIPD = 202,
    _7Z_D = 203,
    
    ZDLTA = 301, // Delta compressor - diff
    
    ZDPAT = 401, // Delta compressor - patch
    
    LZDDP = 501, // Deduplicator 
    
    LZDUP = 601 // Duplicator
} MODALGO;
    

typedef enum {
    S_NULL = 0,
    S_DONE = 1,
    
    SDEDUP_DONE = 101,
    SDEDUP_ERROR = 102,
    SDEDUP_NULL_POINTER = 103,
    
    SDIFF_DONE = 201,
    SDIFF_ERROR = 202,
    SDIFF_NULL_POINTER = 203,
    
    SSTRM_DONE = 301,
    SSTRM_ERROR = 302,
    SSTRM_NULL_POINTER = 303,
    
    SDUP_DONE = 105,
    SDUP_ERROR = 106,
    SDUP_NULL_POINTER = 107,
    
    SPATCH_DONE = 205,
    SPATCH_NOT_DONE = 206,
    SPATCH_ERROR = 207,
    
    SPIPE_DONE = 401,
    SPIPE_NOT_SET = 402,   
    SPIPE_ERROR = 403,
    
    SQU_DONE = 501,
    SQU_ERROR = 502,
    
    STH_ERROR = 601,
    STH_DONE = 602,
} SOLID_RESULT;

typedef void * (*__stream__)(void *);
typedef SOLID_RESULT (*__delta__)(void *);
typedef void * (*__dedup__)(void *);

typedef struct {
    int in;
    int out;
} file_d;

typedef struct {
    int id; // For debugging purpose
    char *in_buffer; 
    char *out_buffer; // NULL buffer
    file_d fd;
    size_t in_len;
    size_t out_len; 
    __dedup__ dupcomp; // de-duplicator 
    __delta__ dcomp; // delta compressor
    __stream__ scomp; // stream compressor
    SOLID_RESULT end_result;
}t_solid_data;

typedef t_solid_data* SOLID_DATA;

int write_buf(int fd, const void *buf, int size);
int fill_buffer(SOLID_DATA buffer, size_t buf_len);
int refill_buffer(SOLID_DATA buffer, size_t buf_len);
SOLID_RESULT wait_for_finish(pthread_t t_th);


extern unsigned long int netIn;
extern unsigned long int netOut;

#ifdef __cplusplus
}
#endif

#endif
