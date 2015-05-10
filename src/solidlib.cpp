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


pthread_mutex_t lock;

ulInt netIn         = 0;
ulInt netOut        = 0;

MODALGO _scompressor_  = ZLIBC;
MODALGO _delta_        = ZDLTA;
MODALGO _duplicator_   = LZDDP;


static void printStats() {
    char in[10], out[10];
    ulInt sizes[] = {1, 1000, 1000000, 1000000000 };
    char post[5] = {"BKMG"};
    int i = 0;
    if(netIn) {
        for(;i<4;i++) {
            if(netIn < sizes[i]) {
                sprintf(in, "%.02f%c", (double)netIn / sizes[i-1], post[i-1]);
                break;
            }
        }
    } else {
        sprintf(in, "%luB", netIn);
    }
    if(netOut) {
        for(i = 0;i<4;i++) {
            if(netOut < sizes[i]) {
                sprintf(out, "%.02f%c", (double)netOut / sizes[i-1], post[i-1]);
                break;
            }
        }
    } else {
        sprintf(in, "%luB", netOut);
    }
    
    fprintf(stderr, "Net IN: %s Net OUT: %s\n", in, out);
}

static SOLID_RESULT _set_header(int out_fd) {
    int ret = 0;
    if((ret = write_buf(out_fd, (char *)&_scompressor_, sizeof(_scompressor_))) < 0) goto error;
    if((ret = write_buf(out_fd, (char *)&_delta_, sizeof(_delta_))) < 0) goto error;
    if((ret = write_buf(out_fd, (char *)&_duplicator_, sizeof(_duplicator_))) < 0) goto error;
    
    return S_DONE;
    
    error:
        fprintf(stderr, "ERROR: Writing file info failed.\n");
        return S_NULL;
}

static SOLID_RESULT _get_header(int in_fd) {
    MODALGO inC;
    int readed = 0;
    if((readed = read( in_fd, (char *)&inC, sizeof(inC))) < 0) {
        fprintf(stderr, "ERROR: Unable to get file header.\n");
        return S_NULL;
    } else {
        _scompressor_ = (MODALGO)((int)inC + 100);
    }
    if((readed = read( in_fd, (char *)&inC, sizeof(inC))) < 0) {
        fprintf(stderr, "ERROR: Unable to get file header.\n");
        return S_NULL;
    } else {
        _delta_ = (MODALGO)((int)inC + 100);
    }
    if((readed = read( in_fd, (char *)&inC, sizeof(inC))) < 0) {
        fprintf(stderr, "ERROR: Unable to get file header.\n");
        return S_NULL;
    } else {
        _duplicator_ = (MODALGO)((int)inC + 100);
    }
    
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
        default : (*buffer)->scomp = (zlib_compress);
    }
    
    switch(_delta_) {
        case ZDLTA: (*buffer)->dcomp = (zdelta_diff);
        break;
        case ZDPAT: (*buffer)->dcomp = (zdelta_patch);
        break;
        default : (*buffer)->dcomp = (zdelta_diff);
    }
    
    switch(_duplicator_) {
        case LZDDP: (*buffer)->dupcomp = (de_dup);
        break;
        default : (*buffer)->dupcomp = (de_dup);
    }
    
    return (void *)(*buffer);
}
/*void test_node() {
    SOLID_DATA node1 = (SOLID_DATA)malloc(sizeof(t_solid_data));
    
    char temp[] = "Hello I am dodo.";
    char temp2[] = "Booo you brutus";
    node1->in_buffer = (char *)malloc(strlen(temp));
    memcpy(node1->in_buffer, temp, strlen(temp));
    node1->in_len = strlen(temp);
    node1->out_buffer = temp2;
    node1->out_len = strlen(temp2);
    node1->id = 23;
    node1->fd.in = 1;
    node1->fd.out = 2;
    compQueue.push(node1);
    t_solid_data node2 = *((SOLID_DATA)compQueue.pop());
    printf("InB: %s Len: %d OutB: %s Len: %d Id: %d FdIn: %d FdOut: %d\n", node2.in_buffer, node2.in_len, node2.out_buffer, node2.out_len, node2.id, node2.fd.in, node2.fd.out);
}*/

