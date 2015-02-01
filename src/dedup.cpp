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



#include "dedup.h"
#include "index.h"
#include <unordered_map>

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <assert.h>


DeDup::DeDup()
{
	strgIndex = t_index(BLOCK_N);
}


DeDup::~DeDup()
{
    
}

void DeDup::testImp() {
    char temp[] = {"HELLO"};
    char temp1[] = {"WORLD"};
    char temp2[] = {"HELLOWORLD"};
    char temp3[] = {"helloworld"};
 
    node.hashNode(1, (char*)temp, strlen(temp));
    
    std::cout<<std::get<0>(node.getNode())<<std::endl;
    std::pair<std::string, IndexNode> retValue = node.getNode();
    strgIndex.insert(retValue);
    node.rehashNode((char*)temp1, strlen(temp1));
    strgIndex.insert(node.getNode());
    node = Index(2,(char*)temp2, strlen(temp2));
    std::cout<<"Exists temp2:"<<nodeExists(std::get<0>(node.getNode()))<<std::endl;
    node.hashNode(3, (char*)temp3, strlen(temp3));
    std::cout<<"Exists temp3:"<<nodeExists(std::get<0>(node.getNode()))<<std::endl;
    strgIndex.insert(node.getNode());
    std::cout<<strgIndex[std::get<0>(node.getNode())].offsetPointer<<" "<<std::get<0>(node.getNode())<<" "<<sizeof(node.getNode())<<std::endl;
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
        
        while(BLOCK_S < segLength) {
            if(BLOCK_S < segLength) {
                if(!exists) {
                    node = Index(blockCounter, (buffer + curPointer), BLOCK_S);
                    bufferSize = deDuplicateSubBlocks(buffer, curPointer, inc, &segLength);
                    if(bufferSize) {
                        // TODO: Generate Index
                        ofile.write((buffer+curPointer), bufferSize - curPointer);
                        //std::cout<<"Buffer offset: "<<bufferSize<<" "<<curPointer<<" "<<segLength<<std::endl;
                        curPointer = bufferSize;
                        bufferSize = 0;
                    }
                    
                    //std::cout<<"BC I:"<<std::get<1>(node.getNode()).offsetPointer<<std::endl;
                } else {
                    node.rehashNode((buffer + curPointer), BLOCK_S);                    
                }
                
                //std::cout<<std::get<0>(node.getNode())<<" BC:"<<blockCounter<<std::endl;
                if(nodeExists(std::get<0>(node.getNode()))) {
                    exists = true;
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
                            //std::cout<<std::get<0>(node.getNode())<<" "<<strgIndex[std::get<0>(node.getNode())].offsetPointer<<std::endl;
                        } else {
                            ofile.write((buffer + curPointer), segLength);
                        }
                                                
                    } else {
                        //TODO: Generate Index
                        exists = false;
                        curPointer += BLOCK_S;
                        segLength -= BLOCK_S;
                     }
                 }            
                     
            } else {
                ofile.write((buffer + curPointer), segLength);
            }
            
            blockCounter++;
        }
     
        delete[] buffer;
        file.close();
        ofile.close();
    }
}
