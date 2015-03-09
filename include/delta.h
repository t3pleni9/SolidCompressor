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


#ifndef DELTA_H
#define DELTA_H

#include <stdio.h>
#include <stdlib.h>
#include <librsync.h>



#ifdef __cplusplus
extern "C" {
#endif

#define DEL_BLOCK 4000
#define SIG_BLOCK RS_DEFAULT_BLOCK_LEN

static size_t block_len = RS_DEFAULT_BLOCK_LEN;
static size_t strong_len = RS_DEFAULT_STRONG_LEN;


rs_result mem_diff(char *, char *, char *, int, int);
rs_result mem_patch(char *, char *, char *,  int , int ); 
rs_result rdiff_sig(char *, char *,  int);
    
    

#ifdef __cplusplus
}
#endif

#endif /* DELTA_H */ 
