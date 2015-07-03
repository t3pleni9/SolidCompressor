/*
 * delta.c
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


#include "diff.h"
#include <math.h>


#define TENPER(x) (x/10)

typedef struct t_min_ds{
    int start;
    int end;
    int size;
    NODESP node_array;
} min_ds;

typedef min_ds* MPTR;

typedef struct t_looper {
    min_ds mXn;    
    int i_val;
    char *buffer;
    bit_feild *done;
} looper;

typedef looper* LPTR;



char *streamHandleBuffer = NULL;

__attribute__((destructor)) void unmapBuffer() {
    if(__diffBuffer__) munmap(__diffBuffer__, SEG_S + 20);
    if(streamHandleBuffer) munmap(streamHandleBuffer, SEG_S + 20);
}


static NODESP build_node_buffer(char *node_buffer, size_t *_offset_) {
    
    if(!node_buffer)
        return NULL;
    int offset = *_offset_;
    NODESP node = (NODESP) malloc(sizeof(_node_t));
    if(!node) 
        return NULL;
    
    if(memcpy((char *)&node->ref_node, (node_buffer + offset), sizeof(node->ref_node)) == NULL) {
        return NULL;
    }
    
    offset += sizeof(node->ref_node);
    if(memcpy((char *)&node->node_size, (node_buffer + offset), sizeof(node->node_size)) == NULL) {
        return NULL;
    }
    
    node->data = (char *)malloc(node->node_size);
    if(!node->data)
        return NULL;
        
    offset += sizeof(node->node_size);
    if(memcpy(node->data, (node_buffer + offset), node->node_size) == NULL) {
        return NULL;
    }
    
    offset += node->node_size;
    
    *_offset_ = offset;
    
    return node;

}

static int flatten_node_buffer(_node_t node, char **node_buffer) {
    *node_buffer = (char *)malloc(node.node_size + sizeof(int) + sizeof(size_t));
    
    if(!*node_buffer)
        return -1;
    int offset = 0;

    if(memcpy((*node_buffer + offset), (char *)&node.ref_node, sizeof(node.ref_node)) == NULL) {
        return -1;
    }
   
    offset += sizeof(node.ref_node);
    if(memcpy((*node_buffer + offset), (char *)&node.node_size, sizeof(node.node_size)) == NULL) {
        return -1;
    }
    
    offset += sizeof(node.node_size);
    if(memcpy((*node_buffer + offset), node.data, node.node_size) == NULL) {
        return -1;
    }
    
    offset += node.node_size;
    
    return offset;    
}

static diff_result _do_diff(const unsigned char *inBuffer, const unsigned char *baseBuffer, char **deltaBuffer,size_t inLen, size_t baseLen, size_t *outLen) {

    Bytef *delta = NULL;
    uLong d_size = 0;
    
 
    if(zd_compress1((const Bytef*) baseBuffer, (uLong)baseLen, (const Bytef*) inBuffer, 
    (uLong)inLen, &delta, (uLongf*)&d_size) == ZD_OK){
        *deltaBuffer = (char*)malloc(sizeof(char) * (size_t)d_size);
        memcpy((void*)(*deltaBuffer), (void*)delta, d_size);
        *outLen = (size_t)d_size;
    } else {
        return DIFF_ERROR;
    }            
    
    return DIFF_DONE;
}

static diff_result _do_patch(char *deltaBuffer, char *baseBuffer, char **patchBuffer, size_t deltaLen, size_t baseLen, size_t *patchLen) {
    
    Bytef *tar = NULL;
    uLong t_size = 0;
    
 
    if(zd_uncompress1((const Bytef*) baseBuffer, (uLong)baseLen, (Bytef**) &tar, 
    (uLongf*)&t_size, (const Bytef*)deltaBuffer, (uLong)deltaLen) == ZD_OK){
        *patchBuffer = (char*)malloc(sizeof(char) * (size_t)t_size);
        memcpy((void*)(*patchBuffer), (void*)tar, t_size);
        *patchLen = (size_t)t_size;
    } else {
        return PATCH_ERROR;
    }                
    
    return PATCH_DONE;
}

void * fd_inner_loop(void *args_) {
    LPTR lp = (LPTR)args_;
    int j;
    int i_val = lp->i_val;
    NODESP node_array = lp->mXn.node_array;
    char *inBuffer = lp->buffer;
    bit_feild *done = lp->done;
    
    if(i_val >= lp->mXn.start && i_val <= lp->mXn.end) {
        fprintf(stderr, "Self comparrison error. i_val in [start, end]\n");
        pthread_exit(NULL);
    }
    
    for(j = lp->mXn.start; j < lp->mXn.end ; j++) {
        if(!done[j].bit)
            continue;
        int sim = fuzzy_compare(node_array[i_val].fuzzy_hash_result, node_array[j].fuzzy_hash_result);                
        if( sim >= DIFF_THLD) {
            _do_diff(
                (const unsigned char *)(inBuffer + j*DIFF_BLOCK), 
                (const unsigned char *)(inBuffer + i_val*DIFF_BLOCK), 
                &node_array[j].data, 
                DIFF_BLOCK, DIFF_BLOCK, 
                &node_array[j].node_size
            );               
            
            node_array[j].ref_node = i_val;
            done[j].bit            = 0;  
        }
    }
    
    free(lp);
    pthread_exit(NULL);
    
}

static diff_result do_diff_fd(char *inBuffer, int out_fd, size_t inLen, size_t *out_len) {
        
        NODESP      node_array;
       	int         node_len;
        int         ret             = 0; 
        char        *node_buffer    = NULL;
        unsigned int blockCount     = inLen / DIFF_BLOCK, i, j;
        bit_feild   *done           = (bit_feild *)malloc(blockCount * sizeof(bit_feild));
        pthread_t   *thread         = NULL; 
        int         j_counter       = 0;
        void        *exit_status;
        
        if(out_len) {
            *out_len = 0;
        } else {
            perror("Out length not declared");
            return DIFF_NULL_POINTER;
        }
        
        node_array = (NODESP)malloc((
            (inLen > blockCount * DIFF_BLOCK)?
                blockCount + 1 
                : blockCount
        ) * sizeof(_node_t));
        
        if(out_len) {
            *out_len = 0;
        } else {
            perror("Out length not Initiallised");
            return DIFF_NULL_POINTER;
        }
        
        for(i = 0; i < blockCount ; i++) {
            done[i].bit = 1;
            node_array[i].fuzzy_hash_result   = (char *)malloc(FUZZY_MAX_RESULT);
            
            fuzzy_hash_buf((const unsigned char *)(inBuffer + i*DIFF_BLOCK), 
                                DIFF_BLOCK, node_array[i].fuzzy_hash_result);
        }
       
        for(i = 0; i < blockCount; i++) {   
            if(!done[i].bit) {
                if(!node_array[i].data) {
                    strcpy(errorMsg,"DIFF: Unable to set delta");
                    close(out_fd);
                    return DIFF_NOT_DONE;
                }
                
                if((node_len = flatten_node_buffer(node_array[i], &node_buffer)) != -1) {                
                    ret = write_buf(out_fd, node_buffer, node_len);
                    if (ret < 0) {
                        close(out_fd);
                        strcpy(errorMsg,"DIFF: Unable to write buffer. ");
                        strcat(errorMsg, strerror(-ret));
                        return DIFF_PIPE_ERROR;
                    }
                    
                    free(node_buffer);
                    free(node_array[i].data);
                    free(node_array[i].fuzzy_hash_result);
                }
                
                continue;
            }
            
            node_array[i].ref_node     = -1;
            node_array[i].node_size    = DIFF_BLOCK;
            node_array[i].data         = (char *)malloc(DIFF_BLOCK * sizeof(char));
            *out_len += (node_array[i].node_size + sizeof(int) + sizeof(size_t));
            memcpy(node_array[i].data, (inBuffer + i*DIFF_BLOCK), DIFF_BLOCK);
                           
            if((node_len = flatten_node_buffer(node_array[i], &node_buffer)) != -1) {    
                ret = write_buf(out_fd, node_buffer, node_len);
                if (ret < 0) {
                    close(out_fd);
                    strcpy(errorMsg,"DIFF: Unable to write buffer. ");
                    strcat(errorMsg, strerror(-ret));
                    return DIFF_PIPE_ERROR;
                }
                
                free(node_buffer);
                free(node_array[i].data);
                free(node_array[i].fuzzy_hash_result);
            }
            
            j = i+1;
            int tot_j_threads = ((blockCount - j) / 3000) + 1;
            if(thread)  free(thread);
            thread = (pthread_t*)malloc(sizeof(pthread_t)*tot_j_threads);
            j_counter = 0;
            while(j < blockCount) {                
                LPTR lp             = (LPTR)malloc(sizeof(looper));
                lp->mXn.node_array  = node_array;   
                lp->i_val           = i;
                lp->done            = done;
                lp->buffer          = inBuffer;
                lp->mXn.start       = j;
                lp->mXn.end         = (
                                        (j + 3000) > blockCount ? 
                                        (j = blockCount, j) : 
                                        (j += 3000,j)
                                    );               
                
                int ret = pthread_create(&thread[j_counter++], NULL, fd_inner_loop, (void *)lp);
                if(ret != 0) {
                    fprintf (stderr, "FATAL ERROR: Create pthread error: %s!\n", strerror(-ret));
                    exit (1);
                }                
            }
            
            j = 0;
            while(j < j_counter) pthread_join(thread[j++], &exit_status); 
        }
        
        if(inLen > blockCount * DIFF_BLOCK) {
            _node_t node;
            node.ref_node   = -1;
            node.node_size  = inLen - blockCount * DIFF_BLOCK;            
            node.data       = (char *)malloc(node.node_size * sizeof(char));
            
            memcpy(node.data, (inBuffer + i*DIFF_BLOCK) ,node.node_size);
            *out_len += (node.node_size + sizeof(int) + sizeof(size_t));
            blockCount++;
            if((node_len = flatten_node_buffer(node, &node_buffer)) != -1) {                
                ret = write_buf(out_fd, node_buffer, node_len);
                if (ret < 0) {
                    close(out_fd);
                    strcpy(errorMsg,"DIFF: Unable to write buffer. ");
                    strcat(errorMsg, strerror(-ret));
                    return DIFF_PIPE_ERROR;
                }
                              
                free(node_buffer);
                free(node.data);
            }
        }
        
        free(thread);
        free(done);
        free(node_array);
        return DIFF_DONE;        
}

void * stream_buff_handle(void *_args) {
    if ((streamHandleBuffer = (char *)mmap ((caddr_t)0, SEG_S + 20, PROT_READ | PROT_WRITE,
        MAP_SHARED|MAP_ANONYMOUS , -1, 0)) == (caddr_t) -1){
        fprintf (stderr, "Error: Unable to allocate memory pages, DEDUP.STRMBUFF\n");
        exit(1);
    }
    SOLID_DATA buffer = (SOLID_DATA)_args;
    buffer->in_buffer = streamHandleBuffer;
    buffer->in_len = 0;
    int readed = 0;
    while(1) {
        readed = fill_buffer(buffer, SEG_S);
        if( readed < 0) {
            fprintf(stderr, "Error: failed to read stream in streamBuffer %s\n",strerror(-readed));
            buffer->end_result = SPIPE_ERROR;
            break;
        } else if(!buffer->in_len) { 
            fprintf(stderr, "DIFF clossed the stream\n");
            buffer->end_result = SSTRM_DONE;
            break; 
        }
        fprintf(stderr, "Done reading %d %d\n", buffer->in_len, buffer->fd.out);
        readed = write_buf(buffer->fd.out, buffer->in_buffer, buffer->in_len);
        if (readed < 0) {
            strcpy(errorMsg,"Stream buffer handl: Unable to write buffer. ");
            strcat(errorMsg, strerror(-readed));
            buffer->end_result = SPIPE_ERROR;
            break;
        }
        fprintf(stderr, "Done writing %d\n", buffer->in_len);
    } 
    
    if(buffer->in_buffer)   buffer->in_buffer = NULL;
    if(streamHandleBuffer) munmap(streamHandleBuffer, SEG_S + 20);
    pthread_exit(&buffer->end_result);    
}


void * loop_thread(void *args_) {
    LPTR    lp          = (LPTR)args_;
    int     sim         = 0;
    int     i_val       = lp->i_val;
    int     i           = 0;
    NODESP  node_array  = lp->mXn.node_array;
    int     j;
    int     *sim_count  = (int *)malloc (sizeof(int));
    int     j_check     = 0;
    int     increment   = 0;
    int     cur_sim_cnt = 0;
    
    clock_t begin, end;
    double time_spent = 0;
        
    begin = clock();
    *sim_count = 0;
    /*if(i_val >= lp->mXn.start && i_val <= lp->mXn.end) {
        fprintf(stderr, "Self comparrison error. i_val in [start, end]\n");
        pthread_exit(NULL);
     }*/
    int diff_degree = (int)pow10((level / 3) + 2);
    for(i = lp->mXn.start; i < lp->mXn.end; i++) {
         j_check = i + TENPER(i_val);
         increment = j_check;
        for(j = i+1; j < i_val; j++) {
            sim = fuzzy_compare(node_array[i].fuzzy_hash_result, node_array[j].fuzzy_hash_result);
            if(sim >= DIFF_THLD) {
                (*sim_count)++;
                cur_sim_cnt++;
                pthread_mutex_lock(&node_array[j].mutex);
                
                if(node_array[j].sim < sim ) {
                    node_array[j].sim = sim;
                    node_array[j].ref_node = i;
                }
                
                pthread_mutex_unlock(&node_array[j].mutex);
            }
            
            
            if(j == j_check) {
                if(cur_sim_cnt < (i_val / diff_degree)) {
                    j += increment;   
                    increment += TENPER(i_val);
                    cur_sim_cnt = 0;                
                } else {
                    increment /= 2;
                }
                
                j_check = (j + TENPER(i_val));
            }
        }
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    fprintf(stderr, "finished: %d %f\n", lp->mXn.end, time_spent);
    if(lp) free(lp);
    pthread_exit(sim_count);
}

