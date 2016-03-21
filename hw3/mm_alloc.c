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
static int reallocating = 0;

/* Initialize the metaList. */
void list_init(struct list *l) {
	// printf("initializing list.\n");
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
    // printf("metasize: %d\n", metasize);
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
    				// printf("Reusing block.\n");
    				block_to_use->size = size;
    				block_to_use->is_free = 0;
    				if (!reallocating) mm_clear(block_to_use);
    				return block_to_use + 1;
    				// if (s == size) {
    				// 	// Definitely use this block.
    				// }
    			}
    		} 
    	}
    	block_to_use = block_to_use->next;
    }

	struct metadata *this_block = sbrk (metasize + size);
    // printf("Adding block.\n");
    if (this_block == (void *) -1) return NULL;
    this_block->size = size;
    this_block->is_free = 0;
    this_block->prev = metaList.tail->prev;
    this_block->next = metaList.tail;
    metaList.tail->prev->next = this_block;
    metaList.tail->prev = this_block;

    if (!reallocating) mm_clear(this_block);
    return this_block + 1;
}

/* Splits a data block in two. CURRENT has a reduced size
	and becomes the second element in the list, with the
	first element being the newly created block that is returned. */
struct metadata *split_data_block(struct metadata *current, int size) {
	int metasize = sizeof(struct metadata);
	// assert(current->is_free);
	// assert(current->size >= size + metasize);

	printf("splitting\n");
	struct metadata *new;
	struct metadata *old;
	old = (void *) (current) + metasize + size;
	new = current;
	current->next->prev = old;
	current->prev->next = new;

	old->size = current->size - (size + metasize);
	old->is_free = 1;
	old->next = current->next;
	old->prev = new;

	new->size = size;
	new->is_free = 0;
	new->next = old;
	new->prev = current->prev;

	if (!reallocating) mm_clear(new);
	return new;
}

/* To zero-fill the data in a memory block. */
void mm_clear(struct metadata *ptr) {
	if (ptr->size == 0) return;
	char *mem = (void *) (ptr) + sizeof(struct metadata);
	int i;
	for (i = 0; i < ptr->size; i = i + 1) {
		mem[i] = '\0';
	}
	// printf("Cleared %d bytes.\n", i);
}

/* Prints info about the contents of the metaList. */
void print_list() {
	struct metadata *m = metaList.tail->prev;
	printf("MetaList:\n");
	while (m != metaList.head) {
		if (m->is_free) {
			printf("  FREE  ");
		} else {
			printf("  USED  ");
		}
		printf("Size: %d   *", m->size);
		uint *p = (void *) m + sizeof(struct metadata) + m->size - 4;
		int i;
		for (i = 0; i < m->size/4; i++) {
			printf(p);
			p--;
		}
		printf("*\n");
		m = m->prev;
	}
}

void *mm_realloc(void *ptr, size_t size) {
    /* YOUR CODE HERE */
    if (ptr == NULL) return mm_malloc(size);
    int memsize = sizeof(struct metadata);
    struct metadata *a = ptr - memsize;
    mm_free(ptr);
    if (size == 0) return NULL;
	reallocating = 1;
    void *newptr = mm_malloc(size);
    if (newptr == NULL) {
    	struct metadata *tmp = a;
    	ptr = mm_malloc(a->size);
    	reallocating = 0;
    	memcopy(((void *) tmp) + memsize, ptr);
    	// ptr = tmpptr;
    	return NULL;
    } else {
    	reallocating = 0;
    	memcopy(ptr, newptr);
    	return newptr;
    }
}

/* Copies the contents of block A into block B,
	and zeros out the remaining space in block B. */
void memcopy(void *a, void *b) {
	struct metadata *aa = a - sizeof(struct metadata);
	struct metadata *bb = b - sizeof(struct metadata);
	if (bb->size == 0) return;
	char *p_a = a;
	char *p_b = b;
	int i;
	for (i = 0; i < aa->size; i = i + 1) {
		if (i >= bb->size) break;
		p_b[i] = p_a[i];
	}
	for (; i < bb->size; i = i + 1) {
		p_b[i] = '\0';
	}
}

void mm_free(void *ptr) {
    /* YOUR CODE HERE */
    if (ptr == NULL) return;
    int metasize = sizeof(struct metadata);
    struct metadata *m = ptr - metasize;
    m->is_free = 1;
    if (m->prev->is_free) {
    	m->prev->prev->next = m;
    	m->size += metasize + m->prev->size;
    	m->prev = m->prev->prev;
    }
    if (m->next->is_free) {
    	m->next->next->prev = m;
    	m->size += metasize + m->next->size;
    	m->next = m->next->next;
    }
}
