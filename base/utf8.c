#include "string.h"

#define UTF8_RANGE1 ((i32)0x7f)
#define UTF8_RANGE2 ((i32)0x7ff)
#define UTF8_RANGE3 ((i32)0xffff)
#define UTF8_RANGE4 ((i32)0x10ffff)

#define UTF16_SURROGATE1 ((i32)0xd800)
#define UTF16_SURROGATE2 ((i32)0xdfff)

#define UTF8_MASK2 ((u32)0b00011111)
#define UTF8_MASK3 ((u32)0b00001111)
#define UTF8_MASK4 ((u32)0b00000111)

#define UTF8_MASKX ((u32)0b00111111)
#define UTF8_SIZE2 ((u32)0xc0) /* 110x_xxxx */
#define UTF8_SIZE3 ((u32)0xe0) /* 1110_xxxx */
#define UTF8_SIZE4 ((u32)0xf0) /* 1111_0xxx */

#define CONT ((u32)0x80) /* 10xx_xxxx */

#define UTF8_DECODE_ERROR ((UTF8Decoded){ .codepoint = UTF8_ERROR, .len = 1 })

i32 utf8_rune_size(rune c){
	if(0){}
	else if(c <= UTF8_RANGE1){ return 1; }
	else if(c <= UTF8_RANGE2){ return 2; }
	else if(c <= UTF8_RANGE3){ return 3; }
	else if(c <= UTF8_RANGE4){ return 4; }
	return 0;
}

UTF8Encoded utf8_encode(rune c){
	UTF8Encoded res = {};

	if(utf8_is_continuation_byte(c) ||
	   (c >= UTF16_SURROGATE1 && c <= UTF16_SURROGATE2) ||
	   (c > UTF8_RANGE4))
	{
		return UTF8_ERROR_ENCODED;
	}

	if(c <= UTF8_RANGE1){
		res.len = 1;
		res.bytes[0] = (u8)(c & 0xff);
	}
	else if(c <= UTF8_RANGE2){
		res.len = 2;
		res.bytes[0] = UTF8_SIZE2 | ((c >> 6) & UTF8_MASK2);
		res.bytes[1] = CONT       | ((c >> 0) & UTF8_MASKX);
	}
	else if(c <= UTF8_RANGE3){
		res.len = 3;
		res.bytes[0] = UTF8_SIZE3 | ((c >> 12) & UTF8_MASK3);
		res.bytes[1] = CONT       | ((c >> 6) & UTF8_MASKX);
		res.bytes[2] = CONT       | ((c >> 0) & UTF8_MASKX);
	}
	else if(c <= UTF8_RANGE4){
		res.len = 4;
		res.bytes[0] = UTF8_SIZE4 | ((c >> 18) & UTF8_MASK4);
		res.bytes[1] = CONT       | ((c >> 12) & UTF8_MASKX);
		res.bytes[2] = CONT       | ((c >> 6)  & UTF8_MASKX);
		res.bytes[3] = CONT       | ((c >> 0)  & UTF8_MASKX);
	}
	return res;
}

UTF8Decoded utf8_decode(byte const* buf, isize len){
	UTF8Decoded res = {};

	if(buf == NULL || len <= 0){ return UTF8_DECODE_ERROR; }

	byte first = buf[0];

	if((first & CONT) == 0){
		res.len = 1;
		res.codepoint |= first;
	}
	else if ((first & ~UTF8_MASK2) == UTF8_SIZE2 && len >= 2){
		res.len = 2;
		res.codepoint |= (buf[0] & UTF8_MASK2) << 6;
		res.codepoint |= (buf[1] & UTF8_MASKX) << 0;
	}
	else if ((first & ~UTF8_MASK3) == UTF8_SIZE3 && len >= 3){
		res.len = 3;
		res.codepoint |= (buf[0] & UTF8_MASK3) << 12;
		res.codepoint |= (buf[1] & UTF8_MASKX) << 6;
		res.codepoint |= (buf[2] & UTF8_MASKX) << 0;
	}
	else if ((first & ~UTF8_MASK4) == UTF8_SIZE4 && len >= 4){
		res.len = 4;
		res.codepoint |= (buf[0] & UTF8_MASK4) << 18;
		res.codepoint |= (buf[1] & UTF8_MASKX) << 12;
		res.codepoint |= (buf[2] & UTF8_MASKX) << 6;
		res.codepoint |= (buf[3] & UTF8_MASKX) << 0;
	}
	else {
		return UTF8_DECODE_ERROR;
	}

	// Validation step
	if(res.codepoint >= UTF16_SURROGATE1 && res.codepoint <= UTF16_SURROGATE2){
		return UTF8_DECODE_ERROR;
	}
	if(res.len > 1 && !utf8_is_continuation_byte(buf[1])){
		return UTF8_DECODE_ERROR;
	}
	if(res.len > 2 && !utf8_is_continuation_byte(buf[2])){
		return UTF8_DECODE_ERROR;
	}
	if(res.len > 3 && !utf8_is_continuation_byte(buf[3])){
		return UTF8_DECODE_ERROR;
	}

	return res;
}

#undef UTF8_RANGE1
#undef UTF8_RANGE2
#undef UTF8_RANGE3
#undef UTF8_RANGE4
#undef UTF16_SURROGATE1
#undef UTF16_SURROGATE2
#undef UTF8_MASK2
#undef UTF8_MASK3
#undef UTF8_MASK4
#undef UTF8_MASKX
#undef UTF8_SIZE2
#undef UTF8_SIZE3
#undef UTF8_SIZE4
#undef CONT

