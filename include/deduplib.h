
#include <openssl/sha.h>
#include "index_struct.h"

//-lsolidComp
#ifdef __cplusplus
extern "C" {
#endif

typedef void* _DeDup;

_DeDup newDeDup();
void delDeDup(_DeDup);
unsigned long int deDuplicate(_DeDup, char*, char *, unsigned long int);
void duplicate(_DeDup, char*, char*);
void clearDictionary(_DeDup);

SHA_CTX getHash(char*, unsigned char*, unsigned int);
SHA_CTX getNextHash(char*, unsigned char*, unsigned int, SHA_CTX);
 
int generateIndex(IndexHeader,unsigned int, int, int);   
int writeIndex(char*);
int readIndex(char*);
int getIndexHeader(unsigned int, IndexHeader*);
int getHeaderIndexCount();
void printIndex();


#ifdef __cplusplus
}
#endif
