#include "base/types.h"
#include "base/ensure.h"
#include "base/memory.h"
#include "base/string.h"

#include "cx.h"

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

	isize temp_size = 24 * mem_kilobyte;
	isize arena_size = 128 * mem_kilobyte;

	byte* arena_mem = heap_alloc(arena_size, alignof(void*));
	byte* temp_mem = heap_alloc(temp_size, alignof(void*));

	Arena arena = arena_create_buffer(arena_mem, arena_size);
	Arena temp_arena = arena_create_buffer(temp_mem, temp_size);

	Lexer lex = {
		.source = s,
		.arena = &arena,
	};

	for(Token token = lexer_next(&lex);
		token.type != Tk_EndOfFile;
		token = lexer_next(&lex))
	{
		printf("%s\n", token_format(token, &temp_arena).v);
		arena_reset(&temp_arena);
	}

	for(CompilerError* error = lex.error;
		error != NULL;
		error = error->next)
	{
		printf("\e[31mError\e[0m: %.*s\n", str_fmt(error->message));
	}
}

