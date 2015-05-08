

#include "solidlib.h"



int main(int argc, char **argv) {
        char mybuf[4096];
        setvbuf(stdin, mybuf, _IOFBF, 4096);
        FILE *inFile, *outFile;
        //char *buffer = (char*)malloc(SEG_S *sizeof(char));
        //char *outBuffer = (char*)malloc(2*SEG_S *sizeof(char));
        
        //size_t outLen = 0;
        inFile = fopen("/home/justin/compFile.t", "rb");
        outFile = fopen("/home/justin/compFile.t.out4", "wb");
        if(inFile) {
            //fread(buffer, SEG_S *sizeof(unsigned char), 1, inFile);
            solid_compress_fd(fileno(inFile), fileno(outFile));
            //solid_compress(buffer, outBuffer, SEG_S, &outLen);
            //printf("%d", outLen);
            //if(outBuffer)
            //    fwrite(outBuffer, sizeof(char), outLen, outFile);
            
        }
        fclose(inFile);
        fclose(outFile);
        //test_node();
        return 1;
             
        
}