static int create_delta_graph(NODESP node_array, int block) {
    int         i = 0, j;
    
    pthread_t   *thread      = NULL;
    int         j_counter    = 0;
    void        *exit_status = NULL;
    int         percDiv[8]   = { 13,17,20,55 };
    int         goodness     = 0;
    //for(i = 0; i < block; i++) { 
        
        j_counter           = 0;
        int tot_j_threads   = 7;
                
        //if(thread)  free(thread);            
        thread  = (pthread_t*)malloc(sizeof(pthread_t)*tot_j_threads);
        
        while(i < block) {                
            LPTR lp             = NULL;
            lp                  = (LPTR)malloc(sizeof(looper));
            lp->mXn.node_array  = node_array;  
            lp->i_val           = block; 
            lp->mXn.start       = i;            
            lp->mXn.end         = (
                                    (i + (block * percDiv[j_counter] / 100) ) >= block ? 
                                    (i = block, i) : 
                                    (i += (block * percDiv[j_counter] / 100),i)
                                );      
            
            int ret = pthread_create(&thread[j_counter++], NULL, loop_thread, (void *)lp);
            fprintf(stderr, "%d %d %d\n", j_counter,i,block);
            if(ret != 0) {
                fprintf (stderr, "FATAL ERROR: Create pthread error: %s!\n", strerror(-ret));
                exit (1);
            }             
        }
        
        j = 0;
        while(j < j_counter) {
            pthread_join(thread[j++], &exit_status);    
            if(!exit_status) {
                fprintf (stderr, "FATAL ERROR: Inner thread error!\n");
                exit (1);
            } else {
                goodness += *((int *)exit_status);
                free(exit_status);
            }
        }
    //}
    
    if(thread) free(thread);
    
    return goodness;
}

