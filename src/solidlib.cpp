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
#include "streamc.h"
#include "solidlib.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <unistd.h>


pthread_mutex_t lock;

extern int dfd;
extern int ifd;

ulInt netIn     = 0;
ulInt netOut    = 0;


unsigned int    SEG_S                   = 200000000;
unsigned short  segment_pre_multiplier  = 3;
unsigned short  degree                  = 10;
unsigned short  level                   = 9;

char *__deDupBuffer__ = NULL;
char *__outBuffer__   = NULL;
char *__tempBuffer__  = NULL;
char *__diffBuffer__  = NULL;

MODALGO _scompressor_  = ZLIBC;
MODALGO _delta_        = ZMSTD;
MODALGO _duplicator_   = LZDDP;

__attribute__((destructor)) void unmaplibBuffer() {
    if(__deDupBuffer__) munmap(__deDupBuffer__, SEG_S + 20);
}

/*
 * |-----------segment size (4 bits)------------|----------compression level (4 bits)--------| 0xff000000
 * |-------------------------stream compressor algorithm 8 bits------------------------------| 0x00ff0000
 * |-------------------------Delta compressor algorithm 8 bits ------------------------------| 0x0000ff00
 * |-------------------------De duplication algorithm 8 bits   ------------------------------| 0x000000ff
 */

static void printStats() {
    fprintf(stderr, "Net IN: %lu Net OUT: %lu\n", netIn, netOut);
}

static SOLID_RESULT _set_header(int out_fd) {
    printf("%d\n", sizeof(unsigned int));
    int ret = 0;
    /*unsigned int header = 0;
    header |= (segment_pre_multiplier << 28);
    header |= (level << 24);
    header |= (((int)_scompressor_) << 16);
    header |= (((int)_delta_) << 8);
    header |= (((int)_duplicator_));*/
    if((ret = write_buf(out_fd, (char *)&_scompressor_, sizeof(_scompressor_))) < 0) goto error;
    if((ret = write_buf(out_fd, (char *)&_delta_, sizeof(_delta_))) < 0) goto error;
    if((ret = write_buf(out_fd, (char *)&_duplicator_, sizeof(_duplicator_))) < 0) goto error;
    
    //if((ret = write_buf(out_fd, (char *)&header, sizeof(header))) < 0) goto error;
    //exit(0);
    return S_DONE;
    error:
        fprintf(stderr, "ERROR: Writing file info failed.\n");
        return S_NULL;
}

static SOLID_RESULT _get_header(int in_fd) {
    MODALGO inC;
    int readed = 0;
    unsigned int header = 0;
    if((readed = read( in_fd, (char *)&inC, sizeof(inC))) < 0) {
        fprintf(stderr, "ERROR: Unable to get file header.\n");
        return S_NULL;
    } else {
        _scompressor_ = (MODALGO)((int)inC + 10);
    }
    if((readed = read( in_fd, (char *)&inC, sizeof(inC))) < 0) {
        fprintf(stderr, "ERROR: Unable to get file header.\n");
        return S_NULL;
    } else {
        _delta_ = (MODALGO)((int)inC + 10);
    }
    if((readed = read( in_fd, (char *)&inC, sizeof(inC))) < 0) {
        fprintf(stderr, "ERROR: Unable to get file header.\n");
        return S_NULL;
    } else {
        _duplicator_ = (MODALGO)((int)inC + 10);
    }
    
    /*if((readed = read( in_fd, (char *)&header, sizeof(header))) < 0) {
        fprintf(stderr, "ERROR: Unable to get file header.\n");
        return S_NULL;
    } else {
        _duplicator_ = (MODALGO)((header & 0x000000ff) + 10);
        _delta_ = (MODALGO)(((header & 0x0000ff00) >> 8) + 10);
        _scompressor_ = (MODALGO)(((header & 0x00ff0000) >> 16) + 10);
        segment_pre_multiplier = (header & 0xf0000000) >> 28;
        //degree = (header & 0x0C000000) >> 26;
        level = (header & 0x0f000000 ) >> 24;
        SEG_S = segment_pre_multiplier * 100000000;
        printf ("%d %d %d %d %d %d\n", (int)_duplicator_, (int)_delta_, (int)_scompressor_, segment_pre_multiplier, degree,level);
    }*/
    SEG_S = segment_pre_multiplier * 100000000;
    //exit(0);
    return S_DONE;
    
}

