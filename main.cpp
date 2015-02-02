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
        deDup.deDuplicate((char*)"/home/cse/temp4");
             
        return 0;        
}