static diff_result do_diff_fd_mst(char *inBuffer, int out_fd, size_t inLen, size_t *out_len, pthread_t* t_diff) {
        time_t t;        
        srand((unsigned) time(&t));
        static int first_run = 1;
        int pipefd[2]   = {-1, -1};
        static t_solid_data strm_buffer;
        NODESP      node_array;
       	int         node_len;
        int         ret             = 0; 
        char        *node_buffer    = NULL;
        unsigned int blockCount     = inLen / DIFF_BLOCK, i;
        ret = pipe(pipefd);    
        if (ret < 0) {
            ret = -errno;
            fprintf(stderr, "ERROR: pipe failed. %s\n", strerror(-ret));
            return DIFF_PIPE_ERROR;
        }
       
        if(out_len) {
            *out_len = 0;
        } else {
            perror("Out length not declared");
            return DIFF_NULL_POINTER;
        }
        
        node_array = (NODESP)malloc((
            (inLen > blockCount * DIFF_BLOCK)?
                blockCount + 1 
                : blockCount
        ) * sizeof(_node_t));        
        
        for(i = 0; i < blockCount ; i++) {
            node_array[i].ref_node = -1;
            node_array[i].sim      = 0;
                      
            node_array[i].fuzzy_hash_result   = (char *)malloc(FUZZY_MAX_RESULT);
            
             if (pthread_mutex_init(&node_array[i].mutex, NULL) != 0) {
                fprintf(stderr, "Mutex init failed\n");
                return DIFF_THRD_ERROR;
            }
            
            fuzzy_hash_buf((const unsigned char *)(inBuffer + i*DIFF_BLOCK), DIFF_BLOCK, node_array[i].fuzzy_hash_result);
        }
        
        clock_t begin, end, begin1, end1;
        double time_spent = 0, tim2 = 0;
        
        begin = clock();
        int goodness = create_delta_graph(node_array, blockCount);
        end = clock();
        fprintf(stderr, "goodness = %d\n", goodness);
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        fprintf(stderr, "Create Graph time: %f\n", time_spent);
        //time_spent = 0;
        if(!first_run) {
            if(wait_for_finish(*t_diff) != SSTRM_DONE) {
                fprintf(stderr, "ERROR: Diff thread not done\n");
                return DIFF_PIPE_ERROR;
            }
        }
        
        strm_buffer.in_buffer = NULL;
        strm_buffer.out_buffer = NULL;
        strm_buffer.in_len = 0;
        strm_buffer.out_len = 0;
        strm_buffer.fd.in = pipefd[0];
        strm_buffer.fd.out = out_fd;
        
        ret = pthread_create(t_diff, NULL, stream_buff_handle, (void *)&strm_buffer);                
        if (ret) {
            ret = -ret;
            fprintf(stderr, "ERROR: thread setup failed: %s\n",
                strerror(-ret));
            if (pipefd[1] != -1)
                close(pipefd[1]);
            return DIFF_PIPE_ERROR;
        }
    
        for(i = 0; i < blockCount; i++) { 
            begin1 = clock();    
            if(node_array[i].ref_node != -1) { 
                           
                _do_diff(
                    (const unsigned char *)(inBuffer + i*DIFF_BLOCK), 
                    (const unsigned char *)(inBuffer + node_array[i].ref_node*DIFF_BLOCK), 
                    &node_array[i].data, 
                    DIFF_BLOCK, DIFF_BLOCK, 
                    &node_array[i].node_size
                );
                
            } else {
                node_array[i].node_size    = DIFF_BLOCK;
                node_array[i].data         = (inBuffer + i*DIFF_BLOCK);
                //memcpy(node_array[i].data, (inBuffer + i*DIFF_BLOCK), DIFF_BLOCK);
            }
            
            *out_len += (node_array[i].node_size + sizeof(int) + sizeof(size_t));
                
            if(!node_array[i].data) {
                strcpy(errorMsg,"DIFF: Unable to set delta");
                close(pipefd[1]);
                return DIFF_NOT_DONE;
            }
            
            if((node_len = flatten_node_buffer(node_array[i], &node_buffer)) != -1) {
                end1 = clock();
                time_spent += (double)(end1 - begin1) / CLOCKS_PER_SEC;
                
                begin = clock();                
                ret = write_buf(pipefd[1], node_buffer, node_len);
                end = clock();
                tim2 += (double)(end - begin) / CLOCKS_PER_SEC;
                if (ret < 0) {
                    close(pipefd[1]);
                    strcpy(errorMsg,"DIFF: Unable to write buffer. ");
                    strcat(errorMsg, strerror(-ret));
                    return DIFF_PIPE_ERROR;
                }
                
                free(node_buffer);
                if(node_array[i].ref_node != -1) free(node_array[i].data);
                free(node_array[i].fuzzy_hash_result);
                pthread_mutex_destroy(&node_array[i].mutex);
            }
        }
        
        first_run = 0;
        free(node_array);
        begin1 = clock(); 
        if(inLen > blockCount * DIFF_BLOCK) {
            _node_t node;
            node.ref_node   = -1;
            node.node_size  = inLen - blockCount * DIFF_BLOCK;            
            node.data       = (char *)malloc(node.node_size * sizeof(char));
            
            memcpy(node.data, (inBuffer + i*DIFF_BLOCK) ,node.node_size);
            *out_len += (node.node_size + sizeof(int) + sizeof(size_t));
            blockCount++;
            if((node_len = flatten_node_buffer(node, &node_buffer)) != -1) {                
                ret = write_buf(pipefd[1], node_buffer, node_len);
                if (ret < 0) {
                    close(out_fd);
                    strcpy(errorMsg,"DIFF: Unable to write buffer. ");
                    strcat(errorMsg, strerror(-ret));
                    return DIFF_PIPE_ERROR;
                }
                              
                free(node_buffer);
                free(node.data);
            }
        }
        end1 = clock();
        time_spent += (double)(end1 - begin1) / CLOCKS_PER_SEC;
        close(pipefd[1]);
        fprintf(stderr, "Str time: %f diff time:%f\n", tim2, time_spent);
        return DIFF_DONE;        
}


