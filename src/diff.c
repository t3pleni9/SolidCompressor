/*
 * delta.h
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




static NODESP build_node_buffer(char *node_buffer, int *_offset_) {
    
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
    printf("here %d\n", node->ref_node);
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

static int create_delta_buffer(NODEDP nodes, size_t block_count, char *out_buffer) {
    int i, node_len, offset = 0;
    char *node_buffer = NULL;
    for(i = 0; i< block_count; i++) {
        if((node_len = flatten_node_buffer(*(nodes[i]), &node_buffer)) != -1) {                
            memcpy((out_buffer + offset), node_buffer, node_len);
            offset += node_len;              
            free(node_buffer);
        } else {
            return 0;
        }
    }
    
    return 1;
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
/* 
 * TODO: Do something about the last block 
 */

static diff_result do_diff(char *inBuffer, char **outBuffer, size_t inLen, size_t *out_len) {
        time_t t;
        srand((unsigned) time(&t));
        unsigned int blockCount = inLen / DIFF_BLOCK, i, j;
        NODEDP node_array;

        size_t deltaLen = 0;
        if(out_len) {
            *out_len = 0;
        } else {
            perror("Out length not declared");
            return DIFF_NULL_POINTER;
        }
        
        char *done = (char *)malloc(blockCount * sizeof(char));
        node_array = (NODEDP)malloc(((inLen > blockCount * DIFF_BLOCK)?blockCount + 1 : blockCount) * sizeof(NODESP));
        for(i = 0; i < blockCount ; i++) {
            done[i] = 1;
            node_array[i] = (NODESP)malloc(sizeof(_node_t));
            node_array[i]->fuzzy_hash_result = (char *)malloc(FUZZY_MAX_RESULT);
            fuzzy_hash_buf((const unsigned char *)(inBuffer + i*DIFF_BLOCK), DIFF_BLOCK, node_array[i]->fuzzy_hash_result);
        }
       
        unsigned int blockCounter = 0;
        
        for(i = 0; i < blockCount; i++) {
            
            if(!done[i])
                continue;
            node_array[i]->ref_node = -1;
            node_array[i]->node_size = DIFF_BLOCK;
            node_array[i]->data = (char *)malloc(DIFF_BLOCK * sizeof(char));
            *out_len += (node_array[i]->node_size + sizeof(int) + sizeof(size_t));
            memcpy(node_array[i]->data, (inBuffer + i*DIFF_BLOCK), DIFF_BLOCK);
            blockCounter+= DIFF_BLOCK;
            //int inc = (rand() % 9) + 2;
            for(j = i+1; j < blockCount ; j++) {
                if(!done[j])
                    continue;
                int sim = 0;
                sim = fuzzy_compare(node_array[i]->fuzzy_hash_result,node_array[j]->fuzzy_hash_result);
                if( sim >= DIFF_THLD) {
                    node_array[j] = (NODESP)malloc(sizeof(_node_t));
                    _do_diff((const unsigned char *)(inBuffer + j*DIFF_BLOCK), (const unsigned char *)(inBuffer + i*DIFF_BLOCK), &node_array[j]->data, DIFF_BLOCK, DIFF_BLOCK, &node_array[j]->node_size);               
                    
                    node_array[j]->ref_node = i;
                    *out_len += (node_array[j]->node_size + sizeof(int) + sizeof(size_t));
                    
                    if(!node_array[j]->data) {
                        strcpy(errorMsg,"delta not set");
                        return DIFF_NOT_DONE;
                    }
                    
                    blockCounter += deltaLen;                        
                    done[j] = 0;
                    
                } else {
                    
                }
            }
            
            free(node_array[i]->fuzzy_hash_result);
        }
        
        if(inLen > blockCount * DIFF_BLOCK) {
            node_array[i] = (NODESP)malloc(sizeof(_node_t));
            node_array[i]->ref_node = -1;
            node_array[i]->node_size = DIFF_BLOCK;
            node_array[i]->data = (char *)malloc(DIFF_BLOCK * sizeof(char));
            *out_len += (node_array[i]->node_size + sizeof(int) + sizeof(size_t));
            memcpy(node_array[i]->data, (inBuffer + i*DIFF_BLOCK), inLen - blockCount * DIFF_BLOCK);
            blockCount++;
        }
        
        *outBuffer = (char *)malloc(*out_len * sizeof(char));
        create_delta_buffer(node_array, blockCount, *outBuffer); 

        for(i = 0; i< blockCount; i++) {
            free(node_array[i]->data);
            free(node_array[i]);
        }
        
        free(node_array);
        return DIFF_DONE;        
}

