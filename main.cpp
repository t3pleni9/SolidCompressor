#include "dedup.h"
//#include "diff.h"

#include <fstream>
#include <iostream>
#include <stdlib.h>


using namespace std;
int main(int argc, char **argv) {
        
        IndexNode status;
        std::cout<<sizeof(status)<<std::endl;
        
        DeDup deDup;
        char * buffer = NULL;
        char * outBuffer = NULL;
        char * tempBuffer;
        char * fileName = NULL;
        int iterations = 0;
        unsigned long int fileSize = 0;
        unsigned long int outFileSize = 0;
       /* if(argc > 1) {
            strcpy(fileName, argv[1]);
        }*/
        std::ifstream file;
        std::ofstream ofile ("dedup1.tmp", std::ofstream::binary); 
       // if(fileName) {
            file.open ("/media/disk1/myStuff/docs/Mtech/project_temp/compFile.t", std::ifstream::ate|std::ifstream::binary);
            fileSize = file.tellg();
            std::cout<<fileSize<<std::endl;
            file.close();
            file.open("/media/disk1/myStuff/docs/Mtech/project_temp/compFile.t", std::ifstream::binary);
            while(fileSize > SEG_S) {
                buffer = new char[SEG_S];
                outBuffer = new char[2*SEG_S];
                file.read(buffer, SEG_S);
                //std::cout<<buffer<<std::endl;
                //break;
                outFileSize = deDup.deDuplicate(buffer, outBuffer, SEG_S);
                if(outFileSize) {
                    std::cout<<outFileSize<<" "<<fileSize<<" "<<SEG_S<<std::endl;
                    ofile.write(outBuffer, outFileSize);
                }
                delete [] outBuffer;
                fileSize -= SEG_S;
                iterations++;
            }
            
            if(fileSize) {
                buffer = new char[fileSize];
                outBuffer = new char[2*SEG_S];
                file.read(buffer, fileSize);
                outFileSize = deDup.deDuplicate(buffer, outBuffer, fileSize);
                if(outFileSize) {
                    ofile.write(outBuffer, outFileSize);
                }
                //delete [] outBuffer;
                //iterations++;
            }
        //}
        
        ofile.close();
        file.close();
        /*tempBuffer = new char[fileSize];
        memcpy(tempBuffer, outBuffer, fileSize);
        std::ofstream ofile ("dedup1.tmp", std::ofstream::binary);       
        if(fileSize) {
            ofile.write(outBuffer, fileSize);
        }
        delete[] outBuffer;
        //ofile.close();
        file.close();
        ofile.close();
        //delete[] outBuffer;
        //delete[] buffer;
        buffer = new char[fileSize];
        size_t out_len = 0;
        std::cout<<fileSize<<"\n";
        do_diff(tempBuffer, buffer, fileSize, &out_len);
        std::cout<<out_len<<std::endl;
        */
        /*
        file.open ("dedup1.tmp", std::ifstream::ate |std::ifstream::binary);     
        ofile.open ("/home/justin/outFile", std::ofstream::binary);   
        fileSize = file.tellg();
        std::cout<<"file Size:"<<fileSize<<std::endl;
        file.close();           
        file.open("dedup1.tmp", std::ifstream::binary);
        
       // while(iterations) {
            outBuffer = new char[fileSize];
            file.read(outBuffer, fileSize);
            tempBuffer = new char[SEG_S];
            deDup.duplicate(outBuffer, tempBuffer);        
            ofile.write(tempBuffer, SEG_S);
            iterations--;
            //delete [] outBuffer;
            delete [] tempBuffer;
       // }
        ofile.close();
        
        */
        
        return 1;        
}
