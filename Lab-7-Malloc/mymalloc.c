/*  COSC360 Lab7 Malloc
	Harrison Reaves
	This program acts as a memory allocator, working just like malloc and free,
	interfacing directly with srbk(). AI was used for debugging.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mymalloc.h"

typedef struct chunk {
    int size;
    struct chunk *next;
    struct chunk *prev;
} Chunk;

Chunk *head; // start of free list

// allocate memory to user
void *my_malloc(size_t i_size) {
	size_t size, tmp_size;
	Chunk *tmp_addr, *last_addr, *new_addr;
	// properly aligned size given padding
	size = (i_size + 7 + 8) & -8;
	tmp_addr = free_list_begin();
	// iterate through whole free list
	while(tmp_addr != NULL) {
		tmp_size = *(int *)(tmp_addr);
		/*  if there's a chunk bigger than what's required, carve off necessary number of bytes to
			create a new chunk and return it, change size of carved block
		*/
		if(tmp_size > size) {
			tmp_addr->size = tmp_size - size;
			new_addr = (Chunk *)((char *)tmp_addr + tmp_addr->size);
			new_addr->size = size;
			return (void *)((char *)new_addr + 8);;
		}
		/* If the whole chunk is needed, remove and return the block. Adjust the pointers of the
		 * previous and next chunks as needed, depending on if there are previous and next chunks
		*/
		if(tmp_size == size) {
			// new head
			if(tmp_addr == free_list_begin()) {
				if(tmp_addr->next != NULL) {
					head = tmp_addr->next;
					tmp_addr->next->prev = NULL;
				}
				else{head = NULL;}
			}
			else {
				// middle
				if(tmp_addr->next != NULL) {
					tmp_addr->prev->next = tmp_addr->next;
					tmp_addr->next->prev = tmp_addr->prev;
				}
				// new tail
				else{tmp_addr->prev->next = NULL;}
			}
			return (void *)((char *)tmp_addr + 8);
		}
		// iterate
		last_addr = tmp_addr;
		tmp_addr = free_list_next(tmp_addr);
	}
	/*  If there's not a chunk in the free list big enough, call sbrk to create more memory
		depending on the inputed size, and call my_free() to add it to the free list
	*/
	if(size < 8184) {
		new_addr = sbrk(8192);
		new_addr->size = 8192;
		my_free((char *)new_addr + 8);
	}
	else {
		Chunk *new_addr = sbrk(size);
		new_addr->size = size;
		my_free((char *)new_addr + 8);
	}
	// With added memory on the free list, go back through the free list and allocate a chunk
	return my_malloc(i_size);
}
// put memory back into the free list
void my_free(void *ptr) {
	Chunk *addr = (Chunk *)((char *)ptr - 8);
	Chunk *tmp_addr = free_list_begin();
	Chunk *last_addr = NULL;
	// empty list, new head
	if(tmp_addr == NULL) {
		head = addr;
		addr->next = NULL;
		addr->prev = NULL;
		return;
	}
	// iterate through whole free list to find where the chunk belongs based on address
	while(tmp_addr != NULL) {
		// if the address is ever less than the current chunk, adjust pointers to put freed chunk right before it
		if(addr < tmp_addr) {
			if(last_addr == NULL) {head = addr;}
			else {last_addr->next = addr;}
			addr->prev = last_addr;
			addr->next = tmp_addr;
			tmp_addr->prev = addr;
			return;
		}
		// iterate
		last_addr = tmp_addr;
        tmp_addr = free_list_next(tmp_addr);
		// if we get to the end of the list without inserting, add to the end, new tail
		if(tmp_addr == NULL) {
			last_addr->next = addr;
			addr->prev = last_addr;
			addr->next = NULL;
			return;
		}
	}
}
// start of free list
void *free_list_begin() {
	return head;
}
// next chunk in the free list
void *free_list_next(void *node) {
	if((Chunk *)node == NULL) {return NULL;}
	return ((Chunk *)node)->next;
}
// combine adjacent chunks in the free list
void coalesce_free_list() {
	Chunk *tmp_addr = free_list_begin();
	Chunk *next_addr;
	int size;
	/*  iterate through the free list, if the current chunk address plus the size equals the address of
		the next chunk, that means they are right nex to eachother, so adjust points and combine size to
		essentially delete the adjacent chunk
	*/
	while(tmp_addr != NULL) {
		size = tmp_addr->size;
		next_addr = free_list_next(tmp_addr);
		if((Chunk *)((char *)tmp_addr + size) == next_addr) {
			tmp_addr->size = size + next_addr->size;
			tmp_addr->next = next_addr->next;
			if(next_addr->next != NULL) {next_addr->next->prev = tmp_addr;}
		}
		/*  Iterate only if there's no merge. If there's a merge, you want to recheck the same chunk with its new 
			size to see if its also adjacent to the next chunk, like if there were three or more separate chunks
			all needing to merge.
		*/
		else {tmp_addr = next_addr;}
	}
}

