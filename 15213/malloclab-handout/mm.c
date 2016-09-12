/*
 * mm.c
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#define DEBUG
#ifdef DEBUG
# define dbg_printf(...) printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif


/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* one asumption is heap is smaller then 2^32
 * so payload size should be smaller then 2^32
 * Header and footer size can be wsize
 */

/* single word (4) or double word (8) alignment */
#define WSIZE 4
#define DSIZE 8
#define ALIGNMENT 8
#define CHUNKSIZE 1 << 12
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7)

#define PACK(size, alloc) ((size) | (alloc))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define GET(p) (*(unsigned int *)(p))

#define GETSIZE(p) (GET(p) & ~0x7)
#define GETALLOC(p) (GET(p) & 0x1)

#define HDRP(p) ((char*)(p) - WSIZE)
#define FTRP(p) ((char*)(p) + GETSIZE(HDRP(p)) - DSIZE)

#define NEXT_BLKP(p) ((char*)(p) + GETSIZE(HDRP(p)))
#define PREV_BLKP(p) ((char*)(p) - GETSIZE((char*)(p)-DSIZE)) 


/*
 * Initialize: return -1 on error, 0 on success.
 */

void* heap_head;

static void *coalesce(void *bp) {
		//4 cases  aa af fa ff
		size_t size = GETSIZE(HDRP(bp));	
		void * prev = PREV_BLKP(bp);
		void * next = NEXT_BLKP(bp);

		//dbg_printf("bp=%d, prev = %d, next = %d\n", bp, prev, next);

		size_t prevA = GETALLOC(HDRP(prev));
		size_t nextA = GETALLOC(HDRP(next));

		//dbg_printf("preva = %d, nexta = %d\n", prevA, nextA);
		if(prevA==1 && nextA==1) {
				//case aa
				return bp;
		}

		if(prevA==0 && nextA == 1) {
				//case fa
				size_t newSize = GETSIZE(HDRP(prev))+size;
				PUT(HDRP(prev), PACK(newSize, 0));
				PUT(FTRP(prev), PACK(newSize, 0));
				bp = prev;
		}

		if(prevA==1 && nextA == 0) {
				//case af
				size_t newSize = GETSIZE(HDRP(prev))+size;
				PUT(HDRP(bp), PACK(newSize, 0));
				PUT(FTRP(bp), PACK(newSize, 0));
		}

		if(prevA==0 && nextA==0) {
				//case ff
				size_t newSize = GETSIZE(HDRP(prev))+GETSIZE(HDRP(next))+size;
				PUT(HDRP(prev), PACK(newSize, 0));
				PUT(FTRP(prev), PACK(newSize, 0));
				bp = prev;
		}

		return bp;
}

static void *expend_heap(size_t size) {
		size = ALIGN(size);
		void* bp;
		if( (bp = mem_sbrk(size)) == (void *)-1) {
			return NULL;
		}
		//put it as free blk
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
		PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

		dbg_printf("bp=%d, hdr=%d ftr=%d, nextblkhdr=%d, prevblk=%d\n",bp, HDRP(bp), FTRP(bp), HDRP(NEXT_BLKP(bp)), PREV_BLKP(bp));
		return coalesce(bp);
}

int mm_init(void) {
		dbg_printf("mm_init\n");
		if ((heap_head = mem_sbrk(4 * WSIZE))	== (void*)-1)
				return -1;
		// init pro epi
		PUT(heap_head, 0x0);
		PUT(heap_head + WSIZE, PACK(8, 1));
		PUT(heap_head + WSIZE*2, PACK(8, 1));
		PUT(heap_head + WSIZE*3, PACK(0, 1));
		heap_head += WSIZE * 2;

		if (expend_heap(CHUNKSIZE) == NULL)	
				return -1;
    return 0;
}

void place(void *bp, size_t size) {
		size_t block_size = GETSIZE(HDRP(bp));
		if(block_size < size){
				return;
		} else if (block_size - size <= DSIZE) {
				//no split
				PUT(HDRP(bp), PACK(block_size, 1));
		} else {
				//need split
				PUT(HDRP(bp), PACK(size, 1));
				PUT(FTRP(bp), PACK(size, 1));
				PUT(HDRP(NEXT_BLKP(bp)), PACK(block_size-size, 0));
				PUT(FTRP(NEXT_BLKP(bp)), PACK(block_size-size, 0));
		}
}

void* find_fit(size_t size) {
		size_t block_size;
		void* cur_p = NEXT_BLKP(heap_head);
		block_size = GETSIZE(HDRP(cur_p));
		printf("find_fit blocksize=%d", block_size);
		while(block_size > 0) {
				unsigned int alloc = GETALLOC(HDRP(cur_p));
				if(block_size >= size && alloc == 0) {
						return cur_p;
				}		

				cur_p = NEXT_BLKP(cur_p);
				block_size = GETSIZE(HDRP(cur_p));
				printf("find_fit blocksize=%d, cur_p=%d", block_size, cur_p);
		}
		return NULL;
}

/*
 * malloc
 */
void *malloc (size_t size) {
		//calculate the actual size
		size_t asize;
		if(size == 0) {
				return NULL;
		}

		//find best fit position
		//place block
		char *bp;
		asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);
		printf("size and asize = %d, %d\n", size, asize);	
		if( (bp=find_fit(asize)) != NULL) {
				place(bp, asize);
				return bp;
		}
		//if not found extend heap
		//place block
		printf("extend heap");
		size_t extendSize = asize > CHUNKSIZE ? asize : CHUNKSIZE;
		bp = expend_heap(extendSize);
		if (bp == NULL) {
				return NULL;
		}
		printf("bp = %d\n", bp);
		place(bp, asize);
		return bp;
}

/*
 * free
 */
void free (void *ptr) {
    if(!ptr) return;
		size_t size = GETSIZE(HDRP(ptr));
		PUT(HDRP(ptr), PACK(size, 0));
		PUT(FTRP(ptr), PACK(size, 0));
		coalesce(ptr);
}

/*
 * realloc - you may want to look at mm-naive.c
 */
void *realloc(void *oldptr, size_t size) {
    return NULL;
}

/*
 * calloc - you may want to look at mm-naive.c
 * This function is not tested by mdriver, but it is
 * needed to run the traces.
 */
void *calloc (size_t nmemb, size_t size) {
    return NULL;
}


/*
 * Return whether the pointer is in the heap.
 * May be useful for debugging.
 */
static int in_heap(const void *p) {
    return p <= mem_heap_hi() && p >= mem_heap_lo();
}

/*
 * Return whether the pointer is aligned.
 * May be useful for debugging.
 */
static int aligned(const void *p) {
    return (size_t)ALIGN(p) == (size_t)p;
}

/*
 * mm_checkheap
 */
void mm_checkheap(int verbose) {
	//check from heap head
	//dbg_printf("start checkheap\n");
	void * cur_p = heap_head;
	
	while(1) {
		unsigned int size = GETSIZE(HDRP(cur_p));
		unsigned int isAlloc = GETALLOC(HDRP(cur_p));
		//dbg_printf("size = %d, alloc=%d\n", size, isAlloc);
		if (size == 0)
		{
			 break;						
		}
		cur_p = (void *)((char *)cur_p + size);
	}
}
