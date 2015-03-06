#include "dedup.h"

#include <fstream>
#include <iostream>


using namespace std;
int main() {
        
        DeDup deDup;
        char * buffer;
        char * outBuffer = new char[2*SEG_S];
        std::ifstream file ("/home/justin/temp4", std::ifstream::binary);
        buffer = new char[SEG_S];
        file.read(buffer, SEG_S);
        
        
        deDup.deDuplicate(buffer, outBuffer, SEG_S);
        
        unsigned int size = Index::getHeaderIndexCount();
        std::cout<<size*sizeof(IndexHeader)<<std::endl;
        char *headerIndex = new char[size*sizeof(IndexHeader)+sizeof(unsigned int)];
        unsigned int tempSize = Index::writeIndex(headerIndex);
        std::cout<<size<<" "<<tempSize<<std::endl;
        memcpy((char*)&size, headerIndex, sizeof(unsigned int));
        std::cout<<size<<" "<<tempSize<<std::endl;
        
        tempSize = Index::readIndex((headerIndex));
        std::cout<<tempSize<<std::endl;
        deDup.duplicate((char*)"/home/justin/outFile"); 
        
        
        
        //Index::writeIndex("index");             
        /*IndexNode temp, temp2;
        IndexHeader temp3, temp4;
        temp3.offsetPointer = 1;
    temp3.block = 9;
    
        temp.offsetPointer = 123;
        temp.size = 1234; /**<  number of blocks*
        char *ch = new char[1];
         memcpy ( ch, (char*)&temp, sizeof(IndexNode) );
         temp.size = 134;
          memcpy ( (ch+sizeof(IndexNode)), (char*)&temp, sizeof(IndexNode) );
          memcpy ( (ch+2*sizeof(IndexNode)), (char*)&temp3, sizeof(IndexHeader) );
         memcpy ( (char*)&temp2, ch, sizeof(IndexNode) );
         std::cout<<temp2.offsetPointer<<" "<<temp2.size<<std::endl;
          memcpy ( (char*)&temp2, (ch+sizeof(IndexNode)), sizeof(IndexNode) );
         std::cout<<temp2.offsetPointer<<" "<<temp2.size<<std::endl;
         memcpy ( (char*)&temp4, (ch+2*sizeof(IndexNode)), sizeof(IndexHeader) );
         std::cout<<temp4.offsetPointer<<" "<<temp4.block<<std::endl;
         delete ch;*/

        return 1;        
}
