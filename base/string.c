#include "string.h"
#include <stdlib.h>

bool rune_is_digit(rune r, int base){
	switch(base){
	case 2:  return r >= '0' && r <= '1';
	case 8:  return r >= '0' && r <= '7';
	case 10: return r >= '0' && r <= '9';
	case 16: return (r >= '0' && r <= '9') || (r >= 'a' && r <= 'f') || (r >= 'A' && r <= 'F');
	}
	return false;
}

isize str_compare(String left, String right){
	if(left.len > right.len){
		return left.len;
	}
	if(left.len < right.len){
		return -right.len;
	}

	isize cmp = mem_compare(left.v, right.v, min(left.len, right.len));
	return cmp;
}

bool str_equals(String left, String right){
	if(left.len != right.len){ return false; }
	isize cmp = mem_compare(left.v, right.v, left.len);
	return cmp == 0;
}

bool str_starts_with(String s, String prefix){
	if(prefix.len > s.len){ return false; }
	if(prefix.len == 0){ return true; }
	return mem_compare(s.v, prefix.v, prefix.len) == 0;
}

bool str_ends_with(String s, String postfix){
	if(postfix.len > s.len){ return false; }
	if(postfix.len == 0){ return true; }
	return mem_compare(s.v + (s.len - postfix.len), postfix.v, postfix.len) == 0;
}

String str_sub(String s, isize start, isize end){
	ensure(start <= s.len && end <= s.len && end >= start, "Improper range");
	return (String){ .v = s.v + start, .len = end - start };
}

static inline
i64 str_ipow10(int exponent){
	ensure(exponent >= 0, "Invalid exponent");
	i64 n = 1;
	for(int i = 0; i < exponent; i++){
		n *= 10;
	}
	return n;
}

static inline
int str_digit_value(char c, int base){
	int val = -1;
	switch(base){
	case 2: {
		if(c == '0' || c == '1'){
			val = c - '0';
		}
	} break;
	case 8: {
		if(c >= '0' && c <= '7'){
			val = c - '0';
		}
	} break;
	case 10: {
		if(c >= '0' && c <= '9'){
			val = c - '0';
		}
	} break;
	case 16: {
		if(c >= '0' && c <= '9'){
			val = c - '0';
		} else if(c >= 'A' && c <= 'F'){
			val = c - 'A' + 10;
		} else if(c >= 'a' && c <= 'f') {
			val = c - 'a' + 10;
		}
	} break;
	}
	return val;
}

bool str_parse_i64(String s, u32 base, i64* out){
	if(s.len == 0){ return false; }

	bool negate = false;
	if(s.v[0] == '-'){
		negate = true;
		s = str_sub(s, 1, s.len);
	}

	i64 n = 0;
	*out = 0;

	int digit_count = 0;
	for(isize i = s.len - 1; i >= 0; i -= 1){
		char c = s.v[i];
		if(c == '_'){ continue; }

		i64 dig = str_digit_value(c, 16);
		if(dig < 0){
			return false;
		}

		switch(base){
		case 2:  n |= dig << digit_count; break;
		case 8:  n |= dig << (digit_count * 3); break;
		case 16: n |= dig << (digit_count * 4); break;
		case 10: n += dig * str_ipow10(digit_count); break;
		}

		digit_count += 1;
	}

	if(negate){
		n = -n;
	}

	*out = n;
	return true;
}

#define STRCONV_TEMP_BUFFER_SIZE 512

bool str_parse_f64(String s, f64* out){
	if(s.len == 0){ return false; }

	*out = 0;
	char digits[STRCONV_TEMP_BUFFER_SIZE] = {0};
	isize digit_count = 0;

	/* Extract digits only */ {
		for(isize i = 0; i < s.len && digit_count < STRCONV_TEMP_BUFFER_SIZE; i++){
			char c = s.v[i];
			if(c == '_'){ continue; }

			digits[digit_count] = s.v[i];
			digit_count += 1;
		}
	}

	char* p = 0;

	f64 val = strtod(&digits[0], &p);
	if(p != &digits[digit_count]){
		return false;
	}

	*out = val;
	return true;
}

#undef STRCONV_TEMP_BUFFER_SIZE
