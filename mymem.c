#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>


/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

struct memoryList
{
  // doubly-linked list
  struct memoryList *last;
  struct memoryList *next;

  int size;            // How many bytes in this block?
  char alloc;          // 1 if this block is allocated,
                       // 0 if this block is free.
  void *ptr;           // location of block in memory pool.
};

strategies myStrategy = NotSet;    // Current strategy


size_t mySize;
void *myMemory = NULL;

static struct memoryList *head;
static struct memoryList *next;


/* initmem must be called prior to mymalloc and myfree.

   initmem may be called more than once in a given exeptr;           // location of block in memory pool.
};uction;
   when this occurs, all memory you previously malloc'ed  *must* be freed,
   including any existing bookkeeping data.

   strategy must be one of the following:
		- "best" (best-fit)
		- "worst" (worst-fit)
		- "first" (first-fit)
		- "next" (next-fit)
   sz specifies the number of bytes that will be available, in total, for all mymalloc requests.
*/

void initmem(strategies strategy, size_t sz)
{
	myStrategy = strategy;

	/* all implementations will need an actual block of memory to use */
	mySize = sz;

	if (myMemory != NULL) free(myMemory); /* in case this is not the first time initmem2 is called */

	/* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */
	struct memoryList *trav = head;

	if (head!=NULL)
	{
		if (trav->next == trav->last) // in case there's only 1 node in the list
			free(trav);
		else // more than 1 nodes: traverse through the list to deallocate memory blocks
		{
			do {
				trav=trav->next;
				free(trav->last);
			} while (trav->next != head);
			free(trav);
		} 
	}

	myMemory = malloc(sz);
	
	/* TODO: Initialize memory management structure. */
	// the structure is a doubly-linked list
	// at first the memory is not separated into blocks so it is the only node
	struct memoryList* mem = malloc(sizeof(struct memoryList));
	head=mem;
	next=mem;
	mem->ptr=myMemory;
	mem->size=mySize;
	mem->alloc=0;
	mem->next=mem;
	mem->last=mem;
}

/* Allocate a block of memory with the requested size.
 *  If the requested block is not available, mymalloc returns NULL.
 *  Otherwise, it returns a pointer to the newly allocated block.
 *  Restriction: requested >= 1 
 */
void *mymalloc(size_t requested)
{
	assert((int)myStrategy > 0);
	size_t req = requested;
	struct memoryList* trav = head;
	struct memoryList* block = NULL;
	struct memoryList* min = NULL;
	struct memoryList* max = NULL;

	switch (myStrategy)
	{
	  	case NotSet: 
	        return NULL;
	  	case First:
		  	do {
	  			if (trav->size >= req && trav->alloc == 0)
	  			{
	  				block=trav;
	  				break;
	  			}
	  			trav=trav->next;
	  		} while (trav!=head);
	  		break;
	  	case Best:
	  		do {
	  			if (trav->size >= req && trav->alloc == 0)
	  			{
					if (min == NULL)
						min=trav;
					else if (min!=NULL && trav->size<min->size)
	  					min=trav;
					else {} // do nothing	
	  			}
	  			trav=trav->next;
	  		} while (trav!=head);
			block = min;
	  		break;
	  	case Worst: // traverse to find the largest memory block that fits requested size
	  		do {
	  			if (trav->size >= req && trav->alloc == 0)
	  			{
	  				if (max == NULL)
						max=trav;
					else if (max!=NULL && trav->size>max->size)
	  					max=trav;
					else {} // do nothing	
	  			}
	  			trav=trav->next;
	  		} while (trav!=head);
			block = max;
	  		break;
	  	case Next:
		  	trav = next; // start from the point after last allocation 
	  		do {
	  			if (trav->size >= req && trav->alloc == 0)
	  			{
	  				block=trav;
					break;
	  			}
	  			trav=trav->next;
	  		} while (trav!=next); 
	  		break;
	}

	if (block == NULL) // if no suitable block is found
		return NULL;

	// in case a block is found
	if(block->ptr!=NULL)
	{
		if (block->size > req) 
		{
			block->alloc=1;

			struct memoryList* newMem = malloc(sizeof(struct memoryList));
			newMem->last=block;
			newMem->next=block->next;
			newMem->next->last= newMem;
			
			newMem->alloc=0;
			newMem->size=block->size-req;
			newMem->ptr=block->ptr+req;

			// next starting point for next-fit strategy
			next=newMem;
			block->size=req;
			block->next=newMem;
		} else {	// if block size == req -> no splitting up the block 
			block->alloc=1;
			next = block->next; // next is simply the block after the current block
		}
	}	
	return block->ptr;
}


