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

#include <zlib.h>

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
    diff_result diff;
    if(buffer->fd.out != -1) {
        diff = do_diff_fd(buffer->in_buffer, buffer->fd.out, buffer->in_len, &buffer->out_len);
    } else {
        diff = do_diff(buffer->in_buffer, &buffer->out_buffer, buffer->in_len, &buffer->out_len);
    }
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
    int ret, flush, in_offset = 0;
    unsigned have;
    z_stream strm;
    //unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    int level = Z_DEFAULT_COMPRESSION;
    
    buffer->out_len = 0;

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return SSTRM_ERROR; // use cases for ret after compile

    /* compress until end of file */
    do {
         if (!(buffer->in_buffer + in_offset)) {
            (void)deflateEnd(&strm);
            return SSTRM_ERROR;
        }
        strm.avail_in = buffer->in_len > CHUNK ? CHUNK : buffer->in_len;
        buffer->in_len = buffer->in_len > CHUNK ? buffer->in_len - CHUNK : 0;
        
       
        flush = !buffer->in_len ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = (Bytef*)(buffer->in_buffer + in_offset);
        in_offset += buffer->in_len > CHUNK ? CHUNK : buffer->in_len;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            
            if (memcpy((buffer->out_buffer + buffer->out_len), out, have)) {
                buffer->out_len += have;
            } else {
                (void)deflateEnd(&strm);
                return SSTRM_ERROR;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return SSTRM_DONE;
}

solid_result solid_compress(char* inbuffer, char *outbuffer, size_t in_len, size_t * out_len) {
    t_solid_data buffer;
    buffer.in_buffer = inbuffer; 
    buffer.out_buffer = outbuffer; // NULL buffer
    buffer.in_len = in_len;
    buffer.busy = 0;
    int pipefd[2] = {-1, -1};
    t_solid_data diff_buffer, strm_buffer;
    if(de_dup(&buffer) == SDEDUP_DONE) {
        if(buffer.out_len) 
        printf("Done dedup %d %d\n", buffer.in_len,buffer.out_len);
        else 
        printf("out len is null\n");
        diff_buffer.in_buffer = buffer.out_buffer;
        diff_buffer.out_buffer = NULL;
        diff_buffer.in_len = buffer.out_len;
        diff_buffer.busy = 0;
        diff_buffer.fd.in = -1;
        diff_buffer.fd.out = -1;
       // memcpy(buffer.in_buffer, buffer.out_buffer, buffer.out_len);
        int ret = pipe(pipefd);
        if (ret < 0) {
            ret = -errno;
            fprintf(stderr, "ERROR: pipe failed. %s\n", strerror(-ret));
            exit(-ret);
        }
        
        diff_buffer.fd.out = pipefd[1];

        solid_result result = diff(&diff_buffer);
        printf("Diff done %d %d\n", diff_buffer.in_len, diff_buffer.out_len);
        /*strm_buffer.in_buffer = diff_buffer.out_buffer;
        strm_buffer.out_buffer = outbuffer;
        strm_buffer.in_len = diff_buffer.out_len;
        strm_buffer.busy = 0;
        result = stream_compress(&strm_buffer);*/
        memcpy(outbuffer, strm_buffer.out_buffer, strm_buffer.out_len);
        *out_len = strm_buffer.out_len;
        return result;
    } else {
        return SDEDUP_ERROR;
    }
}

