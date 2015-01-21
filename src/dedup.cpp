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



DeDup::DeDup()
{
	
}


DeDup::~DeDup()
{
    
}


SHA_CTX DeDup::getHash(string block, unsigned char* digest, SHA_CTX context) {
    
    /**
     * 
     * Do SHA1_Init(&context) when reading a completely new set of blocks.
     * Pass by value preserves the original value of context 
     * between SHA1_Final(...) calls.
     * 
     **/
    
    char *block_char;
    SHA_CTX tempContext;
    block_char = const_cast<char*>(block.c_str());
    SHA1_Update(&context, block_char, block.length());
    tempContext = context;
    SHA1_Final(digest, &context); 
    
    return tempContext;
}


SHA_CTX DeDup::getHash(string block, unsigned char* digest) {
	SHA_CTX context;
	SHA1_Init(&context);
	context = getHash(block, digest, context);	
	
	return context;
}


SHA_CTX DeDup::getNextHash(string block, unsigned char* digest, SHA_CTX context) {
	context = getHash(block, digest, context);
	
	return context;
}
