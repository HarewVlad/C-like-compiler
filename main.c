#include "utils.c"
#include "emitter.c"
#include "lex.c"
#include "ast.c"
#include "parse.c"
#include "resolve.c"
#include "type_check.c"
#include "gen.c"

#pragma comment(lib, "user32.lib")

#define LEX_TEST(a) (assert(token.kind == a), next())

void init_stream(const char *src)
{
	stream = src;
	next();
}

void lex_test()
{
	init_stream("if else hello 'H' 22.22 123.333 12 == += -= *= /= ++ --");
	LEX_TEST(TOKEN_KEYWORD);
	LEX_TEST(TOKEN_KEYWORD);
	LEX_TEST(TOKEN_NAME);
	LEX_TEST(TOKEN_CHAR);
	LEX_TEST(TOKEN_DOUBLE);
	LEX_TEST(TOKEN_DOUBLE);
	LEX_TEST(TOKEN_INT);
	LEX_TEST(TOKEN_EQ_EQ);
	LEX_TEST(TOKEN_ADD_EQ);
	LEX_TEST(TOKEN_SUB_EQ);
	LEX_TEST(TOKEN_MUL_EQ);
	LEX_TEST(TOKEN_DIV_EQ);
	LEX_TEST(TOKEN_INC);
	LEX_TEST(TOKEN_DEC);
}

void resolve_test()
{
	resolve();
	printf("\n");
}

void parse_test()
{
	init_regs();

	const char *code[] = // STMT_EXPR for checking cond.
	{
		// "{ 1 + 2; int a = 10; if (a) {int b = a + 1;} else {int b = 10;}}",
		"{ int a = 10; if (a) { int b = 20; } else {int c = 30; } }"
	};

	Stmt **stmt_list = NULL;
		
	for (size_t i = 0; i < sizeof(code) / sizeof(*code); i++)
	{
		init_stream(code[i]);
		Stmt *s = parse_stmt();
		buf_push(stmt_list, s);
		fill0(s);
	}

	dump_sym_table();

	printf("Resolve -> \n");
 	resolve_test();
	
	printf("Type check -> \n");
	for (size_t i = 0; i < buf_len(stmt_list); i++)
	{
		type_check_stmt(stmt_list[i]);
		dump_stmt(stmt_list[i]);
		printf("\n");
	}

	printf("Code gen -> \n");
	/*for (size_t i = 0; i < buf_len(stmt_list); i++)
	{
		install_operand(stmt_list[i]);
	}
	*/

	for (size_t i = 0; i < buf_len(stmt_list); i++)
	{
		gen_stmt(stmt_list[i]);
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

extern void asm_proc();

int main()
{
	// test_emit();
	init_keywords();
	init_regs();

	lex_test();
	parse_test();
	emit_code();
}
