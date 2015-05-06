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
#include <queue>

#include "dedup.h"
#include "diff.h"

#include "solidlib.h"


std::queue<t_solid_data> compQueue;
pthread_mutex_t lock;


solid_result de_dup(SOLID_DATA buffer) {
    DeDup deDup;
    buffer->out_len = deDup.deDuplicate(buffer->in_buffer, buffer->out_buffer, SEG_S);
    if(!buffer->out_buffer) {
        return SDEDUP_NULL_POINTER;
    } else if(!buffer->out_len) {
        return SDEDUP_ERROR;
    }
    
    return SDEDUP_DONE;
}

solid_result diff(SOLID_DATA buffer) {    
    diff_result diff;
    if(buffer->fd.out != -1) {
        diff = do_diff_fd(buffer->in_buffer, buffer->fd.out, buffer->in_len, &buffer->out_len);
    } else {
        diff = do_diff(buffer->in_buffer, &buffer->out_buffer, buffer->in_len, &buffer->out_len);
    }
    
    switch(diff) {
        case DIFF_DONE: return SDIFF_DONE;
        case DIFF_NULL_POINTER: return SDIFF_NULL_POINTER;
        case DIFF_PIPE_ERROR: return SPIPE_ERROR;
        default: perror(errorMsg);
            return SDIFF_ERROR;
    }
}

solid_result wait_for_finish(pthread_t t_th) {
    void* t_res = NULL;
    int ret = pthread_join(t_th, &t_res );
    if (ret) {
        ret = -ret;
        fprintf(stderr, "ERROR: pthread_join failed: %s\n",
            strerror(-ret));
        return STH_ERROR;
    } 
          
    /*if (t_res ) {
        solid_result retResult = *(solid_result *)t_res ;
        return retResult;
    }*/
    
    return STH_DONE;            
}

