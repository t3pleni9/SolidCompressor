/*
 * queue.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"



#ifdef __cplusplus
extern "C" {
#endif

struct Node {
    t_solid_data buffer;
    struct Node* next;
};

#ifndef __cplusplus
typedef struct Node Node;
#endif


#ifdef __cplusplus
}
#endif

Queue::Queue()
{
    tail = NULL;
    head = NULL;
}
void Queue::push(SOLID_DATA buffer) {
    PTRNODE node = (PTRNODE)malloc(sizeof(Node));
    node->next = NULL;
    memcpy(&node->buffer, buffer, sizeof(t_solid_data));
    node->buffer.in_buffer = (char *)malloc(node->buffer.in_len);
    memcpy(node->buffer.in_buffer, buffer->in_buffer, buffer->in_len);
    if(empty()) {
        head = tail = node;
    } else {
        tail->next = node;
        tail = node;
    }
}

SOLID_DATA Queue::pop() {
    if(head) {
        SOLID_DATA buffer = (SOLID_DATA)malloc(sizeof(t_solid_data));
        memcpy(buffer, &head->buffer, sizeof(t_solid_data));
        buffer->in_buffer = (char *)malloc(buffer->in_len);
        memcpy(buffer->in_buffer, head->buffer.in_buffer, buffer->in_len);
        free(head->buffer.in_buffer);
        PTRNODE temp = head;
        head = head->next;
        free(temp);
        if(head == NULL) {
            tail = NULL;
        }
        return buffer;
    } else {
        return NULL;
    }
}
inline int Queue::empty(){
    return head == NULL;
}

inline SOLID_DATA Queue::front() {
    if(head) {
        return (&head->buffer);
    } else {
        return NULL;
    }
}

Queue::~Queue()
{
    while(!empty()) {
        SOLID_DATA temp = (SOLID_DATA)pop();
        if(temp) {
            free(temp);
        }
    }
}   

