/*
 * queue.h
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


#ifndef QUEUE_H
#define QUEUE_H

#include "scons.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Node* PTRNODE;

#ifdef __cplusplus
}

class Queue
{
    public:
        Queue();
        void push(SOLID_DATA data);
        SOLID_DATA pop();
        int empty();
        SOLID_DATA front();
        virtual ~Queue();
    
    private:
        PTRNODE head;
        PTRNODE tail;
};

#endif 
#endif /* QUEUE_H */ 
