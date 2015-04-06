/*
 * index_struct.h
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

#ifndef INDEX_STRUCT_H
#define INDEX_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _node_{
    char * node;
    int node_len;
    struct _node_ *next;
} _node_t;


typedef struct {
    unsigned size : 5;
    //unsigned segment : 3;
} Node;

typedef struct IndexNode {
    unsigned int offsetPointer; /**< offset index */
    Node node; /**<  number of blocks*/
} IndexNode;

typedef struct IndexHeader {
    unsigned int offsetPointer;
    unsigned int block;
    unsigned int size;
    unsigned int type : 2;    
} IndexHeader;

#define TRUE 1
#define FALSE 0

#ifdef __cplusplus
}
#endif

#endif
