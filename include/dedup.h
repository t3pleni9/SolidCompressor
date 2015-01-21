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

#ifndef DEDUP_H
#define DEDUP_H

using namespace::std;

struct IndexNode {
    int offsetPointer;
    int size;
};

class deDup
{
    public:
        deDup();
        virtual ~deDup();
    
    private:
        typedef unordered_map<string, IndexNode> t_index;
        static t_index index;
        static string getHash(string, SHA_CTX);
        static string readBlock(fstream);
        static int buildNode(int /*index*/);
        
        
};

#endif /* DEDUP_H */ 
