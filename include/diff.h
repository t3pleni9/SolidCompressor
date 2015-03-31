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


#ifndef DIFF_H
#define DIFF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#include <zdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
    DIFF_DONE = 1,
    DIFF_NOT_DONE = 2,
    DIFF_ERROR = 3,
    
    PATCH_DONE = 4,
    PATCH_NOT_DONE = 5,
    PATCH_ERROR = 6
} diff_result;

diff_result do_diff(char *inBuffer, char *baseBuffer, char **deltaBuffer, size_t inLen, size_t baseLen, size_t *outLen);
diff_result do_patch(char *deltaBuffer, char *baseBuffer, char **patchBuffer, size_t deltaLen, size_t baseLen, size_t *patchLen);

#ifdef __cplusplus
}
#endif

#endif /* DIFF_H */ 
