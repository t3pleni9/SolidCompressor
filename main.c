


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fuzzy.h>

#include "solidlib.h"
#include "diff.h"


int main(int argc, char **argv) {
        
        FILE *inFile;
        unsigned char *buffer = (unsigned char*)malloc(SEG_S *sizeof(unsigned char));
        unsigned char *outBuffer = NULL;
        size_t outLen = 0;
        inFile = fopen("/home/justin/temp4", "rb");
        if(inFile) {
            fread(buffer, SEG_S *sizeof(unsigned char), 1, inFile);
            _DeDup deDup = newDeDup();
            deDuplicat(deDup, buffer, char *outBuffer, unsigned long int seg_s)
            if(do_diff(buffer, &outBuffer, MAX_DIFF, &outLen) != DIFF_DONE) {
                printf("%s",errorMsg);
            }
        }
        fclose(inFile);
        return 1;
             
        
}
