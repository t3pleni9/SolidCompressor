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
    buffer->busy = 1;
    buffer->out_len = deDup.deDuplicate(buffer->in_buffer, buffer->out_buffer, SEG_S);
    buffer->busy = 0;
    if(!buffer->out_buffer) {
        return SDEDUP_NULL_POINTER;
    } else if(!buffer->out_len) {
        return SDEDUP_ERROR;
    }
    
    return SDEDUP_DONE;
}

solid_result diff(SOLID_DATA buffer) {
    buffer->busy = 1;
    diff_result diff = do_diff(buffer->in_buffer, buffer->out_buffer, buffer->in_len, &buffer->out_len);
    buffer->busy = 0;
    if(diff == DIFF_DONE) {
        return SDIFF_DONE;
    } else if(diff == DIFF_NULL_POINTER){
        return SDIFF_NULL_POINTER;
    } else {
        perror(errorMsg);
        return SDIFF_ERROR;
    }
}

solid_result stream_compress(SOLID_DATA buffer) {
    return SSTRM_DONE;
}

solid_result solid_compress(char* inbuffer, char *outbuffer, size_t in_len, size_t * out_len) {
    t_solid_data buffer;
    buffer.in_buffer = inbuffer; 
    buffer.out_buffer = outbuffer; // NULL buffer
    buffer.in_len = in_len;
    buffer.busy = 0;
    t_solid_data diff_buffer;
    
    if(de_dup(&buffer) == SDEDUP_DONE) {
        if(buffer.out_len) 
        printf("Done dedup %d %d\n", buffer.in_len,buffer.out_len);
        else 
        printf("out len is null\n");
        diff_buffer.in_buffer = buffer.out_buffer;
        diff_buffer.out_buffer = outbuffer;
        diff_buffer.in_len = buffer.out_len;
        diff_buffer.busy = 0;
       // memcpy(buffer.in_buffer, buffer.out_buffer, buffer.out_len);
        printf("memcopy is the culprit\n");
        solid_result result = diff(&diff_buffer);
        *out_len = diff_buffer.out_len;
        return result;
    } else {
        return SDEDUP_ERROR;
    }
}

