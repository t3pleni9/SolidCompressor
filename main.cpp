#include "dedup.h"
#include <stdio.h>
#include <cstring>
#include <iostream>

int main() {
        
        DeDup deDup;
        /*string string_digest = 
        cout<<string_digest;*/
        char digest[SHA_DIGEST_LENGTH];
        deDup.getHash("HelloWorld", (unsigned char*)digest);       
        
        char mdString[SHA_DIGEST_LENGTH*2+1];
 
        for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
         sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
 
        printf("SHA1 digest: %s\n", mdString);
        
        SHA_CTX context = deDup.getHash("Hello", (unsigned char*)digest);
        deDup.getNextHash("World", (unsigned char*)digest, context);
        
        char mdString1[SHA_DIGEST_LENGTH*2+1];
 
        for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
         sprintf(&mdString1[i*2], "%02x", (unsigned int)digest[i]);
 
        printf("SHA1 digest: %s\n", mdString1);
        
}
