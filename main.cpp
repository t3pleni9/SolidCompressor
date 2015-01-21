#include "dedup.h"
#include "hash.h"
#include "index.h"
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <unordered_set>
using namespace std;
int main() {
        
        DeDup deDup;
        //deDup.testImp();
        deDup.deDuplicate("/home/justin/temp.tar");
        /* deDup;
        /*string string_digest = 
        cout<<string_digest;*
        unsigned char digest[SHA_DIGEST_LENGTH];
        unsigned char digest1[SHA_DIGEST_LENGTH];
        char block[] = "HelloWorld";
        char temp1[] = "Hello";
        char temp2[] = "World";
        Hash::getHash((char*)block, (unsigned char*)digest);       
        printf("%d %d\n", 0, sizeof(digest));
        char mdString[SHA_DIGEST_LENGTH*2+1];
 
        for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
         sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
 
        printf("SHA1 digest: %s\n", mdString);
        strcpy(block, "Hello");
        SHA_CTX context = Hash::getHash((char*)temp1, (unsigned char*)digest1);
        strcpy(block, "World");
        Hash::getNextHash((char*)temp2, (unsigned char*)digest1, context);
        
        char mdString1[SHA_DIGEST_LENGTH*2];
 
        for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
         sprintf(&mdString1[i*2], "%02x", (unsigned int)digest1[i]);
        string dig(mdString1);
        printf("SHA1 digest: %s\n", mdString1);
        cout<<dig<<" "<<dig.length()<<" "<<sizeof(dig)<<" "<<sizeof(char);
        */
        return 0;        
}
