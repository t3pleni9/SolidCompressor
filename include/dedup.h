/*
 * dedup.h
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


#include <string>
#include <cstring>
#include <stdio.h>
#include <openssl/sha.h>
#include <unordered_map>

#include "scons.h"
#include "index.h"

#ifndef DEDUP_H

/**
 * @brief 
 * Header Definition 
 */
#define DEDUP_H

/**
 * @brief 
 * Block Size
 **/
#define BLOCK_S 4000

/**
 * @brief 
 * (delta)d = blockSize / x
 **/
#define BLOCK_X 8



/**
 * @brief 
 * Initialization block numbers for dictionary
 **/
#define BLOCK_N 1000000

extern char *__outBuffer__;
extern char *__tempBuffer__;

#ifdef __cplusplus

class DeDup
{
    public:
        DeDup();
        virtual ~DeDup();
        SOLID_RESULT de_dup(SOLID_DATA buffer);
        SOLID_RESULT duplicate(SOLID_DATA de_buffer);
        void clearDictionary();
    
    private:				
        typedef std::unordered_map<std::string, IndexNode> t_index;
        t_index strgIndex;
        Index node;       
        unsigned long int deDuplicate(char*, char **, unsigned long int);
        int deDuplicateSubBlocks(char*, int, int, unsigned long int*); 
        bool nodeExists(std::string);
        int readBlock(std::fstream*, char*, int, int);
        
};

extern "C" {
#endif

void * de_dup(void* _args);
void * duplicate(void* _args);
extern char errorMsg[100];

#ifdef __cplusplus
}
#endif

#endif /* DEDUP_H */ 
