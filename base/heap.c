#include "memory.h"
#include <stdlib.h>

void* heap_alloc(isize size, isize align){
	ensure(mem_valid_alignment(align), "Invalid alignment");
	align = max(align, (isize)alignof(void*));

	isize space = align - 1 + sizeof(void*) + size;
	void* allocated_mem = calloc(space, 1);

	ensure(allocated_mem != NULL, "Heap allocation failed");
	void* aligned_mem = (void*)((uintptr)allocated_mem + sizeof(void*));

	/* Align the pointer by rounding down */ {
		uintptr aligned_ptr = 0;
		uintptr p = (uintptr)aligned_mem;
		uintptr a = (uintptr)align;
		aligned_ptr = (p + a - 1) & ~(a - 1);

		aligned_mem = (void*)aligned_ptr;
	};

	/* Store actual memory address before the aligned one */ {
		void** ptr_loc = ((void**)aligned_mem) - 1;
		*ptr_loc = allocated_mem;
	}

	return aligned_mem;
}

void heap_free(void* ptr){
	void** pointer_array = (void**)ptr;
	void* actual_memory = pointer_array[-1];
	free(actual_memory);
}

