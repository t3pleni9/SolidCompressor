#include<time.h>

#include "solidlib.h"
#define COMP 0

// Junk does get written.

int main(int argc, char **argv) {
        clock_t begin, end;
        double time_spent;
        FILE *inFile, *outFile;
        #if COMP == 1
        inFile = fopen("/media/disk1/test.tar", "rb");
        outFile = fopen("/media/disk1/test.tar.gz", "wb");
        #else
        inFile = fopen("/media/disk1/test.tar.gz", "rb");
        outFile = fopen("/media/disk1/test.out.tar", "wb");
        #endif
        if(inFile) {
            begin = clock();
            #if COMP == 1
            solid_compress_fd(fileno(inFile), fileno(outFile));
            #else
            solid_de_compress_fd(fileno(inFile), fileno(outFile));
            #endif
            end = clock();
            time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
            printf("time: %f\n", time_spent);
        }
        fclose(inFile);
        fclose(outFile);
        return 1;
}
