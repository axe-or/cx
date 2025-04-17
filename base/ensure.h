#pragma once

#include "types.h"

extern int printf(char const*, ...);

_Noreturn static inline
void trap(){
#if defined(COMPILER_MSVC)
	__debugbreak();
#else
	__builtin_trap();
#endif
}

static inline
void ensure_ex(bool predicate, char const* msg, char const* file, int line){
	if(!predicate){
		printf("(%s:%d) Assertion failed: %s\n", file, line, msg);
		trap();
	}
}

static inline _Noreturn
void panic_ex(char const* msg, char const* file, int line){
	printf("(%s:%d) Panic: %s\n", file, line, msg);
	trap();
}

static inline _Noreturn
void unimplemented_ex(char const* msg, char const* file, int line){
	printf("(%s:%d) Unimplemented: %s\n", file, line, msg);
	trap();
}

#define ensure(Pred, Msg)  ensure_ex((Pred), (Msg), __FILE__, __LINE__)
#define panic(Msg)         panic_ex((Msg), __FILE__, __LINE__)
#define unimplemented(Msg) unimplemented_ex((Msg), __FILE__, __LINE__)


