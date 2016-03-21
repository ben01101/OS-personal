/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines.
 */

#include "mm_alloc.h"
#include <stdlib.h>

#include <unistd.h>

static struct list metaList;
static struct metadata start;
static struct metadata end;

void list_init(struct list *l) {
	// struct metadata start;
	// struct metadata end;
	start.size = 0;
	start.is_free = 0;
	start.next = &end;
	start.prev = NULL;

	end.size = 0;
	end.is_free = 0;
	end.next = NULL;
	end.prev = &start;

	l->head = &start;
	l->tail = &end;
}

void *mm_malloc(size_t size) {
    /* YOUR CODE HERE */
    if (size == 0) return NULL;
    if (metaList.head == NULL) list_init(&metaList);

    int metasize = sizeof(struct metadata);

    struct metadata *block_to_use = metaList.head->next;
    while (block_to_use != metaList.tail) {
    	if (block_to_use->is_free) {
    		int s = block_to_use->size;
    		if (s >= size + metasize) {
    			// Split block in two.
    			return split_data_block(block_to_use, size) + 1;
    		} else {
    			if (s >= size) {
    				// Perhaps reuse this block.
    				block_to_use->size = size;
    				block_to_use->is_free = 0;
    				return block_to_use + 1;
    				// if (s == size) {
    				// 	// Definitely use this block.
    				// }
    			}
    		} 
    	}
    }
    struct metadata *this_block = (struct metadata *)
    	sbrk (sizeof (struct metadata) + size);
    this_block->size = size;
    this_block->is_free = 0;
    this_block->prev = metaList.tail->prev;
    this_block->next = metaList.tail;
    metaList.tail->prev->next = this_block;
    metaList.tail->prev = this_block;

    return this_block + 1;
}

/* Splits a data block in two. CURRENT has a reduced size
	and becomes the second element in the list, with the
	first element being the newly created block that is returned. */
struct metadata *split_data_block(struct metadata *current, int size) {
	int metasize = sizeof(struct metadata);
	// assert(current->is_free);
	// assert(current->size >= size + metasize);

	struct metadata *new;
	new = current;
	struct metadata *old;
	old = (struct metadata *) ((void *) (current) + metasize + size);

	old->size = current->size - (size + metasize);
	old->is_free = 1;
	old->next = current->next;
	old->prev = new;

	new->size = size;
	new->is_free = 0;
	new->next = old;
	new->prev = current->prev;

	current->next->prev = old;
	current->prev->next = new;
	mm_clear(new);
	return new;
}

/* To zero-fill the data in a memory block. */
void mm_clear(struct metadata *ptr) {
	if (ptr->size == 0) return;
	int *mem = (int *) (ptr + 1);
	int i;
	*mem = 0;
	for (i = 1; i < ptr->size; i++) {
		mem++;
		*mem = 0;
	}

}
void *mm_realloc(void *ptr, size_t size) {
    /* YOUR CODE HERE */
    return NULL;
}

void mm_free(void *ptr) {
    /* YOUR CODE HERE */
}
