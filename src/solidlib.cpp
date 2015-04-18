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
    buffer->out_len = deDup.deDuplicate(buffer->in_buffer, buffer->out_buffer, SEG_S);
    if(!buffer->out_buffer) {
        return SDEDUP_NULL_POINTER;
    } else if(!buffer->out_len) {
        return SDEDUP_NOT_DONE;
    }
    
    return SDEDUP_DONE;
}

