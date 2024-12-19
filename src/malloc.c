#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)     ((b) + 1)
#define BLOCK_HEADER(ptr) ((struct _block *)(ptr) - 1)

static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

//pointer to use for NF
struct _block* last_allocated = NULL;


void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes     */ 
   struct _block *next;  /* Pointer to the next _block of allocated memory      */ 
   struct _block *prev;  /* Pointer to the previous _block of allocated memory  */ 
   bool   free;          /* Is this _block free?                                */ 
   char   padding[3];    /* Padding: IENTRTMzMjAgU3jMDEED                       */
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */

void find_frag_fraction(){//implement frag fraction
   struct _block* cpy = heapList;
   float max_free_block = 0;
   
   while(cpy){

      if(cpy->free && (cpy->size > max_free_block)){
         max_free_block = cpy->size;
      }
      cpy = cpy->next;
   }
   
   printf("This block is %0.2f%% fragmented \n",
   100*(1 - (max_free_block/(num_requested - max_heap))));
}



/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;

   /* First fit */
   // Start incrementing from the start of the list
   // If a block is found while running the list the
   // Make last point to block before free block.

#if defined FIT && FIT == 0

   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }


#endif

   /* Best fit */
   // Find the smallest block that can fit the block we're allocating.
   // Iterate through the list from start to finish

#if defined BEST && BEST == 0
   struct _block* best_p = NULL;
   size_t prev_size = INT32_MAX;

   if(heapList)
      prev_size = heapList->size;
      
   while(curr){

      if(curr->free && (curr->size >= size) && (prev_size > curr->size)){
         best_p = curr;
         prev_size = curr->size;
      }
      *last = curr;
      curr = curr->next;
   }
   curr = best_p;

#endif

   /* Worst fit */
   // Find the largest block that can fit the block we're allocating.
   // Iterate through the list from start to finish
   
#if defined WORST && WORST == 0
   struct _block* worst_p = NULL;
   size_t prev_size = 0;

   while(curr){

      if(curr->free && (curr->size >= size) && (prev_size < curr->size)){
         worst_p = curr;
         prev_size = curr->size;
      }
      *last = curr;
      curr = curr->next;
   }
   curr = worst_p;

#endif

   /* Next fit */
   // Start incrementing from the last allocated block
   // If a block is found while running the list then
   // Make last point to block before free block.
   // Wrap around if we reached the end of the list.

#if defined NEXT && NEXT == 0
   int wraped = 0;

   if(last_allocated)
      curr = last_allocated;

   while(curr && !(curr->free && curr->size>= size)){
      
      if(!curr->next)
         *last = curr;

      curr = curr->next;

      if(!curr){ //wrap around
         if(!heapList)
            curr = NULL;
         else
            curr = heapList;

         wraped++;
         if(wraped == 2)
            return NULL;
      }
   }
   if (curr && curr->free){
      last_allocated = curr;
   }

#endif

   if (curr)
      num_reuses++;
   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

   /* Attach new _block to previous _block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update _block metadata:
      Set the size of the new block and initialize the new block to "free".
      Set its next pointer to NULL since it's now the tail of the linked list.
   */
   num_grows++;
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{ 
   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
      atexit(find_frag_fraction);
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block.  If a free block isn't found then we need to grow our heap. */
   num_requested += size;
   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   //Split block if it's bigger than desired size.
   if(next && next->size >= size+4){
      num_splits++;
      num_blocks++;
      size_t old_size = next->size;
      next->size = size;

      //start of new block
      struct _block *split_block = (struct _block*)((char*)BLOCK_DATA(next) + size);
      //size of new block
      split_block->size = ((char*)BLOCK_DATA(next) + old_size) - ((char*)BLOCK_DATA(split_block));
      //swapping pointers
      split_block->prev = next;
      split_block->next = next->next;
      split_block->free = true;
      next->next = split_block;
   }

   /* Could not find free _block, so grow heap */
   if (!next) 
   {
      next = growHeap(last, size);
      if (next)
         max_heap += size;
      
      num_blocks++;
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (!next) 
   {
      return NULL;
   }
   
   if (next->free) 
      max_heap += size;
   

   /* Mark _block as in use */
   next->free = false;
   last_allocated = next;

   /* Return data address associated with _block to the user */
   num_mallocs++;
   return BLOCK_DATA(next);
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (!ptr) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
   num_frees++;
   max_heap -= curr->size;

   /* Coalescing free _blocks.  If the next block or previous block 
      are free then combine them with this block being freed.
   */
   struct _block* prev = curr->prev;
   struct _block* next = curr->next;

   //both are free
   if(prev && next && prev->free && next->free){
      curr->next = next->next;
      curr->size += next->size + 24; //something missing from slides.
      prev->next = curr->next;
      prev->size += curr->size + 24;
      num_blocks -= 2;
      num_coalesces++;
   }
   //prev free
   else if(prev && prev->free){
      prev->next = curr->next;
      prev->size += curr->size + 24;
      num_blocks--;
      num_coalesces++;
   }
   //next free
   else if(next && next->free){
      curr->next = next->next;
      curr->size += next->size + 24;
      num_blocks--;
      num_coalesces++;
   } 
}

void *calloc( size_t nmemb, size_t size )
{
   return malloc(nmemb*size); //crazy funciton I know
}

void *realloc( void *ptr, size_t size )
{
   if(!ptr)
     return malloc(size); //we do things according to the C manual around here

   struct _block* header_ptr = BLOCK_HEADER(ptr);

   if(header_ptr->size == size) //do nothing
      return ptr;
   else if(header_ptr->size > size){ //shrink
      header_ptr->size = size;
      return ptr;
   }
   else{ //expand
      void* new_ptr = malloc(size);
      return memcpy(new_ptr,ptr,header_ptr->size);
   }
   
   return NULL;
}