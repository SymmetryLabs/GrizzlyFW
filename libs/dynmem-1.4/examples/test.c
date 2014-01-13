#include <stdio.h>
#include "rtl_malloc.h"

#define KiloByte  1024
#define POOL_SIZE 1024


// Pool size is in Kbytes.
char pool[POOL_SIZE*KiloByte];


main(){
    int *ptr[100];
    int i, free_mem;

    free_mem = init_memory_pool(0, 5, POOL_SIZE, pool);
    printf("Total free memory= %d\n", free_mem);

    // default malloc and free use this buffer.
    associate_buffer(pool);


    for (i=0; i< 100; i++)
	if (!(ptr[i]=rtl_malloc(100))){
	    printf("Error\n");
	    exit(-1);
	}

    for (i=0; i< 100; i++)
	rtl_free(ptr[i]);


    destroy_memory_pool(pool);
    printf("Test OK\n");
    exit(0);
}

    
	
