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


#ifndef __INDEX_H
#define __INDEX_H

#include <stdio.h>
#include <openssl/sha.h>

#ifdef __cplusplus

#include <string>
#include <cstring>
#include <unordered_map>
#include <map>

#endif

#ifdef __cplusplus
extern "C" {
#endif

struct Node {
    unsigned size : 5;
    unsigned segment : 3;
};

struct IndexNode {
    unsigned int offsetPointer; /**< offset index */
    Node node; /**<  number of blocks*/
};

struct IndexHeader {
    unsigned int offsetPointer;
    unsigned int block;
    unsigned int size;
    unsigned int type : 2;    
};


typedef struct Node Node;
typedef struct IndexNode IndexNode;
typedef struct IndexHeader IndexHeader;

#ifdef __cplusplus
}

class Index{
    
    private:
        unsigned char hashValue[SHA_DIGEST_LENGTH]; /**< Hash value of block(s)  */
        SHA_CTX indexContext;
        IndexHeader parentIndex; /**<  On disk index of block(s)*/ 
        IndexNode index;
        
        typedef std::unordered_map<unsigned int, IndexHeader> t_index;
        static t_index headerIndex;
        
    public:
        Index();
        Index(unsigned int, char*, unsigned int);
        
        virtual ~Index();
        
        IndexHeader getParentIndex();
        IndexNode getIndexNode();   
        std::pair<std::string, IndexNode> getNode();     
        int hashNode(unsigned int, char*, unsigned int);  
        int rehashNode(char*, unsigned int); 
        void setParent(unsigned int, unsigned int);
        
        static int generateIndex(IndexHeader,unsigned int, int, int);   
        static int writeIndex(char*);
        static int readIndex(char*);
        static int getIndexHeader(unsigned int, IndexHeader*);
        static int getHeaderIndexCount();
        static void printIndex();
        static void clearIndex();
};

#endif

#endif /* INDEX_H */ 
