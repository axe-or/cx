#pragma once
#include "base/types.h"
#include "base/memory.h"
#include "base/string.h"

typedef enum {
	CompilerError_UnknownToken,
	CompilerError_InvalidNumber,
} CompilerErrorType;

typedef struct CompilerError CompilerError;

struct CompilerError {
	String filename;
	isize offset;
	String message;
	u32 type;
	CompilerError* next;
};

typedef struct {
	String source;
	isize current;
	isize previous;

	CompilerError* error;
	Arena* arena;
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
	Tk_AssignOp, /* Assignment + Some arithmetic/bitwise operator */

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
	Tk_LogicAnd,
	Tk_LogicOr,

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
	Tk_Let,
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

	Tk__COUNT,
} TokenType;

typedef struct {
	String lexeme;
	u32 type;

	union {
		f64    value_real;
		i64    value_integer;
		rune   value_char;
		String value_string;
		u32    assign_operator; /* Only for Tk_AssignOp */
	};
} Token;

typedef struct {
	Token* tokens;
	isize  token_count;
	CompilerError* error;
} LexerResult;

rune lexer_peek(Lexer* lex, isize delta);

rune lexer_advance(Lexer* lex);

bool lexer_advance_if(Lexer* lex, rune target);

Token lexer_match_number(Lexer* lex);

Token lexer_match_arith_or_assign(Lexer* lex, TokenType alt);

Token lexer_match_identifier_or_keyword(Lexer* lex);

Token lexer_match_string(Lexer* lex);

Token lexer_next(Lexer* lex);

void lexer_emit_error(Lexer* lex, CompilerErrorType errtype, char const * restrict fmt, ...) str_attribute_format(3,4);

String token_format(Token t, Arena* arena);
