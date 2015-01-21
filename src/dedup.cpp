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

#include <iostream>


DeDup::DeDup()
{
	strgIndex = t_index(BLOCK_N);
}


DeDup::~DeDup()
{
    
}

void DeDup::testImp() {
    char temp1[6];
    char temp2[6];
    strcpy(temp1, "HELLO");
    std::string temp4("HELLO");
    strcpy(temp2, temp4.c_str());
    char temp3[] = "World";
    node.hashNode(1, (char*)temp1);
    std::cout<<std::get<0>(node.getNode())<<std::endl;
    std::pair<std::string, IndexNode> retValue = node.getNode();
    strgIndex.insert(retValue);
    node = Index(2,(char*)temp2);
    std::cout<<std::get<0>(node.getNode())<<std::endl;
    //node.rehashNode((char*)temp3);
    strgIndex.insert(node.getNode());
    std::cout<<strgIndex[std::get<0>(node.getNode())].offsetPointer<<" "<<sizeof(strgIndex)<<" "<<sizeof(node.getNode())<<std::endl;
}

void DeDup::deDuplicate(char fileName[]) {
    FILE *file = fopen(fileName, "r");
    FILE *outFile = fopen("dedup.dat", "w");
    
    char buffer[BLOCK_S];
    
    /* append null char at the end please*/
}
