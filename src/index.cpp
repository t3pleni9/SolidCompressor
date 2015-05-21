/*
 * index.cpp
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
#include "hash.h"

#include <iostream>
#include <assert.h>
#include <fstream>

Index::t_index Index::headerIndex = t_index();

Index::Index()
{
    parentIndex.block = 0;
    parentIndex.size = 0;
}

Index::Index(unsigned int _l_index, char* block, unsigned int blockLen) {
    parentIndex.block = 0;
    parentIndex.size = 0;
    hashNode(_l_index, block, blockLen);
}

Index::~Index() { }

void Index::setParent(unsigned int pBlock, unsigned int pSize) {
    parentIndex.block = pBlock;
    parentIndex.size = pSize;
}

int Index::hashNode(unsigned int _l_index, char* block, unsigned int blockLen) {
    index.offsetPointer = _l_index;
    index.node.size = 0;
    indexContext = Hash::getHash(block, (unsigned char*)hashValue, blockLen);
       
    return 1; //TODO: modify for errors
}

int Index::rehashNode(char* block, unsigned int blockLen) {
    index.node.size++;
    indexContext = Hash::getNextHash(block, (unsigned char*)hashValue, blockLen, indexContext);
    
    return 1; //TODO: modify for errors
}

std::pair<std::string, IndexNode> Index::getNode() {
    char mdString[SHA_DIGEST_LENGTH*2+1];
    
    for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
         sprintf(&mdString[i*2], "%02x", (unsigned int)hashValue[i]);
    std::string digest(mdString);
    
    return (std::pair<std::string, IndexNode>(digest, index));
}

IndexHeader Index::getParentIndex() {
    return parentIndex;
}

IndexNode Index::getIndexNode() {
    return index;
}

int Index::generateIndex(IndexHeader node,unsigned int curBlock, int type, int buffer=0) {
    
    node.offsetPointer = curBlock;
    node.type = type;
    if(!type) {
        node.size = buffer;
        node.block = 0;
    }
    
    assert (Index::headerIndex.find (curBlock) == Index::headerIndex.end());                            
    Index::headerIndex.insert(std::pair<unsigned int, IndexHeader>(curBlock, node));
    
    return 0;
}

int Index::writeIndex(char* index) {
    
    unsigned int offset = 0;
    unsigned int size = 0;
    unsigned short int intSize = sizeof(unsigned int);
    for(auto node:Index::headerIndex) {
        memcpy((index + offset + intSize), (char*)&node.second, sizeof(IndexHeader));
        offset += sizeof(IndexHeader);
        size++;
    }
    
    Index::headerIndex.clear();
    memcpy((index), (char*)&size, sizeof(unsigned int));
    //0: Error, nonzero: no Error
    return (offset + intSize);
}

void Index::clearIndex() {
	Index::headerIndex.clear();
}

int Index::readIndex(char* index) {
    
    unsigned int offset = 0;
    unsigned int index_s = 0;
    IndexHeader node;
    memcpy((char*)&index_s, index, sizeof(unsigned int));
    offset = index_s ? sizeof(unsigned int) :  0;
    
    while(index_s) {
        memcpy((char*)&node, (index + offset), sizeof(IndexHeader));
        offset += sizeof(IndexHeader);
        Index::headerIndex.insert(std::pair<unsigned int, IndexHeader>(node.offsetPointer, node));
        index_s--;
    }
    
    //1: No error, 0: error
    return offset;
}

int Index::getIndexHeader(unsigned int key, IndexHeader* node) {
    std::unordered_map<unsigned int,IndexHeader>::const_iterator got = Index::headerIndex.find (key);
    if(got == Index::headerIndex.end()) {
        return 0;
    } else {
        *node = got->second;
        return 1;
    }
}

int Index::getHeaderIndexCount() {
    /*unsigned int count = 0;
    for(auto node:Index::headerIndex) {
        count++;
    }*/
   
    return Index::headerIndex.size();//count;
}

void Index::printIndex() {
    std::map<unsigned int, IndexHeader> ordered(Index::headerIndex.begin(), Index::headerIndex.end());
    for ( auto it : ordered )
        std::cout << " " << it.first << " " << it.second.block<<" " << it.second.size<<" " << it.second.type<< std::endl;
}