static void * _s_init(SOLID_DATA *buffer) {
    *buffer = (SOLID_DATA)malloc(sizeof(t_solid_data));
    if(*buffer == NULL)
        return NULL;
    (*buffer)->in_buffer  = NULL;
    (*buffer)->in_len     = 0;
    (*buffer)->out_buffer = NULL;
    (*buffer)->out_len    = 0;
    (*buffer)->fd.in      = -1;
    (*buffer)->fd.out     = -1;
    
    switch(_scompressor_) {
        case ZLIBC: (*buffer)->scomp = (zlib_compress);
        break;
        case ZLIBD: (*buffer)->scomp = (zlib_decompress);
        break;
        case BZIPC: (*buffer)->scomp = (bzip2_compress);
        break;
        case BZIPD: (*buffer)->scomp = (bzip2_decompress);
        break;
        default : (*buffer)->scomp = (zlib_compress);
    }
    
    switch(_delta_) {
        case ZDLTA: (*buffer)->dcomp = (zdelta_diff);
        break;
        case ZMSTD: (*buffer)->dcomp = (zmst_diff);
        break;
        case ZDPAT: (*buffer)->dcomp = (zdelta_patch);
        break;
        case ZMSTP: (*buffer)->dcomp = (zdelta_patch);
        break;
        default : (*buffer)->dcomp = (zmst_diff);
    }
    
    switch(_duplicator_) {
        case LZDDP: (*buffer)->dupcomp = (de_dup);
        break;
        case LZDUP: (*buffer)->dupcomp = (duplicate);
        break;
        default : (*buffer)->dupcomp = (de_dup);
    }
    
    return (void *)(*buffer);
}

void *diff_pipe(void *_args) {    
    SOLID_DATA buffer = (SOLID_DATA)_args;    
     
    (buffer->dcomp)(buffer);
    pthread_exit(&buffer->end_result);
}

