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


Index::Index()
{
    
}

Index::~Index() { }

int Index::buildNode(int _l_index, char* block) {
    index.offsetPointer = _l_index;
    index.size = 0;
    indexContext = Hash::getHash(block, (unsigned char*)hashValue);
       
    return 1;
}

int Index::rebuildNode(char* block) {
    index.size++;
    indexContext = Hash::getNextHash(block, (unsigned char*)hashValue, indexContext);
    
    return 1;
}

std::pair<std::string, IndexNode> Index::getNode() {
    char mdString[SHA_DIGEST_LENGTH*2+1];
    
    for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
         sprintf(&mdString[i*2], "%02x", (unsigned int)hashValue[i]);
    std::string digest(mdString);
    
    return (std::pair<std::string, IndexNode>(digest, index));
}