/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void* block)
{
	// free block -> traverse to find the block and then mark it as deallocated
	struct memoryList* freeBlock = head;
	do {
		if (freeBlock->ptr == block) {
			freeBlock->alloc = 0;
			break;
		}
		freeBlock = freeBlock->next;
	} while (freeBlock != head);

	// if last is free -> merge current block with last block next to it that is unallocated
	if (freeBlock != head && freeBlock->last->alloc == 0) // freeBlock != head since we can't merge the head with the tail of the list
	{
		freeBlock->last->size += freeBlock->size;
		freeBlock->next->last = freeBlock->last;
		freeBlock->last->next = freeBlock->next;
		if (next==freeBlock) // since we merge 2 block together, if the one to be deleted is next, change next
			next = freeBlock->last;
		struct memoryList* temp1 = freeBlock->last;
		free(freeBlock);
		freeBlock = temp1;
	}

	// if next is free -> merge next block next to current block that is unallocated with the current block
	if (freeBlock->next != head && freeBlock->next->alloc == 0) // freeBlock->next != head since we can't merge the tail with the head of the list
	{
		freeBlock->size += freeBlock->next->size;
		freeBlock->next->next->last = freeBlock;
		if(next == freeBlock->next) // since we merge 2 block together, if the one to be deleted is next, change next
			next = freeBlock;
		struct memoryList* temp2 = freeBlock->next->next;
		free(freeBlock->next);
		freeBlock->next = temp2;
  	}
}


/****** Memory status/property functions ******
 * Implement these functions.
 * Note that when we refer to "memory" here, we mean the 
 * memory pool this module manages via initmem/mymalloc/myfree. 
 */

/* Get the number of contiguous areas of free space in memory. */
int mem_holes()
{
	int count=0;
	struct memoryList* trav=head;
	do
	{
		if (trav->alloc==0)
			count++;
		trav=trav->next;
	} while (trav!=head);
	return count;
}

/* Get the number of bytes allocated */
int mem_allocated()
{	
	int count=0;
	struct memoryList* trav=head;
	do
	{
		if (trav->alloc==1)
			count+=trav->size;
		trav=trav->next;
	} while (trav!=head);
	return count;
}

/* Number of non-allocated bytes */
int mem_free()
{
	int count=0;
	struct memoryList* trav=head;
	do
	{
		if (trav->alloc==0)
			count+=trav->size;
		trav=trav->next;
	} while (trav!=head);
	return count;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free()
{	
	int max=0;
	struct memoryList* trav=head;
	do
	{
		if (trav->alloc==0 && trav->size > max)
			max=trav->size;
		trav=trav->next;
	} while (trav!=head);
	return max;

}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size)
{
	int count=0;
	struct memoryList* trav=head;
	do
	{
		if (trav->alloc==0 && trav->size <= size)
			count++;
		trav=trav->next;
	} while (trav!=head);
	return count;
}       

// Return 0 if mem is NOT allocated, else, return 1
char mem_is_alloc(void *ptr)
{
	struct memoryList* trav=head;
	do
	{
		if (trav->ptr == ptr)
			return trav->alloc;
		trav=trav->next;
	} while (trav!=head);
	return 0;
}

/* 
 * Feel free to use these functions, but do not modify them.  
 * The test code uses them, but you may ind them useful.
 */


//Returns a pointer to the memory pool.
void *mem_pool()
{
	return myMemory;
}

// Returns the total number of bytes in the memory pool. */
int mem_total()
{
	return mySize;
}


// Get string name for a strategy. 
char *strategy_name(strategies strategy)
{
	switch (strategy)
	{
		case Best:
			return "best";
		case Worst:
			return "worst";
		case First:
			return "first";
		case Next:
			return "next";
		default:
			return "unknown";
	}
}

// Get strategy from name.
strategies strategyFromString(char * strategy)
{
	if (!strcmp(strategy,"best"))
	{
		return Best;
	}
	else if (!strcmp(strategy,"worst"))
	{
		return Worst;
	}
	else if (!strcmp(strategy,"first"))
	{
		return First;
	}
	else if (!strcmp(strategy,"next"))
	{
		return Next;
	}
	else
	{
		return 0;
	}
}


/* 
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */

/* Use this function to print out the current contents of memory. */
void print_memory()
{
	return;
}

/* Use this function to track memory allocation performance.  
 * This function does not depend on your implementation, 
 * but on the functions you wrote above.
 */ 
void print_memory_status()
{
	printf("mem_allocated: %d\n", mem_allocated());
	printf("mem_total: %d\n", mem_total());
	printf("mem_free: %d\n", mem_free());
	printf("mem_holes: %d\n", mem_holes());
	printf("mem_largest_free: %d\n", mem_largest_free());
	printf("average hole size: %f\n",((float)mem_free())/mem_holes());
}

/* Use this function to see what happens when your malloc and free
 * implementations are called.  Run "mem -try <args>" to call this function.
 * We have given you a simple example to start.
 */
void try_mymem(int argc, char **argv) {
    strategies strat;
	void *a, *b, *c, *d, *e;
	if(argc > 1)
	  strat = strategyFromString(argv[1]);
	else
	  strat = First;

	
	/* A simple example.  
	   Each algorithm should produce a different layout. */

	initmem(strat,500);

	a = mymalloc(100);				
	b = mymalloc(100);
	c = mymalloc(100);
	myfree(b);
	d = mymalloc(50);
	myfree(a);
	e = mymalloc(25);
	
	print_memory();
	print_memory_status();
	
}
