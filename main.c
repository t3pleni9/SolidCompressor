


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fuzzy.h>

#include "solidlib.h"
#include "diff.h"


int main(int argc, char **argv) {
        
        FILE *inFile;
        char *buffer = (char*)malloc(SEG_S *sizeof(char));
        unsigned char *outBuffer = (unsigned char*)malloc(2*SEG_S *sizeof(unsigned char));
        
        unsigned char * tempBuffer = NULL;
        size_t outLen = 0;
        inFile = fopen("/home/justin/temp4", "rb");
        if(inFile) {
            fread(buffer, SEG_S *sizeof(unsigned char), 1, inFile);
            _DeDup deDup = newDeDup();
            size_t fileSize = deDuplicate(deDup, buffer, outBuffer, SEG_S);
            if(do_diff(outBuffer, &tempBuffer, fileSize, &outLen) != DIFF_DONE) {
                printf("%s",errorMsg);
            }
        }
        fclose(inFile);
        return 1;
             
        
}
