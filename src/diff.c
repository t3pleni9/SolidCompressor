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
 
 rs_result diff_sig() {

    return (rs_result)1;
}
rs_result diff_delta(char *in_buffer, char *out_buffer, char *sig_buffer, int in_len, int sig_len) {
    
    
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
    
    return result;
}

rs_result diff_patch(char *basis_buffer, char *in_buffer, char *out_buffer,  int basis_len, int in_len) {
    
    FILE* in_file = fmemopen(in_buffer, in_len, "rb");
    FILE* basis_file = fmemopen(basis_buffer, basis_len, "rb");
    FILE* out_file = fmemopen(out_buffer, block_len, "wb");
    
    rs_stats_t stats;
    rs_result  result;
    
    result = rs_patch_file(basis_file, in_file, out_file, &stats);

    fclose(in_file);
    fclose(out_file);
    fclose(basis_file);
    
    return result;

}
