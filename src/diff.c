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
 
 
diff_result do_diff(char *inBuffer, char *baseBuffer, char **deltaBuffer,size_t inLen, size_t baseLen, size_t *outLen) {

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

diff_result do_patch(char *deltaBuffer, char *baseBuffer, char **patchBuffer,size_t deltaLen, size_t baseLen, size_t *patchLen) {
    
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
