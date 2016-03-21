/*
 * mm_alloc.h
 *
 * A clone of the interface documented in "man 3 malloc".
 */

#pragma once

#include <stdlib.h>

struct metadata {
	int size;
	int is_free;
	struct metadata *next;
	struct metadata *prev;
};

struct list {
	struct metadata *head;
	struct metadata *tail;
};

void list_init(struct list *l);
struct metadata *split_data_block(struct metadata *current, int size);
void mm_clear(struct metadata *ptr);
void print_list();
void memcopy(void *a, void *b);

void *mm_malloc(size_t size);
void *mm_realloc(void *ptr, size_t size);
void mm_free(void *ptr);
