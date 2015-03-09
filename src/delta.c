/*
 * delta.cpp
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


#include "delta.h"



rs_result mem_diff(char *in_buffer, char *out_buffer, char *sig_buffer, int in_len, int sig_len) {
    
    
    FILE* in_file = fmemopen(in_buffer, in_len, "rb");
    FILE* sig_file = fmemopen(sig_buffer, sig_len, "rb");
    FILE* out_file = fmemopen(out_buffer, block_len, "wb");
    
    
    rs_stats_t      stats;
    rs_result       result;
    rs_signature_t  *sumset;
    
    result = rs_loadsig_file(sig_file, &sumset, &stats);
    if (result != RS_DONE)
        return result;
        
    if ((result = rs_build_hash_table(sumset)) != RS_DONE)
        return result;
    
    result = rs_delta_file(sumset, in_file, out_file, &stats);

    rs_free_sumset(sumset);

    fclose(out_file);
    fclose(in_file);
    fclose(sig_file);
    
    
 /* Use fmemopen to create memory streams.
  * Use the following functions from librsync to do the delta compression part
  * rs_result rs_sig_file(FILE *old_file, FILE *sig_file,
                      size_t block_len, size_t strong_len, rs_stats_t *); 
  * rs_result rs_loadsig_file(FILE *, rs_signature_t **, rs_stats_t *);
  * rs_result rs_file_copy_cb(void *arg, rs_long_t pos, size_t *len, void **buf);
  * rs_result rs_delta_file(rs_signature_t *, FILE *new_file, FILE *delta_file, rs_stats_t *);
  * rs_result rs_patch_file(FILE *basis_file, FILE *delta_file, FILE *new_file, rs_stats_t *);
  * 
  */
  
  return result;
}

rs_result mem_patch(char *basis_buffer, char *in_buffer, char *out_buffer,  int basis_len, int in_len) {
    
    FILE* in_file = fmemopen(in_buffer, in_len, "rb");
    FILE* basis_file = fmemopen(basis_buffer, basis_len, "rb");
    FILE* out_file = fmemopen(out_buffer, block_len, "wb");
    
    rs_stats_t          stats;
    rs_result           result;
    
    result = rs_patch_file(basis_file, in_file, out_file, &stats);

    fclose(in_file);
    fclose(out_file);
    fclose(basis_file);
    
    return result;

}

rs_result rdiff_sig(char *basis_buffer, char *sig_buffer,  int basis_len) {

    FILE* sig_file = fmemopen(sig_buffer, SIG_BLOCK, "wb");
    FILE* basis_file = fmemopen(basis_buffer, basis_len, "rb");
    
    rs_stats_t      stats;
    rs_result       result;
    //rs_long_t       sig_magic = RS_MD4_SIG_MAGIC;
    
    result = rs_sig_file(basis_file, sig_file, block_len, strong_len, 
                            &stats);

    fclose(sig_file);
    fclose(basis_file);
    if (result != RS_DONE)
        return result;
        
    return result;
}
