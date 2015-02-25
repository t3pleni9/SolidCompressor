/*
 * index.h
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

#ifndef INDEX_H
#define INDEX_H

/**
 * @brief
 * Stores the index of block(s) on the file.
 */
struct IndexNode {
    unsigned int offsetPointer; /**< offset index */
    unsigned int size; /**<  number of blocks*/
};


class Index{
    
    private:
        unsigned char hashValue[SHA_DIGEST_LENGTH]; /**< Hash value of block(s)  */
        SHA_CTX indexContext;
        IndexNode index; /**<  On disk index of block(s)*/ 
        unsigned int parentBlock;
        unsigned int parentSize;
        
    public:
        Index();
        Index(unsigned int, char*, unsigned int);
        
        virtual ~Index();
        
        int hashNode(unsigned int, char*, unsigned int);  
        int rehashNode(char*, unsigned int); 
        std::pair<std::string, IndexNode> getNode();   
        int generateIndex(int);
        
        void setParent(unsigned int, unsigned int);
};

#endif /* INDEX_H */ 
