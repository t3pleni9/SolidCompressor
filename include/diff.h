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


#ifndef __DIFF_H
#define __DIFF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <zdlib.h>
#include <fuzzy.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "scons.h"


#ifdef __cplusplus
extern "C" {
#endif

extern char errorMsg[100];

typedef struct{
    unsigned int bit:1;
} bit_feild;

typedef struct _node_{
    char * data;
    char * fuzzy_hash_result;
    int ref_node;
    size_t node_size;
} _node_t;

typedef _node_t* NODESP;
typedef NODESP* NODEDP;


typedef enum{
    
    DIFF_DONE = 1,
    DIFF_NOT_DONE = 2,
    DIFF_NULL_POINTER = 3,
    DIFF_ERROR = 4,
    
    PATCH_DONE = 5,
    PATCH_NOT_DONE = 6,
    PATCH_ERROR = 7,
    
    DIFF_PIPE_ERROR = 8,
    DIFF_PIPE_IO_NULL = 9
} diff_result;

#define DIFF_BLOCK 1000000
#define MAX_DIFF_BLOCK ((SEG_S / DIFF_BLOCK) + 10)
#define PATCH_BLOCK (DIFF_BLOCK + sizeof(int) + sizeof(size_t))
#define DIFF_THLD 10
#define MAX_INT 2000000



SOLID_RESULT zdelta_diff(void* _args);
SOLID_RESULT zdelta_patch(void* _args);

//diff_result _do_patch(char *deltaBuffer, char *baseBuffer, char **patchBuffer, size_t deltaLen, size_t baseLen, size_t *patchLen);

#ifdef __cplusplus
}
#endif

#endif /* DIFF_H */ 
