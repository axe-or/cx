#include "base/types.h"
#include "base/ensure.h"
#include "base/memory.h"
#include "base/string.h"

#include "cx.h"

str_attribute_format(3,4)
void lexer_emit_error(Lexer* lex, CompilerErrorType errtype, char const * restrict fmt, ...) {
	String s = {};
	CompilerError* new_error = arena_make(lex->arena, CompilerError, 1);
	new_error->type = errtype;
	new_error->next = lex->error;
	lex->error = new_error;

	va_list argp;
	va_start(argp, fmt);
	new_error->message = str_vformat(lex->arena, fmt, argp);
	va_end(argp);
}

int token_print(Token t){
	ensure(t.type >= 0 && t.type < Tk__COUNT, "Invalid type value");

	if(t.type == Tk_Integer){
		return printf("Int(%td)", t.value_integer);
	}
	if(t.type == Tk_Real){
		return printf("Real(%g)", t.value_real);
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

int main(){
	String s = str_lit(
		"([ _  += ](){})>>=>>><<=<<<"
		"let skibi: i32 = bop"
		" 0xff_00_1a"
		" 0b1010"
		" 0o777"
		" 69f420"
		" 69.420e-5"
	);

	isize arena_size = 128 * mem_kilobyte;
	byte* arena_mem = heap_alloc(arena_size, alignof(void*));
	Arena arena = arena_create_buffer(arena_mem, arena_size);

	Lexer lex = {
		.source = s,
		.arena = &arena,
	};

	for(Token token = lexer_next(&lex);
		token.type != Tk_EndOfFile;
		token = lexer_next(&lex))
	{
		token_print(token);
		printf("\n");
	}

	for(CompilerError* error = lex.error;
		error != NULL;
		error = error->next)
	{
		printf("\e[31mError\e[0m: %.*s\n", str_fmt(error->message));
	}
}