SOLID_RESULT _solid_compress_fd(int in_fd, int out_fd) {
    SOLID_DATA      de_dup_buffer;    
    SOLID_DATA      diff_buffer;     
     
    SOLID_RESULT    retResult      = SPIPE_DONE;
    int pipefd[2]   = {-1, -1};
    int ret         = 0;
    int first_run   = 1;
    
    pthread_t t_diff;
    if ((__deDupBuffer__ = (char *)mmap (
        (caddr_t)0, SEG_S + 20, PROT_READ | PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS , -1, 0)
        ) == (caddr_t) -1)
        fprintf (stderr, "mmap error for deDupBuffer\n");
    
     /*Initialize buffers and function pointers */
    if(_s_init(&de_dup_buffer) == NULL) {
        return S_NULL;
    }
    
    if(_s_init(&diff_buffer) == NULL) {
        return S_NULL;
    }
    
    ret = pipe(pipefd);    
    if (ret < 0) {
        ret = -errno;
        fprintf(stderr, "ERROR: pipe failed. %s\n", strerror(-ret));
        retResult = SPIPE_ERROR;
        goto out;
    }
    
    if(in_fd > 0 && out_fd > 0) {
        de_dup_buffer->fd.in    = in_fd;
        de_dup_buffer->fd.out   = pipefd[1];
        diff_buffer->fd.out     = out_fd;
        diff_buffer->fd.in      = pipefd[0];
        
    } else {
        fprintf(stderr,"I/O file descriptors not set");
        return SPIPE_NOT_SET;
    }    
    
    ret = pthread_create(&t_diff, NULL, diff_pipe, diff_buffer);                
    if (ret) {
        ret = -ret;
        fprintf(stderr, "ERROR: thread setup failed: %s\n",
            strerror(-ret));
        retResult = STH_ERROR;
        goto out;
    }
    
    while(1) {
        
        if(de_dup_buffer->in_buffer)  de_dup_buffer->in_buffer = NULL;
        de_dup_buffer->in_buffer = __deDupBuffer__;
        de_dup_buffer->in_len = 0;
        
        int readed = fill_buffer(de_dup_buffer, SEG_S);

        if( readed < 0) {
            fprintf(stderr, "Error: failed to read stream, %s\n",  strerror(errno));
            return SPIPE_ERROR;
        } else if(!de_dup_buffer->in_len) {  
            ret         = 0;
            retResult   = S_DONE;
            goto out;
                         
        } else {
            de_dup_buffer->out_buffer = NULL;
            if(*((SOLID_RESULT *)(de_dup_buffer->dupcomp)(de_dup_buffer)) == SDEDUP_DONE) {
                ret = write_buf(de_dup_buffer->fd.out, (char *)&(de_dup_buffer->out_len), sizeof(de_dup_buffer->out_len));
                if (ret < 0) {
                    close(out_fd);
                    strcpy(errorMsg,"DeDup: Unable to write buffer length. ");
                    strcat(errorMsg, strerror(-ret));
                    goto out;
                }
                
                ret = write_buf(de_dup_buffer->fd.out, de_dup_buffer->out_buffer, de_dup_buffer->out_len);
                if (ret < 0) {
                    close(out_fd);
                    strcpy(errorMsg,"DeDup: Unable to write buffer. ");
                    strcat(errorMsg, strerror(-ret));
                    goto out;
                }
                
                de_dup_buffer->out_buffer   = NULL;
                de_dup_buffer->in_buffer    = NULL;
                de_dup_buffer->in_len       = 0;
                de_dup_buffer->out_len      = 0;
                
                first_run = 0;
            }
        }
    }
    
    out:
        if (pipefd[1] != -1)
            close(pipefd[1]); 
        if((retResult = wait_for_finish(t_diff)) != SDIFF_DONE) {
            fprintf(stderr, "ERROR: Diff thread not done\n");
        }
        
        if(retResult == SDIFF_DONE) retResult = S_DONE;
        
        if (pipefd[0] != -1)
            close(pipefd[0]); 
        close(out_fd);
        if(de_dup_buffer->in_buffer) de_dup_buffer->in_buffer = NULL;
        if(de_dup_buffer->out_buffer)
            free(de_dup_buffer->out_buffer);           
        if(de_dup_buffer)
            free(de_dup_buffer);
        pthread_mutex_destroy(&lock);
        if(__deDupBuffer__) munmap(__deDupBuffer__, SEG_S + 20);
        printStats();
        return retResult;

}

SOLID_RESULT solid_compress_fd(int in_fd, int dump_fd) {
    dfd = dump_fd;
    ifd = in_fd;
    
    SEG_S = segment_pre_multiplier * 100000000;
    int pipefd[2]   = {-1, -1};
    pthread_t t_diff;
    SOLID_DATA buffer;
    SOLID_RESULT retResult;
    
    int ret = pipe(pipefd);    
    if (ret < 0) {
        ret = -errno;
        fprintf(stderr, "ERROR: pipe failed. %s\n", strerror(-ret));
        buffer->end_result = SPIPE_ERROR;
        goto out;
    }   
    
    if(_s_init(&buffer) == NULL) {
        return S_NULL;
    }
    
    if ((__outBuffer__ = (char *)mmap ((caddr_t)0, SEG_S + 20, PROT_READ | PROT_WRITE,
   MAP_SHARED|MAP_ANONYMOUS , -1, 0)) == (caddr_t) -1) {
        fprintf (stderr, "Error: Unable to allocate memory pages, DEDUP\n");
        exit(1);
    }
    
    if ((__diffBuffer__ = (char *)mmap ((caddr_t)0, SEG_S + 20, PROT_READ | PROT_WRITE,
   MAP_SHARED|MAP_ANONYMOUS , -1, 0)) == (caddr_t) -1) {
        fprintf (stderr, "Error: Unable to allocate memory pages, DELTA\n");
        exit(1);
    }
    
    buffer->fd.out = dump_fd;
    buffer->fd.in  = pipefd[0];
    
    if(_set_header(dump_fd) == S_NULL) return S_NULL;
    
    ret = pthread_create(&t_diff, NULL, buffer->scomp, buffer);                
    if (ret) {
        ret = -ret;
        fprintf(stderr, "ERROR: thread setup failed: %s\n",
            strerror(-ret));
        retResult = STH_ERROR;
        goto out;
    }
    
    if((retResult = _solid_compress_fd(in_fd, pipefd[1])) != S_DONE) {
        fprintf(stderr, "ERROR: Main compression thread not done\n");
        goto out;
    }
    if((retResult = wait_for_finish(t_diff)) != SSTRM_DONE) {
        fprintf(stderr, "ERROR: Stream compress thread not done\n");
        goto out;
    }
    
    out:
    free(buffer);
    if (pipefd[0] != -1)
        close(pipefd[0]); 
    if(__outBuffer__) munmap(__outBuffer__, SEG_S + 20); 
    if(__diffBuffer__) munmap(__diffBuffer__, SEG_S + 20);
    return retResult;

}

