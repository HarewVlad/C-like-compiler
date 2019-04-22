#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "utils.c"
#include "emitter.c"
#include "lex.c"
#include "ast.c"
#include "parse.c"
#include "resolve.c"
#include "type_check.c"
#include "gen.c"

#pragma comment(lib, "user32.lib")

void lex_test()
{
	stream = "if else hello 'H' 22.22 123.333 12";
	next();
	assert(token.kind == TOKEN_KEYWORD);
	next();
	assert(token.kind == TOKEN_KEYWORD);
	next();
	assert(token.kind == TOKEN_NAME);
	next();
	assert(token.kind == TOKEN_CHAR);
	next();
	assert(token.kind == TOKEN_DOUBLE);
	next();
	assert(token.kind == TOKEN_DOUBLE);
	next();
	assert(token.kind == TOKEN_INT);
}

void init_stream(const char *src)
{
	stream = src;
	next();
}

void resolve_test()
{
	resolve_decls();
	dump_entities();
}

void type_check_test()
{
	resolve_types();
	printf("\n\n");
	dump_entities();
}

void parse_test()
{
	init_regs();

	const char *code[] =
	{
		"{ int a = b + 10.5; int b = 30.333; b = 10; int c = b;}",
	};

	Stmt **stmt_list = NULL;

	for (size_t i = 0; i < sizeof(code) / sizeof(*code); i++)
	{
		init_stream(code[i]);
		Stmt *s = parse_stmt();
		buf_push(stmt_list, s);
		fill_symbol_table(s);
	}

 	resolve_test();
	type_check_test();

	for (size_t i = 0; i < buf_len(stmt_list); i++)
	{
		install_operand(stmt_list[i]);
	}

	for (size_t i = 0; i < buf_len(stmt_list); i++)
	{
		gen(stmt_list[i]);
	}
}

void test_reg()
{
	init_regs();
	for (int i = 0; i < 16; i++)
	{
		Register r = alloc_reg();
		dump_reg(r);
	}
}

int main()
{
	init_keywords();
	lex_test();
	parse_test();
	emit_code();
	}