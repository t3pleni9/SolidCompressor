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
#define BLOCK_S 64

/**
 * @brief 
 * (delta)d = blockSize / x
 **/
#define BLOCK_X 8

/**
 * @brief 
 * Segment Size
 **/
#define SEG_S 200000000

/**
 * @brief 
 * Initialization block numbers for dictionary
 **/
#define BLOCK_N 1000000


/**
 * @brief 
 * Block hash with the index header. 
 */

#ifdef  __cplusplus

class DeDup
{
    public:
        DeDup();
        virtual ~DeDup();
        unsigned long int deDuplicate(char*, char *, unsigned long int);
        void duplicate(char*, char*);
        void clearDictionary();
        
    
    private:				
        typedef std::unordered_map<std::string, IndexNode> t_index;
        t_index strgIndex;
        Index node;       
        
        int deDuplicateSubBlocks(char*, int, int, unsigned long int*); 
        bool nodeExists(std::string);
        int readBlock(std::fstream*, char*, int, int);
        
};

#endif


#endif /* DEDUP_H */ 
