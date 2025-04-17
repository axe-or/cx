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

static const String token_type_name[] = {
	[Tk_ParenOpen]   = str_lit("("),
	[Tk_ParenClose]  = str_lit(")"),
	[Tk_SquareOpen]  = str_lit("["),
	[Tk_SquareClose] = str_lit("]"),
	[Tk_CurlyOpen]   = str_lit("{"),
	[Tk_CurlyClose]  = str_lit("}"),

	[Tk_Dot]        = str_lit("."),
	[Tk_Comma]      = str_lit(","),
	[Tk_Colon]      = str_lit(":"),
	[Tk_Semicolon]  = str_lit(";"),
	[Tk_Assign]     = str_lit("="),
	[Tk_Underscore] = str_lit("_"),
	[Tk_AssignOp]   = str_lit("<AssignOp>"),

	[Tk_Plus]   = str_lit("+"),
	[Tk_Minus]  = str_lit("-"),
	[Tk_Star]   = str_lit("*"),
	[Tk_Slash]  = str_lit("/"),
	[Tk_Modulo] = str_lit("%"),

	[Tk_Tilde]   = str_lit("~"),
	[Tk_And]     = str_lit("&"),
	[Tk_Or]      = str_lit("|"),
	[Tk_ShLeft]  = str_lit("<<"),
	[Tk_ShRight] = str_lit(">>"),

	[Tk_Gt]    = str_lit(">"),
	[Tk_Lt]    = str_lit("<"),
	[Tk_GtEq]  = str_lit(">="),
	[Tk_LtEq]  = str_lit("<="),
	[Tk_Eq]    = str_lit("=="),
	[Tk_NotEq] = str_lit("!="),

	[Tk_Bang]     = str_lit("!"),
	[Tk_LogicAnd] = str_lit("&&"),
	[Tk_LogicOr]  = str_lit("||"),

	[Tk__COUNT] = {},
};

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

String lexer_current_lexeme(Lexer const* lex){
	return str_sub(lex->source, lex->previous, lex->current);
}

static inline
bool is_alpha(rune c){
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static inline
bool is_decimal(rune c){
	return (c >= '0' && c <= '9');
}

static inline
bool is_identifier(rune c){
	return is_alpha(c) || is_decimal(c) || (c == '_');
}

static inline
bool is_whitespace(rune c){
	return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n') || (c == '\v');
}

#define MATCH_ASSIGN(Type) {\
	res.type = Type; \
	if(lexer_match_advance(lex, '=')){ \
		res.assign_operator = Type; \
		res.type = Tk_AssignOp; \
	} \
}

Token lexer_next(Lexer* lex){
	Token res = {
		.type = Tk_Unknown,
	};

	rune c = lexer_advance(lex);
	while(is_whitespace(c) && c != 0){
		c = lexer_advance(lex);
	}

	if(c == 0){
		res.type = Tk_EndOfFile;
		return res;
	}

	switch(c){
	case '(': res.type = Tk_ParenOpen; break;
	case ')': res.type = Tk_ParenClose; break;
	case '[': res.type = Tk_SquareOpen; break;
	case ']': res.type = Tk_SquareClose; break;
	case '{': res.type = Tk_CurlyOpen; break;
	case '}': res.type = Tk_CurlyClose; break;
	
	case ':': res.type = Tk_Colon; break;
	case ';': res.type = Tk_Semicolon; break;
	case ',': res.type = Tk_Comma; break;
	case '.': res.type = Tk_Dot; break;
	case '=':
		if(lexer_match_advance(lex, '=')){
			res.type = Tk_Eq;
		} else {
			res.type = Tk_Assign;
		}
	break;
	case '_':
		if(!is_identifier(lexer_peek(lex, 0))){
			res.type = Tk_Underscore;
		} else {
			unimplemented("Identifier");
		}
	break;

	case '+': MATCH_ASSIGN(Tk_Plus); break;
	case '-': MATCH_ASSIGN(Tk_Minus); break;
	case '*': MATCH_ASSIGN(Tk_Star); break;
	case '%': MATCH_ASSIGN(Tk_Modulo); break;
	case '/':
		if(lexer_match_advance(lex, '/')){
			unimplemented("COmment");
		} else {
			MATCH_ASSIGN(Tk_Slash);
		}
	break;

	case '~': res.type = Tk_Tilde; break;
	case '&':
		if(lexer_match_advance(lex, '&')){
			res.type = Tk_LogicAnd;
		} else {
			MATCH_ASSIGN(Tk_And);
		}
	break;
	case '|':
		if(lexer_match_advance(lex, '|')){
			res.type = Tk_LogicOr;
		} else {
			MATCH_ASSIGN(Tk_Or);
		}
	break;
	case '>':
		if(lexer_match_advance(lex, '>')){
			MATCH_ASSIGN(Tk_ShRight);
		} else if(lexer_match_advance(lex, '=')) {
			res.type = Tk_GtEq;
		} else {
			res.type = Tk_Gt;
		}
	break;
	case '<':
		if(lexer_match_advance(lex, '<')){
			MATCH_ASSIGN(Tk_ShLeft);
		} else if(lexer_match_advance(lex, '=')) {
			res.type = Tk_LtEq;
		} else {
			res.type = Tk_Lt;
		}
	break;

	case '!':
		if(lexer_match_advance(lex, '=')){
			res.type = Tk_NotEq;
		} else {
			res.type = Tk_Bang;
		}
	break;
	}

	return res;
}

#undef MATCH_ASSIGN

int main(){
	String s = str_lit(
		"([ _  += ](){})>>=>>><<=<<<"
	);

	Lexer lex = {
		.source = s,
	};

	for(Token token = lexer_next(&lex);
		token.type != Tk_EndOfFile;
		token = lexer_next(&lex))
	{
		printf("%.*s\n", str_fmt(token_type_name[token.type]));
	}
}

