#include<time.h>

#include "solidlib.h"


// Junk does get written.

int main(int argc, char **argv) {
        clock_t begin, end;
        double time_spent;
        char mybuf[4096];
        setvbuf(stdin, mybuf, _IOFBF, 4096);
        FILE *inFile, *outFile;
        inFile = fopen("/home/justin/bzip2-1.0.6.tar", "rb");
        outFile = fopen("/home/justin/temp.out", "wb");
        if(inFile) {
            begin = clock();
            solid_compress_fd(fileno(inFile), fileno(outFile));
            end = clock();
            time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
            printf("time: %f\n", time_spent);
        }
        fclose(inFile);
        fclose(outFile);
        
        return 1;
}
