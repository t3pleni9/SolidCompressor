/*
 * dedup.cpp
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



#include "dedup.h"



deDup::deDup()
{
    
}


deDup::~deDup()
{
    
}

string deDup::getHash(string block, SHA_CTX context) {
    
    /**
     * 
     * Do SHA1_Init(&context) when reading a completely new set of blocks.
     * Pass by value preserves the original value of context 
     * between SHA1_Final(...) calls.
     * 
     **
    
    string digest;
    SHA1_Update(&context, (void*)&block, strlen(block));
    SHA1_Final((unsigned char*)&digest, &context); 
    return digest; */
}

