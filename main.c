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

static const struct { String lexeme; TokenType type; } token_keywords[] = {
	{ str_lit("fn"), Tk_Fn },
	{ str_lit("return"), Tk_Return },
	{ str_lit("if"), Tk_If },
	{ str_lit("else"), Tk_Else },
	{ str_lit("for"), Tk_For },
	{ str_lit("brea"), Tk_Break },
	{ str_lit("continue"), Tk_Continue },
	{ str_lit("match"), Tk_Match },
	{ str_lit("nil"), Tk_Nil },
	{ str_lit("true"), Tk_True },
	{ str_lit("false"), Tk_False },
};

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
	[Tk_AssignOp]   = str_lit("AssignOp"),

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

	[Tk_Bang]     = str_lit("!"),
	[Tk_LogicAnd] = str_lit("&&"),
	[Tk_LogicOr]  = str_lit("||"),

	[Tk_Gt]    = str_lit(">"),
	[Tk_Lt]    = str_lit("<"),
	[Tk_GtEq]  = str_lit(">="),
	[Tk_LtEq]  = str_lit("<="),
	[Tk_Eq]    = str_lit("=="),
	[Tk_NotEq] = str_lit("!="),

	[Tk_Integer] = str_lit("Int"),
	[Tk_Real]    = str_lit("Real"),
	[Tk_String]  = str_lit("String"),
	[Tk_Char]    = str_lit("Char"),
	[Tk_Id]      = str_lit("Id"),

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

bool lexer_advance_if(Lexer* lex, rune target){
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


int token_print(Token t){
	ensure(t.type >= 0 && t.type < Tk__COUNT, "Invalid type value");

	if(t.type == Tk_Integer){
		return printf("Int(%td)", t.value_integer);
	}
	if(t.type == Tk_Real){
		return printf("Int(%g)", t.value_real);
	}
	if(t.type == Tk_String){
		return printf("Str(\"%.*s\")", str_fmt(t.value_string));
	}
	if(t.type == Tk_Char){
		return printf("Char(%d)", t.value_char);
	}
	if(t.type == Tk_Id){
		return printf("Id(%.*s)", str_fmt(t.lexeme));
	}

	String name = token_type_name[t.type];
	return printf("%.*s", str_fmt(name));
}

static inline
Token lexer_match_arith_or_assign(Lexer* lex, TokenType alt){
	Token res = {
		.type = alt,
	};

	if(lexer_advance_if(lex, '=')){
		res.assign_operator = alt;
		res.type = Tk_AssignOp;
	}

	return res;
}

static inline
Token lexer_match_identifier(Lexer* lex){
	lex->previous = lex->current;
	Token res = {
		.type = Tk_Id,
	};
	ensure(is_identifier(lexer_peek(lex, 0)), "Lexer is not on an identifier");

	for(;;){
		rune c = lexer_advance(lex);
		if(c == 0){ break; }

		if(!is_identifier(c)){
			lex->current -= utf8_rune_size(c);
			break;
		}
	}


	//TODO: kewyrod
	res.lexeme = lexer_current_lexeme(lex);
	return res;
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
		if(lexer_advance_if(lex, '=')){
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

	case '+': res = lexer_match_arith_or_assign(lex, Tk_Plus); break;
	case '-': res = lexer_match_arith_or_assign(lex, Tk_Minus); break;
	case '*': res = lexer_match_arith_or_assign(lex, Tk_Star); break;
	case '%': res = lexer_match_arith_or_assign(lex, Tk_Modulo); break;
	case '/':
		if(lexer_advance_if(lex, '/')){
			unimplemented("COmment");
		} else {
			res = lexer_match_arith_or_assign(lex, Tk_Slash);
		}
	break;

	case '~': res.type = Tk_Tilde; break;
	case '&':
		if(lexer_advance_if(lex, '&')){
			res.type = Tk_LogicAnd;
		} else {
			res = lexer_match_arith_or_assign(lex, Tk_And);
		}
	break;
	case '|':
		if(lexer_advance_if(lex, '|')){
			res.type = Tk_LogicOr;
		} else {
			res = lexer_match_arith_or_assign(lex, Tk_Or);
		}
	break;
	case '>':
		if(lexer_advance_if(lex, '>')){
			res = lexer_match_arith_or_assign(lex, Tk_ShRight);
		} else if(lexer_advance_if(lex, '=')) {
			res.type = Tk_GtEq;
		} else {
			res.type = Tk_Gt;
		}
	break;
	case '<':
		if(lexer_advance_if(lex, '<')){
			res = lexer_match_arith_or_assign(lex, Tk_ShLeft);
		} else if(lexer_advance_if(lex, '=')) {
			res.type = Tk_LtEq;
		} else {
			res.type = Tk_Lt;
		}
	break;

	case '!':
		if(lexer_advance_if(lex, '=')){
			res.type = Tk_NotEq;
		} else {
			res.type = Tk_Bang;
		}
	break;

	default:
		if(is_decimal(c)){
			unimplemented("num");
		}
		else if(is_identifier(c)){
			lex->current -= utf8_rune_size(c);
			res = lexer_match_identifier(lex);
		}
		else {
			panic("bah");
		}
	}

	return res;
}

int main(){
	String s = str_lit(
		"([ _  += ](){})>>=>>><<=<<<"
		"let skibi = bop"
	);

	Lexer lex = {
		.source = s,
	};

	for(Token token = lexer_next(&lex);
		token.type != Tk_EndOfFile;
		token = lexer_next(&lex))
	{
		// printf("%.*s\n", str_fmt(token_type_name[token.type]));
		token_print(token);
		printf("\n");
	}
}

