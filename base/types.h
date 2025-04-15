#pragma once

#include "build_context.h"
//// Useful macros
#if defined(COMPILER_MSVC)
	#define typeof(X) __typeof(X)
#else
	#define typeof(X) __typeof__(X)
#endif

#if defined(COMPILER_MSVC)
	#define forceinline __forceinline
#else
	#define forceinline __attribute__((always_inline)) inline
#endif

#define static_assert(Pred, Msg) _Static_assert((Pred), (Msg))

#define Min(A, B) (((A) < (B)) ? (A) : (B))

#define Max(A, B) (((A) > (B)) ? (A) : (B))

#define Clamp(Lo, X, Hi) Min(Max(Lo, X), Hi)

//// Core types
#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdarg.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef ptrdiff_t isize;
typedef size_t    usize;
typedef uintptr_t uintptr;
typedef uint8_t   byte;
typedef int32_t   rune;

typedef float f32;
typedef double f64;

typedef struct {
	byte const* v;
	isize len;
} String;

// String literal
#define StrLit(S) ((String){.v = (byte const*)("" S ""), .len = (sizeof(S) - 1)})
// To be used with `%.*s`
#define StrFmt(S) (int)(S.len), (char const*)(S.v)


