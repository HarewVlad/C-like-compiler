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

Expr *parse_expr2()
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

Expr *parse_expr1()
{
	Expr *e = parse_expr2();
	while (is_token('*') || is_token('/'))
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
	while (is_token('+') || is_token('-'))
	{
		char op = token.kind;
		next();
		Expr *er = parse_expr1();
		e = new_expr_binary(op, e, er);
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
		if (match_token('='))
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
		if (match_token('='))
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
		if (match_token('='))
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
	expect_token('=');
	Expr *e = parse_expr();
	expect_token(';');
	return new_stmt_asign(name, e);
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
		s = parse_stmt_asign();
	}
	return s;
}