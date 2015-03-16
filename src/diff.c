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
 
 typedef struct _node_{
    char * node;
    int node_len;
    struct _node_ *next;
} _node_t;

static void free_diff_chain(_node_t *diff_chain) {
    _node_t *temp = NULL;
    while(diff_chain != NULL) {
        temp = diff_chain;
        diff_chain = diff_chain->next;
        free(temp->node);
        free(temp);
    }    
}

static rs_result diff_delta(char *in_buffer, char *out_buffer, char *sig_buffer, int in_len, int sig_len, size_t *out_len) {
    
    rs_stats_t      stats;
    rs_result       result;
    rs_signature_t  *sumset;
    
    if(sig_buffer) {
        result = mem_loadsig(sig_buffer, &sumset, &stats, sig_len);        
    } else {
        return RS_MEM_ERROR;
    }

    if (result != RS_DONE)
        return result;
        
    if ((result = rs_build_hash_table(sumset)) != RS_DONE)
        return result;
    
    if(in_buffer && out_buffer) {
        result = mem_delta(sumset, in_buffer, out_buffer, &stats, in_len, out_len);
    } else {
        return RS_MEM_ERROR;
    }       
    
    if(out_len) 
        *out_len = stats.out_bytes;
    
    rs_free_sumset(sumset);

    return result;
}

static rs_result diff_patch(char *basis_buffer, char *in_buffer, char *out_buffer,  int basis_len, int in_len, size_t *out_len) {
     
    rs_stats_t stats;
    rs_result  result;
    
    if(basis_buffer && in_buffer && out_buffer) {    
        result = mem_patch(basis_buffer, in_buffer, out_buffer, &stats, basis_len, in_len, out_len);
    } else {
        result = RS_MEM_ERROR;
    }    
    
    if(out_len) 
        *out_len = stats.out_bytes;
    
    return result;

}

static rs_result diff_sig(char *basis_buffer, char *sig_buffer,  int basis_len, size_t *sig_len) {
     
    rs_stats_t      stats;
    rs_result       result;
       
    result = mem_sig(basis_buffer, sig_buffer, basis_len, sig_len, &stats);
          
    if (result != RS_DONE)
        return result;
        
    return result;
}

diff_result do_diff(char *in_buffer, char * out_buffer, size_t in_len, size_t *out_len) {
    char *sig_buffer = NULL, *new_buffer = NULL;
    _node_t *diff_chain = NULL, *diff_chain_tail = NULL, *temp_node = NULL;
    size_t offset = 0, new_len = 0;
    size_t sig_len = DEL_BLOCK;
    rs_result rs;
    diff_result dr;
    if(out_len) 
        out_len = 0;
    
    sig_buffer = (char*)malloc(sig_len*sizeof(char));    
    diff_chain = (_node_t *)malloc(sizeof(_node_t));
    
    diff_chain->node = in_buffer;
    diff_chain->node_len = DEL_BLOCK;
    diff_chain->next = NULL;
    
    diff_chain_tail = diff_chain;
    
    rs = diff_sig(in_buffer, sig_buffer, DEL_BLOCK, &sig_len);
    if(rs != RS_DONE && rs == RS_MEM_ERROR) {
        dr = DIFF_NOT_DONE;
    } else if(rs  == RS_DONE) {
        offset += DEL_BLOCK;
    }
    
    
    while((offset + DEL_BLOCK) <= in_len && dr != DIFF_NOT_DONE){
        new_buffer = (char *)malloc(DEL_BLOCK * sizeof(char));
        temp_node = (_node_t*)malloc(sizeof(_node_t));
        rs = diff_delta((in_buffer + (offset * sizeof(char))), new_buffer, sig_buffer, DEL_BLOCK, sig_len, &new_len);
        if(rs != RS_DONE && rs == RS_MEM_ERROR) {
             temp_node->node_len = DEL_BLOCK;
        } else if(rs  == RS_DONE) {
             temp_node->node_len = new_len;
        } else {
           dr =  DIFF_OTHER_ERROR;
           break;
        }
        
        temp_node->node = (in_buffer + (offset * sizeof(char)));       
        temp_node->next = NULL;
        out_len += temp_node->node_len;
        diff_chain_tail->next = temp_node;
        diff_chain_tail = temp_node;
        
        offset += DEL_BLOCK;
        printf("offset %d ", offset);
    }
    
    //last block
    temp_node = (_node_t*)malloc(sizeof(_node_t));
    temp_node->node = (in_buffer + (offset * sizeof(char)));   
    temp_node->node_len = in_len - offset;    
    temp_node->next = NULL;
    diff_chain_tail->next = temp_node;
    diff_chain_tail = temp_node;    
    
    free_diff_chain(diff_chain_tail);
    free(sig_buffer);
    return dr;
}