SOLID_RESULT solid_de_compress_fd(int in_fd, int out_fd) {
    dfd = out_fd;
    ifd = in_fd;
    int pipefd[2]   = {-1, -1};
    pthread_t t_diff;
    SOLID_DATA buffer;
    SOLID_DATA diff_buffer;
    SOLID_RESULT retResult;
    
    if(_get_header(in_fd) == S_NULL) return S_NULL;
    
    int ret = pipe(pipefd);    
    if (ret < 0) {
        ret = -errno;
        fprintf(stderr, "ERROR: pipe failed. %s\n", strerror(-ret));
        buffer->end_result = SPIPE_ERROR;
        goto out;
    }   
    
    if(_s_init(&buffer) == NULL) {
        return S_NULL;
    }
    
    if(_s_init(&diff_buffer) == NULL) {
        return S_NULL;
    }
    
    if ((__outBuffer__ = (char *)mmap ((caddr_t)0, SEG_S + 20, PROT_READ | PROT_WRITE,
   MAP_SHARED|MAP_ANONYMOUS , -1, 0)) == (caddr_t) -1) {
        fprintf (stderr, "Error: Unable to allocate memory pages, DEDUP\n");
        exit(1);
    }
    
    if ((__diffBuffer__ = (char *)mmap ((caddr_t)0, SEG_S + 20, PROT_READ | PROT_WRITE,
   MAP_SHARED|MAP_ANONYMOUS , -1, 0)) == (caddr_t) -1) {
        fprintf (stderr, "Error: Unable to allocate memory pages, DELTA\n");
        exit(1);
    }
    
    buffer->fd.in = in_fd;
    buffer->fd.out  = pipefd[1];
    
    diff_buffer->fd.in = pipefd[0];
    diff_buffer->fd.out = out_fd;
    
    ret = pthread_create(&t_diff, NULL, buffer->scomp, buffer);                
    if (ret) {
        ret = -ret;
        fprintf(stderr, "ERROR: thread setup failed: %s\n",
            strerror(-ret));
        retResult = STH_ERROR;
        goto out;
    }
    
    (buffer->dcomp)(diff_buffer);
    
    if((retResult = wait_for_finish(t_diff)) != SSTRM_DONE) {
        fprintf(stderr, "ERROR: Stream compress thread not done\n");
        goto out;
    }
    out:
    free(buffer);
    if (pipefd[0] != -1)
        close(pipefd[0]);
    close(out_fd); 
    if(__outBuffer__) munmap(__outBuffer__, SEG_S + 20);
    if(__diffBuffer__) munmap(__diffBuffer__, SEG_S + 20);
    printStats();
    return retResult;

}

void *solid_de_comp_thread(void *args_) {
	file_d *fd = (file_d *)args_;
	SOLID_DATA buffer;
	_s_init(&buffer);
	buffer->end_result = solid_de_compress_fd(fd->in, fd->out);
	close(fd->out);
	pthread_exit(&buffer->end_result);
}
