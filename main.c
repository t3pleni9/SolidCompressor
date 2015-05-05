

#include "solidlib.h"



int main(int argc, char **argv) {
        
      FILE *inFile, *outFile;
        /*char *buffer = (char*)malloc(SEG_S *sizeof(char));
        char *outBuffer = (char*)malloc(2*SEG_S *sizeof(char));
        
        size_t outLen = 0;
        */inFile = fopen("/home/justin/temp4", "rb");
        outFile = fopen("/home/justin/compFile.t.out", "wb");
        if(inFile) {
            //fread(buffer, SEG_S *sizeof(unsigned char), 1, inFile);
            solid_compress_fd(fileno(inFile), fileno(outFile));
            //printf("%d", outLen);
            //if(outBuffer)
            //    fwrite(outBuffer, sizeof(char), outLen, outFile);
            
        }
        fclose(inFile);
        fclose(outFile);
        return 1;
             
        
}
