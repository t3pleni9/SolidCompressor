#include "dedup.h"

#include <fstream>
#include <iostream>
#include <stdlib.h>


using namespace std;
int main() {
        
        DeDup deDup;
        char * buffer;
        char * outBuffer = new char[2*SEG_S];
        char * tempBuffer;
        std::ifstream file ("/home/justin/temp4", std::ifstream::binary);
        buffer = new char[SEG_S];
        file.read(buffer, SEG_S);
        unsigned long int fileSize = deDup.deDuplicate(buffer, outBuffer, SEG_S);
        std::ofstream ofile ("dedup1.tmp", std::ofstream::binary);       
        if(fileSize) {
            ofile.write(outBuffer, fileSize);
        }
        ofile.close();
        file.close();
        
        file.open ("dedup1.tmp", std::ifstream::ate |std::ifstream::binary);        
        fileSize = file.tellg();
        file.close();           
        file.open("dedup1.tmp", std::ifstream::binary);
        delete outBuffer;
        outBuffer = new char[fileSize];
        file.read(outBuffer, fileSize);
        tempBuffer = new char[SEG_S];
        deDup.duplicate(outBuffer, tempBuffer);
        ofile.open ("/home/justin/outFile", std::ofstream::binary);
        ofile.write(tempBuffer, SEG_S);
        ofile.close();
        
        return 1;        
}
