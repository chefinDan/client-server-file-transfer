#include <stdio.h>
#include "cvector.h"

int main(){
    int it, i;
    
    ARRAY_CREATE(char, array);
    
    for(i = 0; i < 10000000; ++i){
        for(it = 65; it<90; ++it ){
            ARRAY_PUSH(array, it);
        }
    }

    printf("array size: %ld", ARRAY_SIZE(array));
    printf("array capacity: %ld", ARRAY_CAPACITY(array));

    return 0;



}


