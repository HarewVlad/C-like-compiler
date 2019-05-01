typedef struct Expr Expr;

typedef enum ExprKind
{
	EXPR_NONE,
	EXPR_INT,
	EXPR_DOUBLE,
	EXPR_CHAR,
	EXPR_NAME,
	EXPR_BINARY,
	// ...
} ExprKind;

struct Expr // + - * /, TODO: add more
{
	ExprKind kind;
	union
	{
		int64_t int_val;
		double double_val;
		const char *name;
		char char_val;
		struct
		{
			TokenKind op;
			Expr *left;
			Expr *right;
		} binary;
	};
};

Expr *new_expr(ExprKind kind)
{
	Expr *e = malloc(sizeof(Expr));
	e->kind = kind;
	return e;
}

Expr *new_expr_int(int64_t int_val)
{
	Expr *e = new_expr(EXPR_INT);
	e->int_val = int_val;
	return e;
}

Expr *new_expr_double(double double_val)
{
	Expr *e = new_expr(EXPR_DOUBLE);
	e->double_val = double_val;
	return e;
}

Expr *new_expr_name(const char *name)
{
	Expr *e = new_expr(EXPR_NAME);
	e->name = name;
	return e;
}

Expr *new_expr_char(char char_val)
{
	Expr *e = new_expr(EXPR_CHAR);
	e->char_val = char_val;
	return e;
}

Expr *new_expr_binary(TokenKind op, Expr *left, Expr *right)
{
	Expr *e = new_expr(EXPR_BINARY);
	e->binary.op = op;
	e->binary.left = left;
	e->binary.right = right;
	return e;
}

typedef enum DeclKind
{
	DECL_VAR,
	// ...
} DeclKind;

typedef enum Typespec
{
	VAR_INT,
	VAR_DOUBLE,
	VAR_CHAR,
	// ...

} Typespec;

typedef struct Decl
{
	DeclKind kind;
	const char *name;
	union
	{
		struct
		{
			Typespec type;
			Expr *expr;
		} var;
	};
} Decl;

Decl *new_decl(DeclKind kind)
{
	Decl *d = malloc(sizeof(Decl));
	d->kind = kind;
	return d;
}

Decl *new_decl_var(Typespec type, const char *name, Expr *expr)
{
	Decl *d = new_decl(DECL_VAR);
	d->var.type = type;
	d->name = name;
	d->var.expr = expr;
	return d;
}

typedef enum StmtKind
{
	STMT_DECL,
	STMT_ASIGN,
	STMT_BLOCK,
	STMT_IF,
} StmtKind;

typedef struct Stmt Stmt;

typedef struct ElseIf
{
	Expr *cond;
	Stmt *block;
} ElseIf;

struct Stmt
{
	StmtKind kind;
	union
	{
		Decl *decl;
		struct // TODO: rethink this
		{
			Stmt **stmts;
		} block;
		struct
		{
			const char *name;
			Expr *e;
		} asign;
		struct
		{
			Expr *cond;
			Stmt *then_block;
			ElseIf *else_ifs;
			Stmt *else_block;
		} if_stmt;
	};
};

Stmt *new_stmt(StmtKind kind)
{
	Stmt *s = malloc(sizeof(Stmt));
	s->kind = kind;
	return s;
}

Stmt *new_stmt_decl(Decl *decl)
{
	Stmt *s = new_stmt(STMT_DECL);
	s->decl = decl;
	return s;
}

Stmt *new_stmt_asign(const char *name, Expr *e)
{
	Stmt *s = new_stmt(STMT_ASIGN);
	s->asign.name = name;
	s->asign.e = e;
	return s;
}

Stmt *new_stmt_block(Stmt **stmts)
{
	Stmt *s = new_stmt(STMT_BLOCK);
	s->block.stmts = stmts;
	return s;
}

Stmt *new_stmt_if(Expr *cond, Stmt *then_block, ElseIf *else_ifs, Stmt *else_block)
{
	Stmt *s = new_stmt(STMT_IF);
	s->if_stmt.cond = cond;
	s->if_stmt.then_block = then_block;
	s->if_stmt.else_ifs = else_ifs;
	s->if_stmt.else_block = else_block;
	return s;
}

void dump_expr_kind(Expr *e)
{
	switch (e->kind)
	{
	case EXPR_CHAR:
		printf("char");
		break;
	case EXPR_INT:
		printf("int");
		break;
	case EXPR_DOUBLE:
		printf("double");
		break;
	default:
		assert(0);
		break;
	// ...
	}
}

void dump_expr(Expr *e)
{
	switch (e->kind)
	{
	case EXPR_INT:
		printf("%I64d", e->int_val);
		break;
	case EXPR_DOUBLE:
		printf("%g", e->double_val);
		break;
	case EXPR_NAME:
		printf("%s", e->name);
		break;
	case EXPR_CHAR:
		printf("%c", e->char_val);
		break;
	case EXPR_BINARY:
		printf("(%s ", token_to_char[e->binary.op]);
		dump_expr(e->binary.left);
		printf(" ");
		dump_expr(e->binary.right);
		printf(")");
		break;
	case EXPR_NONE:
		break;
	default:
		assert(0);
		break;
	}
}

long long dump_expr_value(Expr *e)
{
	switch (e->kind)
	{
	case EXPR_INT:
		return (long long int)e->int_val;
		break;
	case EXPR_DOUBLE:
		// ...
		break;
	case EXPR_CHAR:
		return (char)e->char_val;
		break;
	default:
		assert(0);
		break;
	}
}

void dump_typespec(Typespec type)
{
	switch (type)
	{
	case VAR_INT:
		printf("int");
		break;
	case VAR_CHAR:
		printf("char");
		break;
	case VAR_DOUBLE:
		printf("double");
		break;
	// ...
	default:
		assert(0);
		break;
	}
}

void dump_decl(Decl *d)
{
	switch (d->kind)
	{
	case DECL_VAR:
		printf("(");
		dump_typespec(d->var.type);
		printf(" %s", d->name);
		printf(" ");
		if (d->var.expr != NULL)
		{
			printf("= ");
			dump_expr(d->var.expr);
		}
		printf(")");
		break;
	default:
		assert(0);
		break;
	}
}

void dump_stmt(Stmt *s)
{
	switch (s->kind)
	{
	case STMT_DECL:
		dump_decl(s->decl);
		break;
	case STMT_BLOCK:
		printf("({");
		for (size_t i = 0; i < buf_len(s->block.stmts); i++)
		{
			dump_stmt(s->block.stmts[i]);
		}
		printf("})");
		break;
	case STMT_ASIGN:
		printf("(%s = ", s->asign.name);
		dump_expr(s->asign.e);
		printf(")");
		break;
	case STMT_IF:
		printf("(if ");
		dump_expr(s->if_stmt.cond);
		if (s->if_stmt.then_block != NULL)
			dump_stmt(s->if_stmt.then_block);
		if (s->if_stmt.else_ifs != NULL)
		{
			for (size_t i = 0; i < buf_len(s->if_stmt.else_ifs); i++)
			{
				printf("(else if ");
				dump_expr(s->if_stmt.else_ifs->cond);
				dump_stmt(s->if_stmt.else_ifs->block);
				printf(")");
			}
		}
		if (s->if_stmt.else_block != NULL)
		{
			printf("(else ");
			dump_stmt(s->if_stmt.else_block);
			printf(")");
		}
		printf(")");
		break;
	default:
		assert(0);
		break;
	}
}