SOLID_RESULT wait_for_finish(pthread_t t_th) {
    void* t_res = NULL;
    int ret     = pthread_join(t_th, &t_res );
    if (ret) {
        ret = -ret;
        fprintf(stderr, "ERROR: pthread_join failed: %s\n",
            strerror(-ret));
        return STH_ERROR;
    } 
          
    if (t_res ) {
        SOLID_RESULT retResult = *(SOLID_RESULT *)t_res ;
        return retResult;
    }
       
    return STH_DONE;
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
    
    int ret         = 0;
    int first_run   = 1;
    
    pthread_t t_diff;
    
     /*Initialize buffers and function pointers */
    if(_s_init(&de_dup_buffer) == NULL) {
        return S_NULL;
    }
    
    if(_s_init(&diff_buffer) == NULL) {
        return S_NULL;
    }
    
    if(in_fd > 0 && out_fd > 0) {
        de_dup_buffer->fd.in    = in_fd;
        diff_buffer->fd.out     = out_fd;
    } else {
        fprintf(stderr,"I/O file descriptors not set");
        return SPIPE_NOT_SET;
    }    
    
    while(1) {
        de_dup_buffer->in_buffer = (char *) malloc(SEG_S);
        int readed = fill_buffer(de_dup_buffer, SEG_S);
        netIn += de_dup_buffer->in_len;
        if( readed < 0) {
            fprintf(stderr, "Error: failed to read stream");
            return SPIPE_ERROR;
        } else if(!de_dup_buffer->in_len) {  
            if(!first_run) {
                if((retResult = wait_for_finish(t_diff)) != SDIFF_DONE) {
                    fprintf(stderr, "ERROR: Diff thread not done\n");
                    goto out;
                }
            }
                         
            ret         = 0;
            retResult   = S_DONE;
            goto out;             
        } else {
            de_dup_buffer->out_buffer = NULL;
            if((de_dup_buffer->dupcomp)(de_dup_buffer) == SDEDUP_DONE) {
                if(!first_run) {                        
                    if((retResult = wait_for_finish(t_diff)) != SDIFF_DONE) {
                        fprintf(stderr, "ERROR: Diff thread not done\n");
                        goto out;
                    }                        
                }
                
                diff_buffer->in_buffer = (char *) malloc(de_dup_buffer->out_len);                
                memcpy(diff_buffer->in_buffer, de_dup_buffer->out_buffer, de_dup_buffer->out_len);
                diff_buffer->out_buffer = NULL;
                diff_buffer->in_len = de_dup_buffer->out_len;
                
                if(de_dup_buffer->in_buffer)    free(de_dup_buffer->in_buffer);                    
                if(de_dup_buffer->out_buffer)   free(de_dup_buffer->out_buffer);
                
                de_dup_buffer->out_buffer   = NULL;
                de_dup_buffer->in_buffer    = NULL;
                de_dup_buffer->in_len       = 0;
                de_dup_buffer->out_len      = 0;
                
                ret = pthread_create(&t_diff, NULL, diff_pipe, diff_buffer);                
                if (ret) {
                    ret = -ret;
                    fprintf(stderr, "ERROR: thread setup failed: %s\n",
                        strerror(-ret));
                    retResult = STH_ERROR;
                    goto out;
                }
                
                first_run = 0;
            }
        }
    }
    
    out:
        close(out_fd);
        if(de_dup_buffer->in_buffer)
            free(de_dup_buffer->in_buffer);
        if(de_dup_buffer->out_buffer)
            free(de_dup_buffer->out_buffer);           
        if(de_dup_buffer)
            free(de_dup_buffer);
        pthread_mutex_destroy(&lock);
       
        printStats();
        return retResult;

}

SOLID_RESULT solid_compress_fd(int in_fd, int dump_fd) {
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
    return retResult;

}

SOLID_RESULT solid_de_compress_fd(int in_fd, int out_fd) {
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
    
    buffer->fd.in = in_fd;
    buffer->fd.out  = pipefd[1];
    
    diff_buffer->fd.in = pipefd[0];
    
    
    
    ret = pthread_create(&t_diff, NULL, buffer->scomp, buffer);                
    if (ret) {
        ret = -ret;
        fprintf(stderr, "ERROR: thread setup failed: %s\n",
            strerror(-ret));
        retResult = STH_ERROR;
        goto out;
    }
    
    (buffer->dcomp)(diff_buffer);
    
    /*
     * For dedup and delta, keep a common buffer, (aka reader writter, diff is
     * writer, whereas dedup is reader. Write out dedup output to dump_fd. no storage required.
     */
    
    /*if((retResult = _solid_compress_fd(in_fd, pipefd[1])) != S_DONE) {
        fprintf(stderr, "ERROR: Main compression thread not done\n");
        goto out;
    }
    if((retResult = wait_for_finish(t_diff)) != SSTRM_DONE) {
        fprintf(stderr, "ERROR: Stream compress thread not done\n");
        goto out;
    }
    */
    out:
    free(buffer);
    if (pipefd[0] != -1)
        close(pipefd[0]); 
    return retResult;

}

/*
 * solid_de_compress_fd(....) {
 *  while(!EOF) {
 * Thread it where ever you feel good about threading
 *      x = read sizeof(size_t)
 *      buffer->in_buffer = read next x;
 *      buffer->scomp = (decompress);
 *      stream_compress(buffer);
 *      memcpy(buffer->in_buffer, buffer->out_buffer, buffer->out_len)
 *      patch(buffer)
 *      memcpy(buffer->in_buffer, buffer->out_buffer, buffer->out_len)
 *      duplicate(buffer);
 *      write_buf(buffer->out_len);
 *  }       
 * }
 */

/*SOLID_RESULT solid_compress(char* inbuffer, char *outbuffer, size_t in_len, size_t * out_len) {
    
    t_solid_data buffer;
    buffer.in_buffer = inbuffer; 
    buffer.out_buffer = outbuffer; // NULL buffer
    buffer.in_len = in_len;
    t_solid_data diff_buffer, strm_buffer;
    DeDup deDup;
    if(deDup.de_dup(&buffer) == SDEDUP_DONE) {
        if(buffer.out_len) 
        printf("Done dedup %d %d\n", buffer.in_len,buffer.out_len);
        else 
        printf("out len is null\n");
        diff_buffer.in_buffer = buffer.out_buffer;
        diff_buffer.out_buffer = NULL;
        diff_buffer.in_len = buffer.out_len;
        diff_buffer.fd.in = -1;
        diff_buffer.fd.out = -1;
       // memcpy(buffer.in_buffer, buffer.out_buffer, buffer.out_len);
        SOLID_RESULT result = zdelta_diff(&diff_buffer);
        printf("Diff done %d %d\n", diff_buffer.in_len, diff_buffer.out_len);
        strm_buffer.in_buffer = diff_buffer.out_buffer;
        strm_buffer.out_buffer = outbuffer;
        strm_buffer.in_len = diff_buffer.out_len;
        strm_buffer.fd.out = -1;
        strm_buffer.fd.in = -1;
        result = (strm_buffer.scomp)(&strm_buffer);
        *out_len = strm_buffer.out_len;
        return result;
    } else {
        return SDEDUP_ERROR;
    }
}*/
