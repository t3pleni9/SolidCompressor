/*
 * delta.h
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


#ifndef MEMBUF_H
#define MEMBUF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <librsync.h>
#include <assert.h>



#ifdef __cplusplus
extern "C" {
#endif

#define DEL_BLOCK RS_DEFAULT_BLOCK_LEN
#define SIG_BLOCK RS_DEFAULT_BLOCK_LEN

struct mem_buf {
    char *memBuf;
    char *buf;
    size_t buf_len;
};

typedef struct mem_buf mem_buf_t;


static size_t block_len = RS_DEFAULT_BLOCK_LEN;
static size_t strong_len = RS_DEFAULT_STRONG_LEN;

mem_buf_t *mem_buf_new(size_t);
void mem_buf_free(mem_buf_t *);

rs_result mem_diff(char *, char *, char *, int, int);
rs_result mem_patch(char *, char *, char *,  int , int ); 
rs_result mem_sig(char *, char *,  int , int , rs_stats_t *);
rs_result mem_fill(rs_job_t *, rs_buffers_t *, void *);
rs_result mem_drain(rs_job_t *, rs_buffers_t *, void *);
rs_result mem_loadsig(char *, rs_signature_t **, rs_stats_t *, size_t);
rs_result mem_delta(rs_signature_t *, char *, char *, rs_stats_t *, size_t , size_t);
//int delta(char *, char *, char *)

rs_result mem_run(rs_job_t *, char *, char *, size_t, size_t);    
    

#ifdef __cplusplus
}
#endif

#endif /* MEMBUF_H */ 
