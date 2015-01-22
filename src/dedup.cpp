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
	//strgIndex = t_index(BLOCK_N);
}


DeDup::~DeDup()
{
    
}

void DeDup::testImp() {
    char temp[] = {'h','e'};
    char temp1[] = {'\0'};
    char temp2[] = {'h', 'e'};
    //strcpy(temp1, "HELLO");
    std::string temp4("HELLO");
    //strcpy(temp2, temp4.c_str());
    char temp3[] = {'\0'};
    node.hashNode(1, (char*)temp, 2);
    //node.rehashNode((char*)temp1);
    std::cout<<std::get<0>(node.getNode())<<std::endl;
    std::pair<std::string, IndexNode> retValue = node.getNode();
    strgIndex.insert(retValue);
    node = Index(2,(char*)temp2, 2);
    std::cout<<nodeExists(std::get<0>(node.getNode()))<<std::endl;
   // node.rehashNode((char*)temp3);
    nodeExists(std::get<0>(node.getNode()));
    std::cout<<std::get<0>(node.getNode())<<std::endl;
    std::cout<<"Of:"<<strgIndex[std::get<0>(node.getNode())].offsetPointer<<std::endl;
    strgIndex.insert(node.getNode());
    std::cout<<strgIndex[std::get<0>(node.getNode())].offsetPointer<<" "<<std::get<0>(node.getNode())<<" "<<sizeof(node.getNode())<<std::endl;
}

bool DeDup::nodeExists(std::string hashValue) {
     
    //std::unordered_map<std::string,IndexNode>::const_iterator got = strgIndex.find (hashValue);
    std::cout<<"Does Not:"<<(strgIndex.find (hashValue) == strgIndex.end())<<std::endl;
    return !(strgIndex.find (hashValue) == strgIndex.end());
}

int DeDup::readBlock(std::fstream* file, char* block, int block_s, int offset) {
    return 0;
}

void DeDup::deDuplicate(char fileName[]) {
    /*FILE *file = fopen(fileName, "r");
    FILE *outFile = fopen("dedup.dat", "w");
    
    char buffer[BLOCK_S];
    */
    char * buffer;
    unsigned int blockCounter = 1;
    bool exists = false;
    
    std::ifstream file (fileName, std::ifstream::binary);
    std::ofstream ofile ("dedup.tmp", std::ofstream::binary);
    if (file) {
        while(!file.eof()) {
            if(blockCounter == 1) {
                buffer = new char[BLOCK_S];
                file.read(buffer, BLOCK_S); // one byte less for '\000'
                //buffer[BLOCK_S-1] = '\0';
            }
            if(file) {
                if(!exists) {
                    node = Index(blockCounter, buffer, BLOCK_S);
                    std::cout<<"BC I:"<<std::get<1>(node.getNode()).offsetPointer<<std::endl;
                } else {
                    node.rehashNode(buffer, BLOCK_S);
                }
                std::cout<<std::get<0>(node.getNode())<<" BC:"<<blockCounter<<std::endl;
                if(nodeExists(std::get<0>(node.getNode()))) {
                    exists = true;
                } else {
                    strgIndex.insert(node.getNode());
                    if(!exists) {
                        ofile.write(buffer, BLOCK_S);
                        
                        delete[] buffer;
                        buffer = new char[BLOCK_S];
                        file.read(buffer, BLOCK_S); // one byte less for '\000'
                        //buffer[BLOCK_S-1] = '\0';
                        
                        if(file) {
                            node.rehashNode(buffer, BLOCK_S);                            
                            assert ( !nodeExists(std::get<0>(node.getNode())) );                            
                            strgIndex.insert(node.getNode());
                            std::cout<<std::get<0>(node.getNode())<<" "<<strgIndex[std::get<0>(node.getNode())].offsetPointer<<std::endl;
                        } else {
                            ofile.write(buffer, BLOCK_S);
                        }                        
                    } else {
                        /**
                         * Generate Index
                         **/
                        exists = false;
                        delete[] buffer;
                        buffer = new char[BLOCK_S];
                        file.read(buffer, BLOCK_S); // one byte less for '\000'
                        //buffer[BLOCK_S-1] = '\0';
                     }
                 }            
                     
            } else {
                ofile.write(buffer, BLOCK_S);
            }
            
            //delete[] buffer;
            blockCounter++;
        }
        delete[] buffer;
        file.close();
        ofile.close();
    }
    
    /* append null char at the end please*/
}
