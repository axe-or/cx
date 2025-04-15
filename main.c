#include "base/types.h"
#include "base/ensure.h"
#include "base/memory.h"
#include "base/string.h"

typedef struct {
	String source;
	isize current;
	isize previous;
} Lexer;

typedef enum {
	Tk_Unknown = 0,

	// Delimiters
	Tk_ParenOpen,
	Tk_ParenClose,
	Tk_SquareOpen,
	Tk_SquareClose,
	Tk_CurlyOpen,
	Tk_CurlyClose,

	// Special operators
	Tk_Dot,
	Tk_Comma,
	Tk_Colon,
	Tk_Semicolon,
	Tk_Assign,
	Tk_Underscore,

	// Arithmetic
	Tk_Plus,
	Tk_Minus,
	Tk_Star,
	Tk_Slash,
	Tk_Modulo,

	// Bitwise
	Tk_Tilde,
	Tk_And,
	Tk_Or,
	Tk_ShLeft,
	Tk_ShRight,

	// Logic
	Tk_Bang,
	Tk_And2,
	Tk_Or2,

	// Comparison
	Tk_Gt,
	Tk_Lt,
	Tk_GtEq,
	Tk_LtEq,
	Tk_Eq,
	Tk_NotEq,

	// Literals & special
	Tk_Integer,
	Tk_Real,
	Tk_String,
	Tk_Char,
	Tk_Id,

	// Keywords
	Tk_Fn,
	Tk_Return,
	Tk_If,
	Tk_Else,
	Tk_For,
	Tk_Break,
	Tk_Continue,
	Tk_Match,
	Tk_Nil,
	Tk_True,
	Tk_False,

	// Control
	Tk_Invalid,
	Tk_EndOfFile,
} TokenKind;

typedef struct {
	String lexeme;
	u32 kind;

	union {
		f64    value_real;
		i64    value_integer;
		rune   value_char;
		String value_string;
	};
} Token;

rune lexer_peek(Lexer* lex, isize delta){
	isize pos = lex->current + delta;
	if(pos < 0 || pos >= lex->source.len){
		return 0; /* OOB */
	}

	UTF8Decoded dec = utf8_decode(lex->source.v + pos, lex->source.len - pos);
	return dec.codepoint;
}

rune lexer_advance(Lexer* lex){
	if(lex->current >= lex->source.len){
		return 0; /* EOF */
	}

	UTF8Decoded dec = utf8_decode(lex->source.v + lex->current, lex->source.len - lex->current);
	if(dec.codepoint == UTF8_ERROR){
		lex->current += 1;
		return UTF8_ERROR; /* Invalid char */
	}

	lex->current += dec.len;
	return dec.codepoint;
}

bool lexer_match_advance(Lexer* lex, rune target){
	rune c = lexer_peek(lex, 0);
	if(c == target){
		lex->current += utf8_rune_size(c);
		return true;
	}
	return false;
}

int main(){
}

