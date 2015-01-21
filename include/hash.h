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
 
#include <openssl/sha.h>
#include <cstring>

#ifndef HASH_H
#define HASH_H

class Hash
{
    public:
        Hash();
        static SHA_CTX getHash(char*, unsigned char*);
        static SHA_CTX getNextHash(char*, unsigned char*, SHA_CTX);
            
    private:
        static SHA_CTX getHash(char*, unsigned char*, SHA_CTX);   

};

#endif /* HASH_H */ 
