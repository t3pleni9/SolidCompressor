
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

#include "index_struct.h"
#include <openssl/sha.h>

#ifndef DEDUPLIB_H
#define DEDUPLIB_H


#ifdef __cplusplus
extern "C" {
#endif
/*
typedef enum{
    DIFF_DONE = 1,
    DIFF_NOT_DONE = 2,
    DIFF_NULL_POINTER = 3,
    DIFF_ERROR = 4,
    
    PATCH_DONE = 5,
    PATCH_NOT_DONE = 6,
    PATCH_ERROR = 7
} diff_result;*/

typedef void* _DeDup;

_DeDup newDeDup();
void delDeDup(_DeDup deDup);
unsigned long int deDuplicate(_DeDup deDup, char *buffer, char *outBuffer, unsigned long int seg_s);
void duplicate(_DeDup deDup, char *ddBuffer, char *buffer);
void clearDictionary(_DeDup deDup);

SHA_CTX getHash(char* block, unsigned char* digest, unsigned int blockLen);
SHA_CTX getNextHash(char* block, unsigned char* digest, unsigned int blockLen, SHA_CTX context);
 
int generateIndex(IndexHeader node,unsigned int curBlock, int type, int buffer);   
int writeIndex(char* index);
int readIndex(char* index);
int getIndexHeader(unsigned int key, IndexHeader* node);
int getHeaderIndexCount();
void printIndex();

//diff_result do_diff(char *inBuffer, char *baseBuffer, char **deltaBuffer, size_t inLen, size_t baseLen, size_t *outLen);
//diff_result do_patch(char *deltaBuffer, char *baseBuffer, char **patchBuffer, size_t deltaLen, size_t baseLen, size_t *patchLen);


#ifdef __cplusplus
}
#endif

#endif
