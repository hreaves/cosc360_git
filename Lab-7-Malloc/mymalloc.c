/*  COSC360 Lab7 Malloc
	Harrison Reaves
	
*/
#include <stdio.h>
#include <stdlib.h>

void *head;

void *my_malloc(size_t size) {
	size_t tmp_size;
	void *tmp_addr;
	
	tmp_addr = free_list_begin();
	tmp_size = *(int *)(tmp_addr-8);
	while(tmp_size < size) {
		tmp_addr = free_list_next(tmp_addr);
		tmp_size = *(int *)(tmp_addr-8);
	}
	if(tmp_size > size) {return tmp_addr;}
	else {
		if(size > 8184) {sbrk(8182);}
		else{sbrk(size);}
	}
}

void my_free(void *ptr) {
	
}

void *free_list_begin() {

}

void *free_list_next(void *node) {

}

void coalesce_free_list() {

}

int main() {
	size_t size;
	void *ptr;

	size = (size + 7 + 8) & -8;
	// if my_malloc called
	my_malloc(size);
	// if my_free called
	my_free(ptr);
	return 0;
}
