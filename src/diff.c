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
                        strcpy(errorMsg,"Delta not created");
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
        
        char        **fuzzy_hash_result;        
        NODEDP      node_array;
       	int         node_len;
        int         ret             = 0; 
        char        *node_buffer    = NULL;
        unsigned int blockCount     = inLen / DIFF_BLOCK, i, j;
        bit_feild   **done          = (bit_feild **)malloc(blockCount * sizeof(bit_feild *));
        
        if(out_len) {
            *out_len = 0;
        } else {
            perror("Out length not declared");
            return DIFF_NULL_POINTER;
        }
        
        node_array = (NODEDP)malloc((
            (inLen > blockCount * DIFF_BLOCK)?
                blockCount + 1 
                : blockCount
        ) * sizeof(NODESP));
        
        if(out_len) {
            *out_len = 0;
        } else {
            perror("Out length not Initiallised");
            return DIFF_NULL_POINTER;
        }
        
        
        fuzzy_hash_result = (char **)malloc((
            (inLen > blockCount * DIFF_BLOCK)?
                blockCount + 1 
                : blockCount
        ) * sizeof(char *));
        
        for(i = 0; i < blockCount ; i++) {
            done[i]                 = (bit_feild *)malloc(sizeof(bit_feild));
            done[i]->bit            = 1;
            node_array[i]           = (NODESP)malloc(sizeof(_node_t));
            fuzzy_hash_result[i]    = (char *)malloc(FUZZY_MAX_RESULT);
            
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
                    free(done[i]);
                }
                
                continue;
            }
            
            node_array[i]->ref_node     = -1;
            node_array[i]->node_size    = DIFF_BLOCK;
            node_array[i]->data         = (char *)malloc(DIFF_BLOCK * sizeof(char));
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
                free(done[i]);
            }

            for(j = i+1; j < blockCount ; j++) {
                if(!done[j]->bit)
                    continue;
                int sim = fuzzy_compare(fuzzy_hash_result[i],fuzzy_hash_result[j]);                
                if( sim >= DIFF_THLD) {
                    _do_diff(
                        (const unsigned char *)(inBuffer + j*DIFF_BLOCK), 
                        (const unsigned char *)(inBuffer + i*DIFF_BLOCK), 
                        &node_array[j]->data, 
                        DIFF_BLOCK, DIFF_BLOCK, 
                        &node_array[j]->node_size
                    );               
                    
                    node_array[j]->ref_node = i;
                   	done[j]->bit            = 0;     
                    
                    /* Debugger part*/
                     //fprintf(stderr,"i = %d j = %d %d %d\n", i, j, blockCount, node_array[j]->node_size);               
                     
                } else {
                    /*
                     * Do Nothing
                     */
                }
            }
            
            free(fuzzy_hash_result[i]);
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
                //fprintf(stderr,"node size: %d %d %d\n", node.node_size, blockCount, node_len);
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
            
            if(buffer->out_buffer)	free(buffer->out_buffer);
            
            buffer->out_buffer 	= (char *) malloc(buffer->out_len);
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
                
                if(buffer->out_buffer)	free(buffer->out_buffer);
                
                buffer->out_buffer	= 	(char *) malloc(buffer->out_len);
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
    if(buffer->fd.out != -1) {
        diff = do_diff_fd(buffer->in_buffer, buffer->fd.out, buffer->in_len, &buffer->out_len);
    } else {
        diff = do_diff(buffer->in_buffer, &buffer->out_buffer, buffer->in_len, &buffer->out_len);
    }
    
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
