/*
 * hash.h
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
 

#ifndef HASH_H
#define HASH_H
 
#include <openssl/sha.h>


#ifdef __cplusplus

#include <cstring>


class Hash
{
    public:
        Hash();
        static SHA_CTX getHash(char*, unsigned char*, unsigned int);
        static SHA_CTX getNextHash(char*, unsigned char*, unsigned int, SHA_CTX);
            
    private:
        static SHA_CTX getHash(char*, unsigned char*, unsigned int, SHA_CTX);   

};

#endif

#endif /* HASH_H */ 
