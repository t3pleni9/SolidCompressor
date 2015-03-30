#include "deduplib.h"
#include "diff.h"


#include <stdlib.h>


int main(int argc, char **argv) {
        
        char arr[] = {"Hello world"};
        unsigned char* digest = (unsigned char*)malloc(140);
        getHash(arr, digest, strlen(arr));
        
        printf("%s", digest);
        
        
        
        return 1;        
}
