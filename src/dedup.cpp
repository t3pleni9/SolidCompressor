/*
 * dedup.cpp
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

//TODO:Include index in the deduplicated file.


#include <stdio.h>
#include <openssl/sha.h>
#include <cstring>
#include <unordered_map>

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <assert.h>

#include "index.h"
#include "dedup.h"


DeDup::DeDup()
{
	
}


DeDup::~DeDup()
{
    
}

bool DeDup::nodeExists(std::string hashValue) {
     
    //std::unordered_map<std::string,IndexNode>::const_iterator got = strgIndex.find (hashValue);
    //std::cout<<"Does Not:"<<(strgIndex.find (hashValue) == strgIndex.end())<<std::endl;
    return !(strgIndex.find (hashValue) == strgIndex.end());
}

void DeDup::clearDictionary() {
    strgIndex.clear();
}

int DeDup::readBlock(std::fstream* file, char* block, int block_s, int offset) {
    return 0;
}

int DeDup::deDuplicateSubBlocks(char* buffer, int curPointer, int inc, long unsigned int* segLength) {
        int nextBlockPointer = curPointer + (inc * BLOCK_X);
        int tempSeg = *segLength;
        curPointer += inc;
        tempSeg -= inc;
        int blockCounter = 1;
        
        while(curPointer != nextBlockPointer && BLOCK_S < tempSeg) {
            Index subNode(blockCounter, (buffer + curPointer), BLOCK_S);            
            if(nodeExists(std::get<0>(subNode.getNode()))) {
                *segLength = tempSeg;
                return curPointer;
            } else {
                curPointer += inc;
                tempSeg -= inc;
            }
        }
        return 0;//TODO: return Error cases
} 

unsigned long int DeDup::deDuplicate(char *buffer, char *outBuffer, unsigned long int seg_s ) {
    
    unsigned int blockCounter = 1;
    bool exists = false;    
    const int inc = BLOCK_S / BLOCK_X;
    int bufferSize = 0;
    char *tempBuff = new char[SEG_S];
    unsigned long int segLength = seg_s;
    unsigned long int curPointer = 0;
    unsigned long int indexOffset = 0;
    unsigned long int fileSize = 0;
    
   /* if(strgIndex.size() > BLOCK_N) {
        clearDictionary();
    }*/
    
    if (tempBuff) {        
        while(segLength > 0) {
            
            if(BLOCK_S < segLength) {
                if(!exists) {
                    bufferSize = deDuplicateSubBlocks(buffer, curPointer, inc, &segLength);
                    if(bufferSize) {
                        // Generate Index
                        Index::generateIndex(node.getParentIndex(),curPointer, 0, bufferSize - curPointer);
                        memcpy((tempBuff + fileSize), (buffer+curPointer), bufferSize - curPointer);
                        fileSize += (bufferSize - curPointer);
                        
                        curPointer = bufferSize;
                        bufferSize = 0;
                    }
                    node = Index(curPointer, (buffer + curPointer), BLOCK_S);
                } else {
                    node.rehashNode((buffer + curPointer), BLOCK_S);
                }
                
                std::unordered_map<std::string,IndexNode>::const_iterator tempNode;
                if(nodeExists(std::get<0>(node.getNode()))) {
                    tempNode = strgIndex.find (std::get<0>(node.getNode()));
                    node.setParent(tempNode->second.offsetPointer, tempNode->second.node.size);
                    curPointer += BLOCK_S;
                    segLength -= BLOCK_S;
                    exists = true;
                    if(BLOCK_S > segLength) {
                        Index::generateIndex(node.getParentIndex(),node.getIndexNode().offsetPointer, 1, 0);
                    }
                    
                } else {                    
                    strgIndex.insert(node.getNode());                    
                    if(!exists) {
                        memcpy((tempBuff + fileSize), (buffer+curPointer), BLOCK_S);
                        fileSize += BLOCK_S;
                        curPointer += BLOCK_S;
                        segLength -= BLOCK_S;
                                
                        if(BLOCK_S < segLength) {
                            node.rehashNode((buffer + curPointer), BLOCK_S);  
                            assert ( !nodeExists(std::get<0>(node.getNode())) );                            
                            strgIndex.insert(node.getNode());                            
                        } else {
                            //last block
                            memcpy((tempBuff + fileSize), (buffer+curPointer), segLength);
                            fileSize += segLength;
                        }
                                                
                    } else {
                        //Generate Index
                        Index::generateIndex(node.getParentIndex(),node.getIndexNode().offsetPointer, 1, 0);
                        exists = false;
                     }
                 }            
                     
            } else {
                //last block
                memcpy((tempBuff + fileSize), (buffer+curPointer), segLength);
                fileSize += segLength;
                Index::generateIndex(node.getParentIndex(),curPointer, 0, segLength);  
                segLength -= segLength;              
            }
            
            blockCounter++;
        }
     
            
        //Copying index into the buffer
        indexOffset = Index::writeIndex(outBuffer);   
            
        //Copying size into the buffer
        memcpy((outBuffer + indexOffset), (char*)&fileSize, sizeof(unsigned long int));
        indexOffset += sizeof(unsigned long int);
        
        //Copy compressed data into buffer
        memcpy((outBuffer + indexOffset), tempBuff, fileSize);
        //std::cout<<strgIndex.size()<<" "<<indexOffset<<" "<<fileSize<<std::endl; 
        delete[] tempBuff;
        return (fileSize + indexOffset); 
    }
    
    return 0;
}

void DeDup::duplicate(char *ddBuffer, char *buffer) {
    
    char *inBuffer;
    Index temp;
    unsigned int offset;
    offset = Index::readIndex(ddBuffer);
    unsigned long int fileSize = 0;
    memcpy((char*)&fileSize, (ddBuffer + offset), sizeof(unsigned long int));
    offset += sizeof(unsigned long int);    
    
    if (offset) {
                
        inBuffer = new char[fileSize];
        
        memcpy(inBuffer, (ddBuffer + offset), fileSize);     
           
        unsigned long int curPointer = 0;
        unsigned long int inBufPtr = 0;
        IndexHeader node;
        while(curPointer <= SEG_S) {
            int exists = Index::getIndexHeader(curPointer, &node);
            if(exists) {
                if(node.type == 0 ) {
                    memcpy((buffer + curPointer), (inBuffer + inBufPtr), node.size);
                    curPointer += node.size;
                    inBufPtr += node.size;
                    
                } else {
                    memcpy((buffer + curPointer), (buffer + node.block), (node.size+1) * BLOCK_S);
                    curPointer += (node.size+1) * BLOCK_S;
                }
            } else {
                memcpy((buffer + curPointer), (inBuffer + inBufPtr), BLOCK_S);
                curPointer += BLOCK_S;
                inBufPtr += BLOCK_S;
                       
            }   
        }     
    }
}
        