static int recreate_delta(NODEDP node_array, SOLID_DATA buffer, int node_count) {
    NODESP node = node_array[node_count - 1];
    if(node->ref_node == -1){
        return node->node_size == DIFF_BLOCK ? -1 : 0 ;
    } else {
        if(node->ref_node >= node_count)    return -2;
        
        NODESP temp_node = (NODESP)malloc(sizeof(_node_t));
         _do_patch(
            node->data, 
            node_array[node->ref_node]->data, 
            &temp_node->data, 
            node->node_size, 
            DIFF_BLOCK, 
            &temp_node->node_size
        );
        
        node_array[node_count - 1] = temp_node;
        free(node);
        return 1;
    }
    
    return -2; // Non reaching case
}

static diff_result do_patch_fd(SOLID_DATA buffer) {
    
    pthread_t 	t_dup;
    
    size_t	temp_out_len 	= 0;
    int		node_count 		= 0;
    int		ret 			= 0;
    int		first_run 		= 1;
    NODEDP	node_array 		= (NODEDP)malloc(MAX_DIFF_BLOCK*sizeof(NODESP));
    
    buffer->in_buffer 		= (char *)malloc(PATCH_BLOCK);
    buffer->in_len 			= 0;
    buffer->out_len 		= 0;
   
    while(1) {
        int	readed = refill_buffer(buffer, PATCH_BLOCK);
        
       	if(readed == 0) {
            if(!first_run) {                        
                if((buffer->end_result = wait_for_finish(t_dup)) != SDUP_DONE) {
                    fprintf(stderr, "ERROR: Duplicator thread not done\n");
                    buffer->end_result = STH_ERROR;                    
                    return PATCH_ERROR; 
                }                        
            }
            
            node_array[node_count++] 	= build_node_buffer(buffer->in_buffer, &temp_out_len);        
        	ret 						= recreate_delta(node_array, buffer, node_count);
            
            if(ret == -1) {
                buffer->in_len 	= 0;
                temp_out_len 	= 0;
            } else if(ret == -2) {
                fprintf(stderr, "Undefined ref node\n");
                buffer->end_result = SPATCH_ERROR;
                return PATCH_ERROR;
            }
            
            buffer->out_len		=	(
            	node_count > 1 ? 
            		((node_count-1)* DIFF_BLOCK + node_array[node_count-1]->node_size) 
                	: node_array[node_count-1]->node_size
        	);
            
            if(buffer->out_buffer)	buffer->out_buffer = NULL;//free(buffer->out_buffer);
            
            buffer->out_buffer 	= __diffBuffer__;//(char *) malloc(buffer->out_len);
            int i 				= 0; 
            int offset 			= 0;
            
            while(i < node_count) {
                if(node_array[i]->data) {
                    memcpy((buffer->out_buffer + offset), node_array[i]->data, node_array[i]->node_size);
                    offset += node_array[i]->node_size;
                    free(node_array[i]->data);
                    node_array[i]->data = NULL;
                    free(node_array[i]);
                }
                
                i++;
            }
            
           	ret = pthread_create(&t_dup, NULL, buffer->dupcomp, buffer);                
            if (ret) {
                ret = -ret;
                fprintf(stderr, "ERROR: thread setup failed: %s\n",
                    strerror(-ret));
                buffer->end_result = STH_ERROR;
                //clean up 
                exit(-1); // remove to return clauses.
            }
            
            if(!first_run) {                        
                if((buffer->end_result = wait_for_finish(t_dup)) != SDUP_DONE) {
                    fprintf(stderr, "ERROR: Duplicator thread not done\n");
                    buffer->end_result = STH_ERROR;
                    //clean up 
                    exit(-1); // remove to return clauses.
                }                        
            }
            
            break;
        } else if(readed < 0) {
            fprintf(stderr, "ERROR: Refil buffer returned < 1 %s\n", strerror(-readed));
            buffer->end_result = SPIPE_ERROR;
            exit(-1);
        }
        
        node_array[node_count++] = build_node_buffer(buffer->in_buffer, &temp_out_len);
        
        ret = recreate_delta(node_array, buffer, node_count);
        if(ret == -1) {
            buffer->in_len 	= 0;
            temp_out_len 	= 0;
        } else if(ret == -2) {
            fprintf(stderr, "Undefined ref node\n");
            buffer->end_result = SPATCH_ERROR;
            return PATCH_ERROR;
        } else {
            if(node_count <= 0) {
                fprintf(stderr, "Undefined ref node. No nodes in the buffer\n");
                buffer->end_result = SPATCH_ERROR;
                return PATCH_ERROR;
            }
            
            if(node_array[node_count-1]->node_size != DIFF_BLOCK && node_array[node_count-1]->ref_node == -1) {                
                if(!first_run) {                        
                    if((buffer->end_result = wait_for_finish(t_dup)) != SDUP_DONE) {
                        fprintf(stderr, "ERROR: Duplicator thread not done\n");
                        buffer->end_result = STH_ERROR;
                        exit(-1); 
                    }                        
                }
                
                buffer->out_len = (
                	node_count > 1 ? 
                		((node_count-1)* DIFF_BLOCK + node_array[node_count-1]->node_size) 
                    	: node_array[node_count-1]->node_size
            	);
                
                if(buffer->out_buffer)	buffer->out_buffer = NULL;
                
                buffer->out_buffer	= 	__diffBuffer__;//(char *) malloc(buffer->out_len);
                int i 				= 	0; 
                int offset 			= 	0;
                
                while(i < node_count) {
                    if(node_array[i]->data) {
                        memcpy((buffer->out_buffer + offset), node_array[i]->data, node_array[i]->node_size);
                        offset += node_array[i]->node_size;
                        free(node_array[i]->data);
                        node_array[i]->data = NULL;
                        free(node_array[i]);
                    }
                    
                    i++;
                }
               	
               	ret = pthread_create(&t_dup, NULL, buffer->dupcomp, buffer);                
                if (ret) {
                    ret = -ret;
                    fprintf(stderr, "ERROR: thread setup failed: %s\n",
                        strerror(-ret));
                    buffer->end_result = STH_ERROR;
                    exit(-1);
                }
                
                first_run 	= 0;                
                node_count 	= 1;
            }
            
            char *temp = (char *)malloc(PATCH_BLOCK - temp_out_len);
            memcpy(temp, (buffer->in_buffer + temp_out_len), PATCH_BLOCK - temp_out_len);
            memset(buffer->in_buffer, 0, PATCH_BLOCK);
            memcpy(buffer->in_buffer, temp, PATCH_BLOCK - temp_out_len);
            buffer->in_len = PATCH_BLOCK - temp_out_len;
            
            free(temp);
            temp_out_len = 0;
            
            if(!ret)	node_count = 0;
            
        }
    }
    
    buffer->end_result = SPATCH_DONE;
    free(node_array);
    return PATCH_DONE;
}