void *stream_compress_thread(void *_arg) {
    SOLID_DATA buffer = (SOLID_DATA)_arg;
    t_solid_data qBuffer;
    
    while(1) {
        //printf("Threaded compress");
        pthread_mutex_lock(&lock);  
        if(!compQueue.empty()) {
            qBuffer =  compQueue.front();
            compQueue.pop();
            pthread_mutex_unlock(&lock);
        } else {
            pthread_mutex_unlock(&lock);
            continue;
        }
        
        if(qBuffer.in_len == 0 && qBuffer.out_len == 0 ) {
            printf("got empty\n");
            sleep(2);
            //return (void*)SQU_DONE;
            pthread_exit(0);
        }
        printf("out %d %c\n", qBuffer.id, qBuffer.in_buffer[qBuffer.in_len - 1]);
        buffer->in_buffer = qBuffer.in_buffer;
        buffer->in_len = qBuffer.in_len;
        solid_result result = stream_compress(&qBuffer);
        if( result != SSTRM_DONE) pthread_exit(0);//return (void *)result; 
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
            if(buffer->fd.out != -1) {
                int ret = write_buf(buffer->fd.out, out, have, "STRM 1");
                if (ret < 0) {
                    (void)deflateEnd(&strm);
                    return SPIPE_ERROR;                
                }
            } else {
                if (memcpy((buffer->out_buffer + buffer->out_len), out, have)) {
                    buffer->out_len += have;
                } else {
                    (void)deflateEnd(&strm);
                    return SSTRM_ERROR;
                }
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


void *stream_compress_pipe(void *_args) {
    //sleep(1);
    int readed, ret = 0;
    pthread_t t_que;
    SOLID_DATA buffer = (SOLID_DATA)_args;
    printf("pipe compress %d\n", buffer->fd.in);
    t_solid_data emptyBuffer;
    SOLID_DATA pip_buffer = (SOLID_DATA)malloc(sizeof(t_solid_data));
    pip_buffer->fd.in = buffer->fd.in;
    pip_buffer->fd.out = buffer->fd.out;
    (emptyBuffer.in_len = 0, emptyBuffer.out_len = 0);
    solid_result retResult;
    buffer->id = 0;
    if(buffer->fd.in != -1) {
        ret = pthread_create(&t_que, NULL, stream_compress_thread, pip_buffer);
        if (ret) {
            ret = -ret;
            fprintf(stderr, "ERROR: thread setup failed: %s\n",
                strerror(-ret));
            retResult = STH_ERROR;
            goto pipe_out;
        }
        
        if(!buffer->in_buffer) {
           buffer->in_buffer = (char *)malloc(CHUNK);
        }
        
        while(1) {
            readed = read(buffer->fd.in, buffer->in_buffer, CHUNK);
            
            if( readed < 0) {
                fprintf(stderr, "Error: failed to read stream");
                pthread_exit(0);
                //return (void*)SPIPE_ERROR;
            } else if(!readed) {
                printf("Empty section");
                pthread_mutex_lock(&lock);
                compQueue.push(emptyBuffer);
                pthread_mutex_unlock(&lock);
                printf("Waiting for queue\n");
                //close(buffer->fd.in);
                if((retResult = wait_for_finish(t_que)) != SQU_DONE) {
                    printf("QUE not done\n");
                    //goto pipe_out;
                }
                printf("Done Waiting for queue\n");
                pthread_exit(0);
                //return (void*)SPIPE_DONE;
            } else {
                buffer->in_len = readed;                
                buffer->id++;
                printf("IN %d %d\n", buffer->id, buffer->in_len);
                pthread_mutex_lock(&lock);
                compQueue.push(*buffer);
                pthread_mutex_unlock(&lock);
                
            }
        }
    }
    pipe_out:
    //return (void *)SPIPE_NOT_SET;    
    pthread_exit(0);
}



void *diff_pipe(void *_args) {    
    pthread_t t_pipe;
    int pipefd[2] = {-1, -1};
    int ret = pipe(pipefd);
    if (ret < 0) {
        ret = -errno;
        fprintf(stderr, "ERROR: pipe failed. %s\n", strerror(-ret));
        return (void*)SPIPE_ERROR;
    }
    
    SOLID_DATA strm_buffer = (SOLID_DATA)malloc(sizeof(t_solid_data));
    solid_result retResult;
    diff_result diff;
    SOLID_DATA buffer = (SOLID_DATA)_args;
    strm_buffer->fd.out = buffer->fd.out;
    buffer->fd.out = pipefd[1];
    strm_buffer->fd.in = pipefd[0];
    
    ret = pthread_create(&t_pipe, NULL, stream_compress_pipe, strm_buffer);
    if (ret) {
        ret = -ret;
        fprintf(stderr, "ERROR: thread setup failed: %s\n",
            strerror(-ret));
        retResult = STH_ERROR;
        goto diff_out;
    }
    
    diff = do_diff_fd(buffer->in_buffer, buffer->fd.out, buffer->in_len, &buffer->out_len);
    switch(diff) {
        case DIFF_DONE: retResult = SDIFF_DONE; break;
        case DIFF_NULL_POINTER: retResult = SDIFF_NULL_POINTER; break;
        case DIFF_PIPE_ERROR: retResult = SPIPE_ERROR; break;
        default: perror(errorMsg);
            retResult = SDIFF_ERROR;
    }
    
    printf("Waiting for pipe\n");
    if((retResult = wait_for_finish(t_pipe)) != SPIPE_DONE) {
        printf("PIPE not done\n");
        //goto diff_out;
    }
     printf("Done Waiting for pipe\n");
    
    if(retResult != SDIFF_DONE) 
        goto diff_out;         
               
    diff_out:
        free(strm_buffer);
       /* if (pipefd[0] != -1)
            close(pipefd[0]);
        if (pipefd[1] != -1)
            close(pipefd[1]);
        */pthread_exit(0);
}

solid_result solid_compress_fd(int in_fd, int dump_fd) {
    SOLID_DATA de_dup_buffer, diff_buffer, strm_buffer;
    de_dup_buffer = (SOLID_DATA)malloc(sizeof(t_solid_data));
    diff_buffer = (SOLID_DATA)malloc(sizeof(t_solid_data));
    
    int ret = 0;
    
    void *t_err = NULL;
    solid_result retResult = SPIPE_DONE;
    pthread_t t_diff, t_que, t_pipe;
    int first_run = 1;
    
    while(1) {
        
        if(in_fd > 0 && dump_fd > 0) {
            de_dup_buffer->fd.in = in_fd;
            diff_buffer->fd.out = dump_fd;
        } else {
            fprintf(stderr,"I/O file descriptors not set");
            return SPIPE_NOT_SET;
        }
        
        de_dup_buffer->in_buffer = (char *) malloc(SEG_S);
        int readed = read( de_dup_buffer->fd.in,  de_dup_buffer->in_buffer, SEG_S);
        
        if( readed < 0) {
            fprintf(stderr, "Error: failed to read stream");
            return SPIPE_ERROR;
        } else if(!readed) {  
            if(!first_run) {
                printf("Waiting for diff\n");
                if((retResult = wait_for_finish(t_diff)) != SDIFF_DONE) {
                    printf("DIFF not done\n");
                    //goto out;
                }
                printf("done Waiting for Diff\n"); 
             }
            ret = 0;
            retResult = SPIPE_DONE;
            goto out;             
        } else {
            de_dup_buffer->in_len = readed;
            de_dup_buffer->out_buffer = (char *)malloc((SEG_S + 20));
            
            if(!first_run) {
                printf("Waiting for diff\n");
                if((retResult = wait_for_finish(t_diff)) != SDIFF_DONE) {
                    printf("DIFF not done\n");
                   // goto out;
                }
                printf("done Waiting for Diff\n"); 
            }
            
            if(de_dup(de_dup_buffer) == SDEDUP_DONE) {
                if(de_dup_buffer->out_len) 
                    printf("Done dedup %d %d\n", de_dup_buffer->in_len, de_dup_buffer->out_len);
                else 
                    printf("out len is null\n");
                diff_buffer->in_buffer = (char *) malloc(de_dup_buffer->out_len);
                
                memcpy(diff_buffer->in_buffer, de_dup_buffer->out_buffer, de_dup_buffer->out_len);
                diff_buffer->out_buffer = NULL;
                diff_buffer->in_len = de_dup_buffer->out_len;
                
               /* if(de_dup_buffer->in_buffer)
                    free(de_dup_buffer->in_buffer);
                if(de_dup_buffer->out_buffer)
                    free(de_dup_buffer->out_buffer);
                */de_dup_buffer->in_len = 0;
                de_dup_buffer->out_len = 0;
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
        /*if(de_dup_buffer->in_buffer)
            free(de_dup_buffer->in_buffer);
        if(de_dup_buffer->out_buffer)
            free(de_dup_buffer->out_buffer);
        
        if(diff_buffer->in_buffer)
            free(diff_buffer->in_buffer);
        if(diff_buffer->out_buffer)
            free(diff_buffer->out_buffer);
        
        if(strm_buffer->in_buffer)
            free(strm_buffer->in_buffer);
        if(strm_buffer->out_buffer)
            free(strm_buffer->out_buffer);            
        */// TODO: destroy the lock
        pthread_mutex_destroy(&lock);
       
        return retResult;

}

solid_result solid_compress(char* inbuffer, char *outbuffer, size_t in_len, size_t * out_len) {
    
    /*use semaphores to check if the threads are active, 
     * if not active call them, else wait until it becomes non active again
     * dedup -> check if delt_stream thread is active
     *  if active wait
     *  else call delta_stream thread, and proceed with the next dedup
     *  to save data space, output the stream to a output file handler from anywhere (cmd_send's output file handler would be apt)
     */
    
    t_solid_data buffer;
    buffer.in_buffer = inbuffer; 
    buffer.out_buffer = outbuffer; // NULL buffer
    buffer.in_len = in_len;
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
        diff_buffer.fd.in = -1;
        diff_buffer.fd.out = -1;
       // memcpy(buffer.in_buffer, buffer.out_buffer, buffer.out_len);
        #ifdef _PTHREAD_H
            pthread_t inc_x_thread;
            printf("pthread area\n");
            int ret = pipe(pipefd);
            if (ret < 0) {
                ret = -errno;
                fprintf(stderr, "ERROR: pipe failed. %s\n", strerror(-ret));
                exit(-ret);
            }
            
            diff_buffer.fd.out = pipefd[1];
        #else 
            //Non threaded application
        #endif
        solid_result result = diff(&diff_buffer);
        printf("Diff done %d %d\n", diff_buffer.in_len, diff_buffer.out_len);
        strm_buffer.in_buffer = diff_buffer.out_buffer;
        strm_buffer.out_buffer = outbuffer;
        strm_buffer.in_len = diff_buffer.out_len;
        result = stream_compress(&strm_buffer);
        memcpy(outbuffer, strm_buffer.out_buffer, strm_buffer.out_len);
        *out_len = strm_buffer.out_len;
        return result;
    } else {
        return SDEDUP_ERROR;
    }
}
