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



Delta::Delta()
{
    
}


Delta::~Delta()
{
    
}

void Delta::doDiff(char *inbuffer, char *outBuffer, int bufLen) {
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
}
