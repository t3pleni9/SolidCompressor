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
#include "queue.h"
#include "streamc.h"
#include "solidlib.h"


//6000000

Queue compQueue;
pthread_mutex_t lock;

ulInt netIn         = 0;
ulInt netOut        = 0;

char _scompressor_  = 'z';
char _delta_        = 'z';
char _duplicator_   = 'l';


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

static void* _s_init(SOLID_DATA *buffer) {
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
        case 'z': (*buffer)->scomp = (zlib_compress);
        break;
        default : (*buffer)->scomp = (zlib_compress);
    }
    
    switch(_delta_) {
        case 'z': (*buffer)->dcomp = (zdelta_diff);
        break;
        default : (*buffer)->dcomp = (zdelta_diff);
    }
    
    switch(_duplicator_) {
        case 'l': (*buffer)->dupcomp = (de_dup);
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

void *stream_compress_thread(void *_arg) {    
    SOLID_DATA qBuffer;
    
    SOLID_DATA buffer   = (SOLID_DATA)_arg;
    buffer->in_buffer   = NULL;
    buffer->in_len      = 0;
        
    while(1) {        
        pthread_mutex_lock(&lock);  
        if(!compQueue.empty()) {
            qBuffer =  (SOLID_DATA)compQueue.pop();
            pthread_mutex_unlock(&lock);
        } else {
            pthread_mutex_unlock(&lock);
            continue;
        }
        
        if(!qBuffer) {
            buffer->end_result = SSTRM_NULL_POINTER;
            fprintf(stderr, "ERROR: Null returned from queue.\n");
            pthread_exit(&buffer->end_result);
        }
        
        if(qBuffer->in_len == 0 && qBuffer->out_len == 0 ) {
            buffer->end_result = SQU_DONE;
            goto thread_out;
        }
        
        (qBuffer->scomp)(qBuffer); /* Function Call */
        
        if(qBuffer->end_result != SSTRM_DONE) {
            buffer->end_result = qBuffer->end_result;
            goto thread_out;
        }
        
        if(qBuffer->fd.out > 0) {            
            int ret = write_buf(qBuffer->fd.out, (char *)&qBuffer->out_len, 
                sizeof(qBuffer->out_len));
            if(ret < 0) {
                buffer->end_result = SPIPE_ERROR;
                goto thread_out;
            }
            
            ret = write_buf(qBuffer->fd.out, qBuffer->out_buffer, qBuffer->out_len);
            if (ret < 0) {
                buffer->end_result = SPIPE_ERROR;
                goto thread_out;
            }    
            
            if(qBuffer->out_buffer) {
                free(qBuffer->out_buffer);
            }
        } else {
            buffer->end_result = SPIPE_NOT_SET;
            strcpy(errorMsg, "Out pipe not set in stream thread.");
            goto thread_out;
        }
        
        buffer->end_result = qBuffer->end_result;
        if(qBuffer->in_buffer) {
            free(qBuffer->in_buffer);
        }
        if(qBuffer) {
            free(qBuffer);
        }
    }
    
    thread_out:
        /*Clean up*/
        if(qBuffer->in_buffer) free(qBuffer->in_buffer);
        if(qBuffer->out_buffer) free(qBuffer->out_buffer);
        if(qBuffer) free(qBuffer);
        
        pthread_exit(&buffer->end_result); 
}

void *stream_compress_pipe(void *_args) {
        
    int readed, ret = 0;
    pthread_t t_que;
    t_solid_data emptyBuffer;
    SOLID_DATA pip_buffer;
    
    SOLID_DATA buffer = (SOLID_DATA)_args;    
    
    if(_s_init(&pip_buffer) == NULL) {
        buffer->end_result = SSTRM_NULL_POINTER;
        pthread_exit(&buffer->end_result);
    }    

    pip_buffer->fd.in       = buffer->fd.in;
    pip_buffer->fd.out      = buffer->fd.out;
    emptyBuffer.in_len      = 0;
    emptyBuffer.out_len     = 0;
    buffer->id              = 0;
    
    if(buffer->fd.in != -1) {
        ret = pthread_create(&t_que, NULL, stream_compress_thread, pip_buffer);
        if (ret) {
            ret = -ret;
            fprintf(stderr, "ERROR: thread setup failed: %s\n",
                strerror(-ret));
            buffer->end_result = STH_ERROR;
            goto pipe_out;
        }
        
        if(!buffer->in_buffer) {
           buffer->in_buffer = (char *)malloc(PIPE_SEG);
        }
        
        while(1) {
            readed = fill_buffer(buffer, PIPE_SEG);
            
            if( readed < 0) {
                fprintf(stderr, "Error: failed to read stream");
                buffer->end_result = SPIPE_ERROR;
                pthread_exit(&buffer->end_result);                
            } else if(!buffer->in_len) {               
                pthread_mutex_lock(&lock);
                compQueue.push(&emptyBuffer);
                pthread_mutex_unlock(&lock);
                
                if((buffer->end_result = wait_for_finish(t_que)) != SQU_DONE) {
                    fprintf(stderr, "ERROR: Stream compress thread not done\n");
                    goto pipe_out;
                }
                
                buffer->end_result = SPIPE_DONE;
                pthread_exit(&buffer->end_result);                
            } else {
                buffer->id++;
                pthread_mutex_lock(&lock);
                compQueue.push(buffer);
                pthread_mutex_unlock(&lock);                
            }
        }
    }
    
    pipe_out:
        if(pip_buffer) free(pip_buffer);
        pthread_exit(&buffer->end_result);
}



void *diff_pipe(void *_args) {    
    pthread_t       t_pipe;
    SOLID_DATA      strm_buffer;
    SOLID_RESULT    retResult;
    
    int     pipefd[2]       = {-1, -1};
    int     temp_out_fd     = 0;
    SOLID_DATA buffer       = (SOLID_DATA)_args;
    
    
    if(_s_init(&strm_buffer) == NULL) {
        buffer->end_result = SDIFF_NULL_POINTER;
        pthread_exit(&buffer->end_result);
    }  
    
    int ret = pipe(pipefd);
    if (ret < 0) {
        ret = -errno;
        fprintf(stderr, "ERROR: pipe failed. %s\n", strerror(-ret));
        buffer->end_result = SPIPE_ERROR;
        goto diff_out;
    }
    
    strm_buffer->in_len = buffer->in_len;
    strm_buffer->fd.out = buffer->fd.out;
    temp_out_fd         = strm_buffer->fd.out;
    
    buffer->fd.out      = pipefd[1];
    strm_buffer->fd.in  = pipefd[0];
    
    ret = pthread_create(&t_pipe, NULL, stream_compress_pipe, strm_buffer);
    if (ret) {
        ret = -ret;
        fprintf(stderr, "ERROR: thread setup failed: %s\n",
            strerror(-ret));
        buffer->end_result = STH_ERROR;
        goto diff_out;
    }
    
    (buffer->dcomp)(buffer);
    
    if((retResult = wait_for_finish(t_pipe)) != SPIPE_DONE) {
        fprintf(stderr, "ERROR: Pipe thread not done\n");
        buffer->end_result = retResult;
        goto diff_out;
    }       
               
    diff_out:
        free(strm_buffer);
        if (pipefd[0] != -1)
            close(pipefd[0]);
        if(temp_out_fd)
            buffer->fd.out = temp_out_fd;
        pthread_exit(&buffer->end_result);
}

SOLID_RESULT solid_compress_fd(int in_fd, int dump_fd) {
    
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
    
    if(in_fd > 0 && dump_fd > 0) {
        de_dup_buffer->fd.in    = in_fd;
        diff_buffer->fd.out     = dump_fd;
    } else {
        fprintf(stderr,"I/O file descriptors not set");
        return SPIPE_NOT_SET;
    }    
    
    while(1) {
        de_dup_buffer->in_buffer = (char *) malloc(SEG_S);
        int readed = fill_buffer(de_dup_buffer, SEG_S);
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

SOLID_RESULT solid_compress(char* inbuffer, char *outbuffer, size_t in_len, size_t * out_len) {
    
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
}
