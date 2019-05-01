typedef enum SymKind
{
	SYM_UNRESOLVED,
	SYM_RESOLVING,
	SYM_RESOLVED
} SymKind;

typedef enum SymType
{
	SYM_DECL,
	SYM_STMT,
} SymType;

typedef struct Sym
{
	SymKind kind;
	SymType type;
	union
	{
		Decl *decl;
		Stmt *stmt;
	};
} Sym;

Sym *global_table;
Sym **local_table;

Decl *get_sym_decl(const char *name) // TODO: add name to struct
{
	Decl *d = NULL;
	for (size_t i = 0; i < buf_len(local_table); i++)
	{
		for (size_t j = 0; j < buf_len(local_table[i]); j++)
		{
			Sym s = local_table[i][j];
			d = local_table[i][j].decl;
			if (s.type == SYM_DECL && strcmp(d->name, name) == 0) // TODO: str interning
			{
				return d;
			}
		}
	}
	return d;
}

void fill_local_table(Stmt *s);

void fill_sym(Sym **new_local_table, Stmt *stmt) // TODO: for now assume thats all inside block, redo later
{
	switch (stmt->kind)
	{
	case STMT_DECL:
		buf_push(*new_local_table, (Sym) { SYM_UNRESOLVED, SYM_DECL, stmt->decl });
		break;
	case STMT_IF: // TODO: resolve cond.
		fill_local_table(stmt->if_stmt.then_block);
		if (stmt->if_stmt.else_ifs != NULL)
		{
			for (size_t i = 0; i < buf_len(stmt->if_stmt.else_ifs); i++)
			{
				fill_local_table(stmt->if_stmt.else_ifs[i].block);
			}
		}
		if (stmt->if_stmt.else_block != NULL)
		{
			fill_local_table(stmt->if_stmt.else_block);
		}
		break;
	case STMT_ASIGN:
		buf_push(*new_local_table, (Sym) { SYM_UNRESOLVED, SYM_STMT, stmt });
		break;
	default:
		assert(0);
		break;
	}
}

// |1| - 1, 2, 3 |2| - 1, 2, 3

void fill_local_table(Stmt *s)
{
	Sym *new_local_table = NULL;
	for (size_t i = 0; i < buf_len(s->block.stmts); i++)
	{
		fill_sym(&new_local_table, s->block.stmts[i]);
	}
	buf_push(local_table, new_local_table);
}

void fill_another(Stmt *s)
{
	Sym *new_local_table = NULL;
	for (size_t i = 0; i < buf_len(s->block.stmts); i++)
	{
		fill_local_table(s->block.stmts[i]);
	}

}

// FOR NOW, only resolve local table

Sym *resolve_name_decl(size_t index, const char *name)
{
	for (size_t i = index; i < buf_len(local_table); i++)
	{
		for (size_t j = 0; j < buf_len(local_table[i]); j++)
		{
			Sym *s = &local_table[i][j];
			Decl *d = s->decl;
			if (strcmp(d->name, name) == 0) // TODO: str interning
			{
				if (s->kind == SYM_RESOLVING)
					fatal("cyclic dependency");

				return s;
			}
		}
	}
	return NULL;
}

void resolve_sym(size_t index_table, Sym *s);

void resolve_expr(size_t index_table, Sym *s, Expr *e)
{
	switch (e->kind)
	{
	case EXPR_INT: case EXPR_CHAR: case EXPR_DOUBLE:
		s->kind = SYM_RESOLVED;
		break;
	case EXPR_NAME:
		s->kind = SYM_RESOLVING;
		Sym *new_s = resolve_name_decl(index_table, e->name);
		if (new_s != NULL)
		{
			resolve_sym(index_table, new_s);
		}
		else
			fatal("unresolved sym '%s'", e->name);
		break;
	case EXPR_BINARY:
		resolve_expr(index_table, s, e->binary.left);
		resolve_expr(index_table, s, e->binary.right);
		break;
	default:
		assert(0);
		break;
	}
}

void resolve_decl(size_t index_table, Sym *s)
{
	switch (s->decl->kind)
	{
	case DECL_VAR:
		resolve_expr(index_table, s, s->decl->var.expr);
		break;
	default:
		assert(0);
		break;
	}
}

void resolve_stmt(size_t index_table, Sym *s)
{
	switch (s->stmt->kind)
	{
	case STMT_ASIGN:
		resolve_expr(index_table, s, s->stmt->asign.e);
		break;
	case STMT_IF:
		break;
	default:
		assert(0);
		break;
	}
}

void resolve_sym(size_t index_table, Sym *s)
{
	switch (s->type)
	{
	case SYM_DECL:
		resolve_decl(index_table, s);
		break;
	case SYM_STMT:
		resolve_stmt(index_table, s);
		break;
	default:
		assert(0);
		break;
	}
}

void resolve()
{
	for (size_t i = 0; i < buf_len(local_table); i++)
	{
		for (size_t j = 0; j < buf_len(local_table[i]); j++)
		{
			Sym s = local_table[i][j];
			resolve_sym(i, &s);
		}
	}
}

void dump_sym(Sym s)
{
	switch (s.type)
	{
	case SYM_DECL:
		dump_decl(s.decl);
		break;
	case SYM_STMT:
		dump_stmt(s.stmt);
		break;
	default:
		assert(0);
		break;
	}
}

void dump_global_table()
{
	printf("(Global table -> ");
	for (size_t i = 0; i < buf_len(global_table); i++)
	{
		dump_decl(global_table[i].decl);
	}
	printf(")");
}

void dump_local_table()
{
	for (size_t i = 0; i < buf_len(local_table); i++)
	{
		printf("(Local table -> ");
		for (size_t j = 0; j < buf_len(local_table[i]); j++)
		{
			dump_sym(local_table[i][j]);
		}
		printf(")");
	}
}
