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


#include "dedup.h"
#include "index.h"
#include <unordered_map>

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <assert.h>


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

void DeDup::deDuplicate(char fileName[]) {
    
    char * buffer;
    unsigned int blockCounter = 1;
    bool exists = false;    
    const int inc = BLOCK_S / BLOCK_X;
    int bufferSize = 0;
    
    std::ifstream file (fileName, std::ifstream::binary);
    std::ofstream ofile ("dedup.tmp", std::ofstream::binary);
    if (file) {
        buffer = new char[SEG_S];
        file.read(buffer, SEG_S); 
        unsigned long int segLength = file.gcount();
        //const unsigned long int SEG = segLength;
        unsigned long int curPointer = 0;
        
        while(segLength > 0) {
            
            if(BLOCK_S < segLength) {
                //std::cout<<"Buffer offset: "<<bufferSize<<" "<<curPointer<<" "<<segLength<<std::endl;
                if(!exists) {
                    bufferSize = deDuplicateSubBlocks(buffer, curPointer, inc, &segLength);
                    if(bufferSize) {
                        // Generate Index
                        Index::generateIndex(node.getParentIndex(),curPointer, 0, bufferSize - curPointer);
                        ofile.write((buffer+curPointer), bufferSize - curPointer);
                        
                        curPointer = bufferSize;
                        bufferSize = 0;
                    }
                    node = Index(curPointer, (buffer + curPointer), BLOCK_S);
                    //std::cout<<"BC I:"<<std::get<1>(node.getNode()).offsetPointer<<std::endl;
                } else {
                    node.rehashNode((buffer + curPointer), BLOCK_S);
                }
                std::unordered_map<std::string,IndexNode>::const_iterator tempNode;
                //std::cout<<std::get<0>(node.getNode())<<" BC:"<<blockCounter<<std::endl;
                if(nodeExists(std::get<0>(node.getNode()))) {
                    tempNode = strgIndex.find (std::get<0>(node.getNode()));
                    node.setParent(tempNode->second.offsetPointer, tempNode->second.size);
                    //std::cout<<"BC I:"<<std::get<1>(node.getNode()).offsetPointer<<" "<<segLength<<std::endl;
                    curPointer += BLOCK_S;
                    segLength -= BLOCK_S;
                    exists = true;
                    if(BLOCK_S > segLength) {
                        Index::generateIndex(node.getParentIndex(),node.getIndexNode().offsetPointer, 1, 0);
                    }
                } else {                    
                    strgIndex.insert(node.getNode());                    
                    if(!exists) {
                        ofile.write((buffer+curPointer), BLOCK_S);
                        
                        curPointer += BLOCK_S;
                        segLength -= BLOCK_S;
                                
                        if(BLOCK_S < segLength) {
                            node.rehashNode((buffer + curPointer), BLOCK_S);  
                            assert ( !nodeExists(std::get<0>(node.getNode())) );                            
                            strgIndex.insert(node.getNode());                            
                        } else {
                            //last block
                            ofile.write((buffer + curPointer), segLength);
                        }
                                                
                    } else {
                        //Generate Index
                        Index::generateIndex(node.getParentIndex(),node.getIndexNode().offsetPointer, 1, 0);
                        
                        exists = false;
                        //curPointer += BLOCK_S;
                        //segLength -= BLOCK_S;
                     }
                 }            
                     
            } else {
                //last block
                ofile.write((buffer + curPointer), segLength);
                Index::generateIndex(node.getParentIndex(),curPointer, 0, segLength);  
                segLength -= segLength;              
            }
            
            blockCounter++;
        }
     
        delete[] buffer;
        file.close();
        ofile.close();
        Index::writeIndex("index");
    }
}

void DeDup::duplicate(char fileName[]) {
    
    char *buffer, *inBuffer;
    Index temp;
    std::ifstream file ("dedup.tmp", std::ifstream::ate |std::ifstream::binary);
    unsigned long int fileSize = file.tellg();
    file.close();
    file.open("dedup.tmp", std::ifstream::binary);
    std::ofstream ofile (fileName, std::ofstream::binary);
    if (file) {
        Index::readIndex("index");
        buffer = new char[SEG_S];
        inBuffer = new char[fileSize];
        
        file.read(inBuffer, fileSize);     
           
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
    ofile.write(buffer, SEG_S);
    ofile.close();
    file.close();
}
        
