

#include "solidlib.h"



int main(int argc, char **argv) {
        
      FILE *inFile;
        char *buffer = (char*)malloc(SEG_S *sizeof(char));
        char *outBuffer = (char*)malloc(2*SEG_S *sizeof(char));
        
        size_t outLen = 0;
        inFile = fopen("/home/justin/temp4", "rb");
        if(inFile) {
            fread(buffer, SEG_S *sizeof(unsigned char), 1, inFile);
            solid_compress(buffer, outBuffer, SEG_S, &outLen);
            printf("%d", outLen);
            
        }
        fclose(inFile);
        return 1;
             
        
}
