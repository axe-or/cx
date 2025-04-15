#include "memory.h"

#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
void mem_copy(void* dest, void const* source, isize count){
	__builtin_memmove(dest, source, count);
}

void mem_copy_no_overlap(void* dest, void const* source, isize count){
	__builtin_memcpy(dest, source, count);
}

void mem_set(void* dest, byte val, isize count){
	__builtin_memset(dest, val, count);
}

int mem_compare(void const* left, void const* right, isize count){
	return __builtin_memcmp(left, right, count);
}
#else
#include <string.h>
void mem_copy(void* dest, void const* source, isize count){
	memmove(dest, source, count);
}

void mem_copy_no_overlap(void* dest, void const* source, isize count){
	memcpy(dest, source, count);
}

void mem_set(void* dest, byte val, isize count){
	memset(dest, val, count);
}

int mem_compare(void const* left, void const* right, isize count){
	return memcmp(left, right, count);
}
#endif

