


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "solidlib.h"

int main(int argc, char **argv) {
        
        char arr[] = {"Hello world big"};
        char arr1[] = {"Hello world"};
        char *delt = NULL;
        char *patch = NULL;
        size_t delta = 0;
        int res = (int)do_diff(arr, arr1, &delt, strlen(arr), strlen(arr1), &delta);
        res = (int)do_patch(delt, arr1, &patch, delta, strlen(arr1), &delta);
        printf("%d %d %s", res, delta, patch);
        return 1;        
        
}