SOLID_RESULT zdelta_patch(void* _args) {
    SOLID_DATA buffer = (SOLID_DATA)_args;
    do_patch_fd(buffer);
    return buffer->end_result;
}

SOLID_RESULT zdelta_diff(void* _args) {    
    diff_result diff;
    SOLID_DATA buffer = (SOLID_DATA)_args;
    diff = do_diff_fd(buffer->in_buffer, buffer->fd.out, buffer->in_len, &buffer->out_len);
        
    switch(diff) {
        case DIFF_DONE:         buffer->end_result = SDIFF_DONE;
        break;
        case DIFF_NULL_POINTER: buffer->end_result = SDIFF_NULL_POINTER;
        break;
        case DIFF_PIPE_ERROR:   buffer->end_result = SPIPE_ERROR;
        break;
        default: fprintf(stderr, "ERROR: Diff error, %d %s\n", (int)diff, errorMsg);
                                buffer->end_result = SDIFF_ERROR;
    }
    
    if(buffer->in_buffer)
        free(buffer->in_buffer);
    return buffer->end_result;
}

SOLID_RESULT zmst_diff(void* _args) {  
    diff_result diff;
    static int i = 0;
    clock_t begin, end;
    double time_spent;
    SOLID_DATA buffer = (SOLID_DATA)_args;
    
    SOLID_RESULT retResult;
    pthread_t t_diff;
        
    while(1){
        int ret = read( buffer->fd.in, (char *)&(buffer->in_len), sizeof(buffer->in_len));
        if(buffer->in_buffer)   /*free(*/buffer->in_buffer/*);/*/ = NULL;
        
        buffer->in_buffer = __diffBuffer__;//(char *)malloc(buffer->in_len * sizeof(char));//
        ret = fill_buffer(buffer, buffer->in_len);
        if( ret < 0) {
            fprintf(stderr, "Error: failed to read stream in zmst_diff\n");
            buffer->end_result = SPIPE_ERROR;
            goto out;
        } else if(!buffer->in_len) {  
            /*if(!first_run) {
                    
            }*/
                         
            buffer->end_result   = SDIFF_DONE;
            goto out;
        }
        
        begin = clock();
        diff = do_diff_fd_mst(buffer->in_buffer, buffer->fd.out, buffer->in_len, &buffer->out_len, &t_diff);
        
        end = clock();
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        fprintf(stderr,"Delta:%d %f\n", i++, time_spent);
        switch(diff) {
            case DIFF_DONE:         buffer->end_result = SDIFF_DONE;
            break;
            case DIFF_NULL_POINTER: buffer->end_result = SDIFF_NULL_POINTER;
            break;
            case DIFF_PIPE_ERROR:   buffer->end_result = SPIPE_ERROR;
            break;
            default: fprintf(stderr, "ERROR: Diff error, %d %s\n", (int)diff, errorMsg);
                                    buffer->end_result = SDIFF_ERROR;
        }
        
        if(buffer->end_result != SDIFF_DONE)
            break;
    }
out:
    
    if((retResult = wait_for_finish(t_diff)) != SSTRM_DONE) {
        fprintf(stderr, "ERROR: Diff thread not done\n");
        buffer->end_result = retResult;
    }
    if(buffer->in_buffer) buffer->in_buffer = NULL;
    return buffer->end_result;
}
