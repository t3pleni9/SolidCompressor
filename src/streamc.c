/*
 * streamc.c
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
 

#include "streamc.h"
#include <string.h>

int level = Z_DEFAULT_COMPRESSION;

SOLID_RESULT zlib_compress(char *in_buffer, char **out_buffer, size_t in_len, size_t *out_len) {
    char *temp_buffer = (char *)malloc(sizeof(char)*in_len);
    if(!temp_buffer) {
        strcpy(errorMsg, "Unable to allocate memory for temporary storage.");
        return SSTRM_NULL_POINTER;
    }
    
    int ret, flush, in_offset = 0;
    unsigned have;
    z_stream strm;
    //unsigned char in[CHUNK];
    unsigned char out[CHUNK];
        
    *out_len = 0;
    
    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return SSTRM_ERROR; // use cases for ret after compile
    netIn += in_len;
    
    /* compress until end of file */
    do {
         if (!(in_buffer + in_offset)) {
            (void)deflateEnd(&strm);
            return SSTRM_ERROR;
        }
        strm.avail_in = in_len > CHUNK ? CHUNK : in_len;
        in_len = in_len > CHUNK ? in_len - CHUNK : 0;
        
       
        flush = !in_len ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = (Bytef*)(in_buffer + in_offset);
        in_offset += in_len > CHUNK ? CHUNK : in_len;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            
            if (memcpy((temp_buffer + *out_len), out, have)) {
                *out_len += have;
            } else {
                (void)deflateEnd(&strm);
                return SSTRM_ERROR;
            }
                   
            netOut += have;
            
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    if((*out_buffer = (char *)malloc(sizeof(char)*(*out_len))) == NULL) {
        strcpy(errorMsg, "Unable to allocate memory to out buffer.");
        return SSTRM_NULL_POINTER;
    }
    
    if(!memcpy(*out_buffer, temp_buffer, *out_len)) {
        strcpy(errorMsg, "Unable to copy to out buffer.");
        return SSTRM_NULL_POINTER;
    }
    
    if(temp_buffer)
        free(temp_buffer);
        
    return SSTRM_DONE;
}
