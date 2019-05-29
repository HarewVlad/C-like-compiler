bool is_token(TokenKind kind)
{
	return token.kind == kind;
}

bool is_keyword(const char *name)
{
	return is_token(TOKEN_KEYWORD) && token.name == name;
}

bool match_token(TokenKind kind)
{
	if (is_token(kind))
	{
		next();
		return true;
	}
	return false;
}

bool match_keyword(const char *keyword)
{
	if (is_keyword(keyword))
	{
		next();
		return true;
	}
	return false;
}

bool expect_token(TokenKind kind)
{
	if (is_token(kind))
	{
		next();
		return true;
	}
	fatal("expected %c got %c", kind, token.kind);
	return false;
}
Expr *parse_expr();

Expr *parse_expr3()
{
	if (is_token(TOKEN_INT))
	{
		Expr *e = new_expr_int(token.int_val);
		next();
		return e;
	}
	else if (is_token(TOKEN_DOUBLE))
	{
		Expr *e = new_expr_double(token.double_val);
		next();
		return e;
	}
	else if (is_token(TOKEN_NAME))
	{
		Expr *e = new_expr_name(token.name);
		next();
		return e;
	}
	else if (is_token(TOKEN_CHAR))
	{
		Expr *e = new_expr_char(token.char_val);
		next();
		return e;
	}
	else if (match_token('('))
	{
		Expr *e = parse_expr();
		expect_token(')');
		return e;
	}
	else
		fatal("wrong expression");
	return NULL;
}

Expr *parse_expr2()
{
	Expr *e = parse_expr3();
	while (is_token(TOKEN_MUL) || is_token(TOKEN_DIV))
	{
		char op = token.kind;
		next();
		Expr *er = parse_expr3();
		e = new_expr_binary(op, e, er);
	}
	return e;
}

Expr *parse_expr1()
{
	Expr *e = parse_expr2();
	while (is_token(TOKEN_ADD) || is_token(TOKEN_SUB))
	{
		char op = token.kind;
		next();
		Expr *er = parse_expr2();
		e = new_expr_binary(op, e, er);
	}
	return e;
}

Expr *parse_expr0()
{
	Expr *e = parse_expr1();
	while (is_token(TOKEN_EQ_EQ))
	{
		next();
		Expr *er = parse_expr1();
		e = new_expr_binary(TOKEN_EQ_EQ, e, er);
	}
	return e;
}

Expr *parse_expr()
{
	return parse_expr0();
}

const char *parse_name()
{
	const char *name = token.name;
	assert(token.kind == TOKEN_NAME);
	next();
	return name;
}

Decl *parse_decl() // TODO: optimize
{
	Decl *d = NULL;
	if (match_keyword(int_keyword))
	{
		const char *name = parse_name();
		Expr *e = NULL;
		if (match_token(TOKEN_EQ))
		{
			e = parse_expr();
		}
		expect_token(';');
		d = new_decl_var(VAR_INT, name, e);
	}
	else if (match_keyword(char_keyword))
	{
		const char *name = parse_name();
		Expr *e = NULL;
		if (match_token(TOKEN_EQ))
		{
			e = parse_expr();
		}
		expect_token(';');
		d = new_decl_var(VAR_CHAR, name, e);
	}
	else if (match_keyword(double_keyword))
	{
		const char *name = parse_name();
		Expr *e = NULL;
		if (match_token(TOKEN_EQ))
		{
			e = parse_expr();
		}
		expect_token(';');
		d = new_decl_var(VAR_DOUBLE, name, e);
	}
	return d;
}

Stmt *parse_stmt();

Stmt **parse_stmt_list()
{
	expect_token('{');

	Stmt **stmts = NULL;
	while (!is_token('}') && !is_token('\0'))
	{
		Stmt *s = parse_stmt();
		buf_push(stmts, s);
	}

	expect_token('}');

	return stmts;
}

Stmt *parse_stmt_asign()
{
	const char *name = parse_name();
	expect_token(TOKEN_EQ);
	Expr *e = parse_expr();
	expect_token(';');
	return new_stmt_asign(name, e);
}

Stmt *parse_stmt_if()
{
	expect_token('(');
	Expr *cond = NULL;
	if (!is_token(')'))
	{
		cond = parse_expr();
	}
	expect_token(')');
	Stmt *then_block = parse_stmt();
	ElseIf *else_ifs = NULL;
	Stmt *else_block = NULL;
	while (match_keyword(else_keyword))
	{
		if (match_keyword(if_keyword))
		{
			expect_token('(');
			Expr *e = parse_expr();
			expect_token(')');
			Stmt *s = parse_stmt();
			buf_push(else_ifs, (ElseIf) { e, s });
		}
		else
			else_block = parse_stmt();
	}
	return new_stmt_if(cond, then_block, else_ifs, else_block);
}

Stmt *parse_stmt_expr()
{
	Expr *e = parse_expr();
	Stmt *s = new_stmt_expr(e);
	expect_token(';');
	return s;
}

bool make_prediction(int k, TokenKind kind)
{
	char *save_stream = stream;
	TokenKind save_kind = token.kind;

	for (int i = 0; i < k; i++)
	{
		next();
	}

	if (token.kind == kind)
	{
		stream = save_stream;
		token.kind = save_kind;
		return true;
	}
	else
	{
		stream = save_stream;
		token.kind = save_kind;
		return false;
	}
}

Stmt *parse_stmt()
{
	Stmt *s = NULL;
	if (is_token('{'))
	{
		s = new_stmt_block(parse_stmt_list());
	}
	else if (is_keyword(int_keyword)) // TODO: steal handle for this from my work
	{
		s = new_stmt_decl(parse_decl());
	}
	else if (is_keyword(double_keyword))
	{
		s = new_stmt_decl(parse_decl());
	}
	else if (is_keyword(char_keyword))
	{
		s = new_stmt_decl(parse_decl());
	}
	else if (is_token(TOKEN_NAME))
	{
		if (make_prediction(1, TOKEN_ADD))
			s = parse_stmt_expr();
		else
			s = parse_stmt_asign();
	}
	else if (match_keyword(if_keyword))
	{
		s = parse_stmt_if();
	}
	else if (is_token(TOKEN_INT))
	{
		if (make_prediction(1, TOKEN_ADD))
		{
			s = parse_stmt_expr();
		}
		else
			fatal("unexpected decimal '%d'", token.int_val);
	}
	return s;
}
