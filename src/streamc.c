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

//int level =9;

static SOLID_RESULT _bzip2_decompress(int fd_in, int fd_out) {
        //level = 9;
        BZFILE *BZ2fp_r = NULL;
        int len = 0;
        char buff[0x1000];
        if((BZ2fp_r = BZ2_bzdopen(fd_in,"rb"))==NULL){
            strcpy(errorMsg, "Can't bz2openstream");
            return SSTRM_ERROR;
        }
        
        while((len=BZ2_bzread(BZ2fp_r,buff,0x1000))>0){
            if (write_buf(fd_out, buff,len) != len) {
                return SPIPE_ERROR;
            }
        }
         
        BZ2_bzclose(BZ2fp_r);
        return SSTRM_DONE;
}

static SOLID_RESULT _bzip2_compress(int fd_in, int fd_out) {
    //level = 9;
    BZFILE *BZ2fp_w = NULL;
    int len = 0;
    t_solid_data buffer;
    buffer.in_buffer = (char *)malloc(0x1000 * sizeof(char));
    buffer.fd.in = fd_in;
    buffer.fd.out = fd_out;
    
    char mode[10];
    mode[0]='w';
    mode[1] = '0' + level;
    mode[2] = '\0';

    if((BZ2fp_w = BZ2_bzdopen(fd_out,mode))==NULL){
        strcpy(errorMsg, "Can't bz2openstream");
        return SSTRM_ERROR;
    }
    
    while((len=fill_buffer(&buffer, 0x1000))>=0){
        if(buffer.in_len == 0) // No more data to be read. fill_buffer returning 0 means last read.
            break;
        BZ2_bzwrite(BZ2fp_w,buffer.in_buffer,buffer.in_len);
    }
    
    BZ2_bzclose(BZ2fp_w);
    if(buffer.in_buffer)    free(buffer.in_buffer);
    return SSTRM_DONE;
}

static SOLID_RESULT _zlib_compress(int fd_in, int fd_out) {
    int ret, flush;
    unsigned have;
    z_stream strm;
    
    t_solid_data buffer;
    buffer.in_buffer = (char*)malloc(CHUNK);
    buffer.out_buffer = (char *)malloc(CHUNK);
    buffer.fd.in = fd_in;

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return SSTRM_ERROR;

    /* compress until end of file */
    do {
        
        int read = fill_buffer(&buffer, CHUNK);
        if(read < 0) {(void)deflateEnd(&strm); return SPIPE_ERROR;}
        strm.avail_in = buffer.in_len;
        /*if (ferror(source)) {
            (void)deflateEnd(&strm);
            return SSTRM_ERROR;
        }*/
        flush = !read ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = (unsigned char*)buffer.in_buffer;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = (unsigned char *)buffer.out_buffer;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (write_buf(fd_out, buffer.out_buffer, have) != have) {
                (void)deflateEnd(&strm);
                return SPIPE_ERROR;
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

static SOLID_RESULT _zlib_decompress(int fd_in, int fd_out) {
    int ret;
    unsigned have;
    z_stream strm;
    t_solid_data buffer;
    buffer.in_buffer = (char*)malloc(CHUNK);
    buffer.out_buffer = (char *)malloc(CHUNK);
    buffer.fd.in = fd_in;

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        int read = fill_buffer(&buffer, CHUNK);
        if(read < 0) { (void)inflateEnd(&strm); return SPIPE_ERROR;}
        strm.avail_in = buffer.in_len;
        
        if (strm.avail_in == 0)
            break;
        strm.next_in = (unsigned char *)buffer.in_buffer;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = (unsigned char *)buffer.out_buffer;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return SSTRM_ERROR;
            }
            have = CHUNK - strm.avail_out;
            if (write_buf(fd_out, buffer.out_buffer, have) != have) {
                (void)inflateEnd(&strm);
                return SPIPE_ERROR;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? SSTRM_DONE : SSTRM_ERROR;
}


void* zlib_compress(void* _args) {
    SOLID_DATA buffer = (SOLID_DATA)_args;
    if((buffer->end_result = _zlib_compress(buffer->fd.in, buffer->fd.out)) != SSTRM_DONE) {
        fprintf(stderr, "ERROR: Stream compress error - %s\n", errorMsg);
        pthread_exit(&buffer->end_result);
    }   
    pthread_exit(&buffer->end_result);   
}

void* zlib_decompress(void* _args) {
    SOLID_DATA buffer = (SOLID_DATA)_args;
    if((buffer->end_result = _zlib_decompress(buffer->fd.in, buffer->fd.out)) != SSTRM_DONE) {
        fprintf(stderr, "ERROR: Stream compress error - %s\n", errorMsg);
        pthread_exit(&buffer->end_result);
    }  
    close(buffer->fd.out);
    pthread_exit(&buffer->end_result);
}

void* bzip2_compress(void* _args) {
    SOLID_DATA buffer = (SOLID_DATA)_args;
    if((buffer->end_result = _bzip2_compress(buffer->fd.in, buffer->fd.out)) != SSTRM_DONE) {
        fprintf(stderr, "ERROR: Stream compress error - %s\n", errorMsg);
        pthread_exit(&buffer->end_result);
    }  
    close(buffer->fd.out);
    pthread_exit(&buffer->end_result);
}

void* bzip2_decompress(void* _args) {
    SOLID_DATA buffer = (SOLID_DATA)_args;
    if((buffer->end_result = _bzip2_decompress(buffer->fd.in, buffer->fd.out)) != SSTRM_DONE) {
        fprintf(stderr, "ERROR: Stream compress error - %s", errorMsg);
        pthread_exit(&buffer->end_result);
    }  
    close(buffer->fd.out);
    pthread_exit(&buffer->end_result);
}
