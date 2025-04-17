#pragma once
#include "types.h"
#include "memory.h"

#if defined(__has_attribute)
	#if __has_attribute(format)
		#define str_attribute_format(fmt,va) __attribute__((format(printf,fmt,va)))
	#endif
#else
	#define str_attribute_format(fmt,va)
#endif

typedef struct {
	byte bytes[4];
	i32  len;
} UTF8Encoded;

typedef struct {
	rune codepoint;
	i32  len;
} UTF8Decoded;

static inline
bool utf8_is_continuation_byte(rune c){
	static const rune CONTINUATION1 = 0x80;
	static const rune CONTINUATION2 = 0xbf;
	return (c >= CONTINUATION1) && (c <= CONTINUATION2);
}

#define UTF8_ERROR ((rune)0xfffd)

#define UTF8_ERROR_ENCODED ((UTF8Encoded){ .bytes = {0xef, 0xbf, 0xbd}, .len = 0 })

i32 utf8_rune_size(rune c);

UTF8Encoded utf8_encode(rune r);

UTF8Decoded utf8_decode(byte const* buf, isize n);

String str_format(Arena* arena, char const * restrict fmt, ...) str_attribute_format(2, 3);

String str_vformat(Arena* arena, char const * restrict fmt, va_list argp);

String str_sub(String s, isize start, isize end);

isize str_compare(String left, String right);

bool str_parse_i64(String s, u32 base, i64* out);

bool str_parse_f64(String s, f64* out);

bool str_equals(String left, String right);

bool str_starts_with(String s, String prefix);

bool str_ends_with(String s, String postfix);

bool rune_is_digit(rune r, int base);
