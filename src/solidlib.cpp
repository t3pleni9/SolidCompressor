/*
 * solidlib.cpp
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
#include "diff.h"

#include "solidlib.h"

solid_result de_dup(SOLID_DATA buffer) {
        DeDup deDup;
        char * buffer1;
        char * outBuffer = new char[2*SEG_S];
        char * tempBuffer; 
        //std::ifstream file ("/home/justin/temp4", std::ifstream::binary);
        buffer1 = new char[SEG_S];
        //file.read(buffer, SEG_S);
        size_t outLen = 0;
        unsigned long int fileSize = deDup.deDuplicate(buffer1, outBuffer, SEG_S);
        //std::cout<<fileSize<<std::endl;
}
