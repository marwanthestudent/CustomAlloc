#include <stdlib.h>
#include <stdio.h>

int main()
{

    char *ptr = (char *)malloc(65535); // Allocate a large block
    char *ptr_array[2048];

    // Phase 1: Allocate smaller blocks
    for (int i = 0; i < 1024; i++)
    {
        ptr_array[i] = (char *)malloc(1024);
        if (!ptr_array[i])
        {
            return 1;
        }
    }

    free(ptr);

    // creating fragmentation by freeing every alternate block.
    for (int i = 0; i < 1024; i++)
    {
        if (i % 2 == 0)
        {
            free(ptr_array[i]);
            ptr_array[i] = NULL; //clear pointer to avoid dangling reference
            
        }
    }

    //allocate another large block to test heap growth
    ptr = (char *)malloc(65535);
    if (!ptr)
    {
        printf("Large allocation failed!\n");
    }
    else
    {
        free(ptr);
    }

    //Allocate blocks in fragmented spaces
    for (int i = 0; i < 1024; i++)
    {
        if (!ptr_array[i])
        {
            ptr_array[i] = (char *)malloc(512); //small size to fit in fragmented space
        }
    }

    //stress test with larger array
    for (int i = 1024; i < 2048; i++)
    {
        ptr_array[i] = (char *)malloc(1024); // 1KB blocks
        if (!ptr_array[i])
        {
            // allocation failed
            printf("stress test failed\n");
            break;
        }
    }

    for (int i = 0; i < 2048; i++)
    {
        if (ptr_array[i])
        {
            free(ptr_array[i]);
        }
    }

    return 0;
}