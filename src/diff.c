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

char errorMsg[100];

static void free_node(NODESP node) {
    if(node->data) {
        free(node->data);
    printf("clearing done data\n");
    }if(node->fuzzy_hash_result){
        free(node->fuzzy_hash_result);
    printf("clearing done hash\n");}
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

diff_result do_patch(char *deltaBuffer, char *baseBuffer, char **patchBuffer, size_t deltaLen, size_t baseLen, size_t *patchLen) {
    
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

diff_result do_diff(char *inBuffer, char **outBuffer, size_t inLen, size_t *out_len) {
        time_t t;
        srand((unsigned) time(&t));
        unsigned int blockCount = inLen / DIFF_BLOCK, i, j;
        char *delta;
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
            fprintf(stderr,"i = %d %d\n", i, blockCount);
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
                     _do_diff((const unsigned char *)(inBuffer + j*DIFF_BLOCK), (const unsigned char *)(inBuffer + i*DIFF_BLOCK), &delta, DIFF_BLOCK, DIFF_BLOCK, &deltaLen);               
                    node_array[j] = (NODESP)malloc(sizeof(_node_t));
                    node_array[j]->ref_node = i;
                    node_array[j]->node_size = deltaLen;
                    node_array[j]->data = (char *)malloc(deltaLen * sizeof(char));
                    *out_len += (node_array[j]->node_size + sizeof(int) + sizeof(size_t));
                    memcpy(node_array[j]->data, delta, deltaLen);
                     
                    if(!delta) {
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
