#include "dbll.h"
#include <stdlib.h>
#include <stdio.h>
#include "poolalloc.h"

/*
   a pool-based allocator that uses doubly-linked lists to track
   allocated and free blocks
 */

/* create and initialize a memory pool of the required size */
/* use malloc() or calloc() to obtain this initial pool of memory from the system */
/* observe: 
    1. alloc_list points to an empty doubly-linked list (dbll).
    2. free_list points to a dbll containing a single llnode.
    3. The user_data of this llnode points to an alloc_info object that contains offset 0 and size equal to the pool size (X).
       Essentially this indicates that a free block of at most size X is available for allocation. */
struct memory_pool *mpool_create(size_t size)
{
  struct memory_pool *mp = (struct memory_pool*)malloc(sizeof(struct memory_pool));

  if (mp != NULL) {
    /* set start to memory obtained from malloc */
    mp->start = mp;
    /* set size to size */
    mp->size = size;

    /* create a doubly-linked list to track allocations */
    mp->alloc_list = dbll_create();
    /* create a doubly-linked list to track free blocks */
    mp->free_list = dbll_create();

    /* create a free block of memory for the entire pool and place it on the free_list */
    struct alloc_info *ai = (struct alloc_info*)malloc(sizeof(struct alloc_info)); //user_data to be stored in ll_free node
    ai->offset = 0;
    ai->size = size;
    dbll_append(mp->free_list, ai); //create node, add to list

    /* return memory pool object */
    //printf("memory pool mem allo worked");
    return mp;
  }
  //printf("memory pool mem allo didn't work");
  return NULL;
}

/* ``destroy'' the memory pool by freeing it and all associated data structures */
/* this includes the alloc_list and the free_list as well */
void mpool_destroy(struct memory_pool *p)
{
  /* make sure the allocated list is empty (i.e. everything has been freed) */
  /* free the alloc_list dbll */
  dbll_free(p->alloc_list);
  /* free the free_list dbll  */
  dbll_free(p->free_list);
  /* free the memory pool structure */
  free(p);
}

/* allocate a chunk of memory out of the free pool */

/* Return NULL if there is not enough memory in the free pool */

/* The address you return must be aligned to 1 (for size=1), 2 (for
   size=2), 4 (for size=3,4), 8 (for size=5,6,7,8). For all other
   sizes, align to 16.
*/
void *mpool_alloc(struct memory_pool *p, size_t size)
{
  /* check if there is enough memory for allocation of `size` (taking
	 alignment into account) by checking the list of free blocks */
  if (p->size < size) {
    return NULL;
  }

  /* search the free list for a suitable block */
  /* there are many strategies you can use: first fit (the first block that fits),
	 best fit (the smallest block that fits), etc. */
  struct llnode *node = p->free_list->first; //node to search free_list ll
  struct alloc_info *ai = node->user_data; //ai object to access size in each node
  int free = 0;

  while (node != NULL) {
    if (ai->size >= size) {
      free = 1;
      break;
    }
    node = node->next;
    //ai = node->user_data;
  }

  /* if no suitable block can be found, return NULL */
  if (free == 0) {
    return NULL;
  }

  /* if found, create an alloc_info node, store start of new region
	 into offset, set size to allocation size (take alignment into
	 account!), set free to null */
  struct alloc_info *alloc_ai = (struct alloc_info*)malloc(sizeof(struct alloc_info));
  //node = p->free_list->last;

  
  alloc_ai->size = alloc_ai->request_size = size;
  alloc_ai->offset = ai->size;
  ai->size -= size;
  ai->offset = size;

  /* add the new alloc_info node to the memory pool's allocated list */
  dbll_append(p->alloc_list, alloc_ai);

  /* return pointer to allocated region*/
  return p->start + ai->offset;
}

/* Free a chunk of memory out of the pool */
/* This moves the chunk of memory to the free list. */
/* You may want to coalesce free blocks [i.e. combine two free blocks
   that are are next to each other in the pool into one larger free
   block. Note this requires that you keep the list of free blocks in order */
void mpool_free(struct memory_pool *p, void *addr)
{
  /* search the alloc_list for the block */
  /* move it to the free_list */
  /* coalesce the free_list */
}
