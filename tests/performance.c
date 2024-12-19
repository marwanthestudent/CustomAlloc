#include <stdio.h>
#include <stdlib.h>


int main(){
   int* def = malloc(100000);
   free(def);

    int block_sizes[] = {16, 64, 128, 512, 1024}; // random size blocks
    int num_allocations = 10000;
    void* allocations[num_allocations];
    
    // Allocation phase
    for (int i = 0; i < num_allocations; i++) {
        int size = block_sizes[rand() % 5];
        allocations[i] = malloc(size);
    }

    // Deallocation phase
    for (int i = 0; i < num_allocations; i++) {
        free(allocations[i]);
    }
    return 0;
}