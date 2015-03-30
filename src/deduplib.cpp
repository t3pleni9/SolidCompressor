#include <stdio.h>
#include <openssl/sha.h>
#include <cstring>
#include <unordered_map>

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <assert.h>




#include "deduplib.h"

_DeDup newDeDup() {
    return reinterpret_cast<void*>(new DeDup());
}

void delDeDup(_DeDup deDup) {
    delete reinterpret_cast<DeDup*>(deDup);
}

unsigned long int deDuplicate(_DeDup deDup, char *buffer, char *outBuffer, unsigned long int seg_s) {
    return reinterpret_cast<DeDup*>(deDup)->deDuplicate(buffer, outBuffer, seg_s);
}

void duplicate(_DeDup deDup, char *ddBuffer, char *buffer) {
    return reinterpret_cast<DeDup*>(deDup)->duplicate(ddBuffer, buffer);
}

void clearDictionary(_DeDup deDup) {
    return reinterpret_cast<DeDup*>(deDup)->clearDictionary();
}

SHA_CTX getHash(char* block, unsigned char* digest, unsigned int blockLen) {
    return Hash::getHash(block, digest, blockLen);
}

SHA_CTX getNextHash(char* block, unsigned char* digest, unsigned int blockLen, SHA_CTX context) {
    return Hash::getNextHash(block, digest, blockLen, context);
}
 

int generateIndex(IndexHeader node,unsigned int curBlock, int type, int buffer=0) {
    return Index::generateIndex(node, curBlock, type, buffer);
}

int writeIndex(char* index) {
    return Index::writeIndex(index);
}

int readIndex(char* index){
    return Index::readIndex(index);
}

int getIndexHeader(unsigned int key, IndexHeader* node) {
    return Index::getIndexHeader(key, node);
}

int getHeaderIndexCount() {
    return Index::getHeaderIndexCount();
}

void printIndex() {
    return Index::printIndex();
}