//TODO: Free every malloc buffers on error conditions.
static diff_result do_diff_fd(char *inBuffer, int out_fd, size_t inLen, size_t *out_len) {
        time_t t;
        srand((unsigned) time(&t));
        char **fuzzy_hash_result;
        unsigned int blockCount = inLen / DIFF_BLOCK, i, j;
        NODEDP node_array;

        size_t deltaLen = 0;
        if(out_len) {
            *out_len = 0;
        } else {
            perror("Out length not declared");
            return DIFF_NULL_POINTER;
        }
        
        node_array = (NODEDP)malloc(((inLen > blockCount * DIFF_BLOCK)?blockCount + 1 : blockCount) * sizeof(NODESP));
        int ret = 0, node_len;
        char *node_buffer = NULL;
        
        if(out_len) {
            *out_len = 0;
        } else {
            perror("Out length not declared");
            return DIFF_NULL_POINTER;
        }
        
        bit_feild **done = (bit_feild **)malloc(blockCount * sizeof(bit_feild *));
        fuzzy_hash_result = (char **)malloc(((inLen > blockCount * DIFF_BLOCK)?
            blockCount + 1 : blockCount) * sizeof(char *));
        
        for(i = 0; i < blockCount ; i++) {
            done[i] = (bit_feild *)malloc(sizeof(bit_feild));
            done[i]->bit = 1;
            node_array[i] = (NODESP)malloc(sizeof(_node_t));
            fuzzy_hash_result[i] = (char *)malloc(FUZZY_MAX_RESULT);
            fuzzy_hash_buf((const unsigned char *)(inBuffer + i*DIFF_BLOCK), DIFF_BLOCK, fuzzy_hash_result[i]);
        }
       
        for(i = 0; i < blockCount; i++) {     
            
            if(!done[i]->bit) {
                if(!node_array[i]->data) {
                    strcpy(errorMsg,"DIFF: Unable to set delta");
                    close(out_fd);
                    return DIFF_NOT_DONE;
                }
                if((node_len = flatten_node_buffer(*(node_array[i]), &node_buffer)) != -1) {                
                    ret = write_buf(out_fd, node_buffer, node_len);
                    if (ret < 0) {
                        close(out_fd);
                        strcpy(errorMsg,"DIFF: Unable to write buffer. ");
                        strcat(errorMsg, strerror(-ret));
                        return DIFF_PIPE_ERROR;
                    }
                    
                    free(node_buffer);
                    free(node_array[i]->data);
                    free(node_array[i]);
                }
                continue;
            }
            
            node_array[i]->ref_node = -1;
            node_array[i]->node_size = DIFF_BLOCK;
            node_array[i]->data = (char *)malloc(DIFF_BLOCK * sizeof(char));
            *out_len += (node_array[i]->node_size + sizeof(int) + sizeof(size_t));
            memcpy(node_array[i]->data, (inBuffer + i*DIFF_BLOCK), DIFF_BLOCK);
            if((node_len = flatten_node_buffer(*(node_array[i]), &node_buffer)) != -1) {                
                ret = write_buf(out_fd, node_buffer, node_len);
                if (ret < 0) {
                    close(out_fd);
                    strcpy(errorMsg,"DIFF: Unable to write buffer. ");
                    strcat(errorMsg, strerror(-ret));
                    return DIFF_PIPE_ERROR;
                }
                
                free(node_buffer);
                free(node_array[i]->data);
                free(node_array[i]);
            }
            //int inc = (rand() % 9) + 2;
            for(j = i+1; j < blockCount ; j++) {
                if(!done[j]->bit)
                    continue;
                int sim = 0;
                sim = fuzzy_compare(fuzzy_hash_result[i],fuzzy_hash_result[j]);
                if( sim >= DIFF_THLD) {
                    _do_diff((const unsigned char *)(inBuffer + j*DIFF_BLOCK), (const unsigned char *)(inBuffer + i*DIFF_BLOCK), &node_array[j]->data, DIFF_BLOCK, DIFF_BLOCK, &node_array[j]->node_size);               
                    NODESP temp_node = (NODESP)malloc(sizeof(_node_t));
                    _do_patch((node_array[j]->data), (inBuffer + i*DIFF_BLOCK), 
                    &temp_node->data, node_array[j]->node_size, DIFF_BLOCK, &temp_node->node_size);
        
                    node_array[j]->ref_node = i;
                   // *out_len += (node_array[i]->node_size + sizeof(int) + sizeof(size_t));
                    
                    
                    
                    done[j]->bit = 0;     
                    
                    /* Debugger part*/
                     printf("i = %d j = %d %d %d\n", i, j, blockCount, node_array[j]->node_size);               
                     
                } else {
                    
                }
            }
            
            free(fuzzy_hash_result[i]);
        }
        
        if(inLen > blockCount * DIFF_BLOCK) {
            _node_t node;
            node.ref_node = -1;
            node.node_size = inLen - blockCount * DIFF_BLOCK;
            node.data = (char *)malloc(node.node_size * sizeof(char));
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
        
        free(fuzzy_hash_result);
        return DIFF_DONE;        
}

static int recreate_delta(NODEDP node_array, SOLID_DATA buffer, int node_count) {
    static int i = 0;
    //printf("i = %d\n", i);
    NODESP node = node_array[node_count - 1];
    if(node->ref_node == -1){
        return node->node_size == DIFF_BLOCK ? -1 : 0 ;
    } else {
        if(node->ref_node >= node_count)    return -2;
        
        NODESP temp_node = (NODESP)malloc(sizeof(_node_t));
        printf("d_len = %d %d\n",node_array[node_count - 1]->node_size,node->ref_node);
        
        _do_patch(node->data, node_array[node->ref_node]->data, 
        &temp_node->data, node->node_size, DIFF_BLOCK, &temp_node->node_size);
        node_array[node_count - 1] = temp_node;
        free(node);
       
        i++;
        printf("d_len = %d\n",  node_array[node_count - 1]->node_size);
        return 1;
    }
}

static diff_result do_patch_fd(SOLID_DATA buffer) {
    
    pthread_t t_dup;
    buffer->in_buffer = (char *)malloc(PATCH_BLOCK);
    size_t temp_out_len = 0;
    buffer->in_len = 0;
    buffer->out_len = 0;
    int node_count = 0;
    int ret = 0;
    NODEDP node_array = (NODEDP)malloc(MAX_DIFF_BLOCK*sizeof(NODESP));
    int first_run = 1;
    SOLID_RESULT retResult;
    while(1) {
        //buffer.out_len = 0;
        printf("node count: %d\n", node_count);
        int readed = refill_buffer(buffer, PATCH_BLOCK);
        printf("ret %d %d %d\n", readed, buffer->in_len, temp_out_len);
        if(readed == 0) {
            if(!first_run) {                        
                if((retResult = wait_for_finish(t_dup)) != SDUP_DONE) {
                    fprintf(stderr, "ERROR: Duplicator thread not done\n");
                    //retResult = STH_ERROR;
                    //clean up 
                    exit(-1); // remove to return clauses.
                }                        
            }
            
            buffer->out_len = (node_count > 1 ? ((node_count-2)* DIFF_BLOCK + node_array[node_count-1]->node_size) 
                : node_array[node_count-1]->node_size);
            printf("final node len : %d\n", buffer->out_len);
            if(buffer->out_buffer) free(buffer->out_buffer);
            buffer->out_buffer = (char *) malloc(buffer->out_len);
            int i = 0; 
            int offset = 0;
            while(i < node_count) {
                if(node_array[i]->data) {
                    memcpy((buffer->out_buffer + offset), node_array[i]->data, node_array[i]->node_size);
                    offset += node_array[i]->node_size;
                    free(node_array[i]);
                }
                
                i++;
            }
            
            
            
            ret = pthread_create(&t_dup, NULL, buffer->dupcomp, buffer);                
            if (ret) {
                ret = -ret;
                fprintf(stderr, "ERROR: thread setup failed: %s\n",
                    strerror(-ret));
                //retResult = STH_ERROR;
                //clean up 
                exit(-1); // remove to return clauses.
            }
            
            if(!first_run) {                        
                if((retResult = wait_for_finish(t_dup)) != SDUP_DONE) {
                    fprintf(stderr, "ERROR: Duplicator thread not done\n");
                    //retResult = STH_ERROR;
                    //clean up 
                    exit(-1); // remove to return clauses.
                }                        
            }
            close(buffer->fd.out);
            break;
        } else if(readed < 0) {
            //TODO: clean up
            exit(-1); // remove to return clauses.
        }
        
        node_array[node_count++] = build_node_buffer(buffer->in_buffer, &temp_out_len);
        
        printf("%d %d %d %d\n", node_array[node_count-1]->node_size, node_array[node_count-1]->ref_node, buffer->in_len,  temp_out_len );
        
        ret = recreate_delta(node_array, buffer, node_count);
        printf("%d %d %d %d\n", node_array[node_count-1]->node_size, node_array[node_count-1]->ref_node, buffer->in_len,  temp_out_len );
        if(ret == -1) {
            
            buffer->in_len = 0;
            temp_out_len = 0;
        } else if(ret == -2) {
            fprintf(stderr, "Undefined ref node\n");
            //TODO: clean up
            exit(-1); // remove to return clauses.
        } else {
            if(node_count <= 0) {
                fprintf(stderr, "Undefined ref node\n");
                //TODO: clean up
                exit(-1); // remove to return clauses.
            }
            
            if(node_array[node_count-1]->node_size != DIFF_BLOCK && node_array[node_count-1]->ref_node == -1) {
                
                if(!first_run) {                        
                    if((retResult = wait_for_finish(t_dup)) != SDUP_DONE) {
                        fprintf(stderr, "ERROR: Duplicator thread not done\n");
                        //retResult = STH_ERROR;
                        //clean up 
                        exit(-1); // remove to return clauses.
                    }                        
                }
                
                buffer->out_len = (node_count > 1 ? ((node_count-2)* DIFF_BLOCK + node_array[node_count-1]->node_size) 
                    : node_array[node_count-1]->node_size);
                if(buffer->out_buffer) free(buffer->out_buffer);
                buffer->out_buffer = (char *) malloc(buffer->out_len);
                int i = 0; 
                int offset = 0;
                while(i < node_count) {
                    if(node_array[i]->data) {
                        memcpy((buffer->out_buffer + offset), node_array[i]->data, node_array[i]->node_size);
                        offset += node_array[i]->node_size;
                        free(node_array[i]);
                    }
                    
                    i++;
                }
                
                
                
                ret = pthread_create(&t_dup, NULL, buffer->dupcomp, buffer);                
                if (ret) {
                    ret = -ret;
                    fprintf(stderr, "ERROR: thread setup failed: %s\n",
                        strerror(-ret));
                    //retResult = STH_ERROR;
                    //clean up 
                    exit(-1); // remove to return clauses.
                }
                
                first_run = 0;
                
                node_count = 1;
            }
            memcpy((char *)&node_array[node_count-1]->node_size, 
                (buffer->in_buffer + temp_out_len + sizeof(node_array[node_count-1]->node_size)), 
                sizeof(node_array[node_count-1]->node_size));
            char *temp = (char *)malloc(PATCH_BLOCK - temp_out_len);
            memcpy(temp, (buffer->in_buffer + temp_out_len), PATCH_BLOCK - temp_out_len);
            memset(buffer->in_buffer, 0, PATCH_BLOCK);
            memcpy(buffer->in_buffer, temp, PATCH_BLOCK - temp_out_len);
            buffer->in_len = PATCH_BLOCK - temp_out_len;
            
            free(temp);
            temp_out_len = 0;
            
            if(!ret) node_count = 0;
            
        }
        //free(node);
        
       
    }
}

SOLID_RESULT zdelta_patch(void* _args) {
    SOLID_DATA buffer = (SOLID_DATA)_args;
    do_patch_fd(buffer);
    return SPATCH_DONE;
}

SOLID_RESULT zdelta_diff(void* _args) {    
    diff_result diff;
    SOLID_DATA buffer = (SOLID_DATA)_args;
    if(buffer->fd.out != -1) {
        diff = do_diff_fd(buffer->in_buffer, buffer->fd.out, buffer->in_len, &buffer->out_len);
    } else {
        diff = do_diff(buffer->in_buffer, &buffer->out_buffer, buffer->in_len, &buffer->out_len);
    }
    
    switch(diff) {
        case DIFF_DONE: buffer->end_result = SDIFF_DONE;
        break;
        case DIFF_NULL_POINTER: buffer->end_result = SDIFF_NULL_POINTER;
        break;
        case DIFF_PIPE_ERROR: buffer->end_result = SPIPE_ERROR;
        break;
        default: fprintf(stderr, "ERROR: Diff error, %d %s\n", (int)diff, errorMsg);
            buffer->end_result = SDIFF_ERROR;
    }
    if(buffer->in_buffer)
        free(buffer->in_buffer);
    return buffer->end_result;
}
