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

Sym *new_sym(SymType type)
{
	Sym *s = malloc(sizeof(Sym));
	s->type = type;
	return s;
}

Sym *new_sym_decl(Decl *decl)
{
	Sym *s = new_sym(SYM_DECL);
	s->kind = SYM_UNRESOLVED;
	s->decl = decl;
	return s;
}

Sym *new_sym_stmt(Stmt *stmt)
{
	Sym *s = new_sym(SYM_STMT);
	s->kind = SYM_UNRESOLVED;
	s->stmt = stmt;
	return s;
}

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

typedef struct SymTable
{
	Sym *syms;
} SymTable;

typedef struct LocalSymTable
{
	SymTable *sym_tables;
} LocalSymTable;

LocalSymTable **local_sym_tables;

void fill1(LocalSymTable **new_local_sym_table, Stmt *s);
void fill0(Stmt *s);

void fill2(SymTable **new_sym_table, Stmt *s)
{
	Sym *syms = NULL;
	for (size_t i = 0; i < buf_len(s->block.stmts); i++)
	{
		Stmt *stmt = s->block.stmts[i];
		switch (stmt->kind)
		{
		case STMT_DECL:
			buf_push(syms, (Sym) { SYM_UNRESOLVED, SYM_DECL, stmt->decl });
			break;
		case STMT_IF: // TODO: refactor
		{
			SymTable *new_if_sym_table = NULL;
			fill2(&new_if_sym_table, stmt->if_stmt.then_block);
			if (stmt->if_stmt.else_ifs != NULL)
			{
				for (size_t j = 0; j < buf_len(stmt->if_stmt.else_ifs); j++)
				{
					fill2(&new_if_sym_table, stmt->if_stmt.else_ifs[j].block);
				}
			}
			if (stmt->if_stmt.else_block != NULL)
			{
				fill2(&new_if_sym_table, stmt->if_stmt.else_block);
			}
			LocalSymTable *new_if_local_sym_table = NULL;
			buf_push(new_if_local_sym_table, (LocalSymTable) { new_if_sym_table });
			buf_push(local_sym_tables, new_if_local_sym_table);
		}
		break;
		}
	}
	buf_push(*new_sym_table, (SymTable) { syms });
}

void fill1(LocalSymTable **new_local_sym_table, Stmt *s)
{
	SymTable *new_sym_table = NULL;
	fill2(&new_sym_table, s);
	buf_push(*new_local_sym_table, (LocalSymTable) { new_sym_table });
}

void fill0(Stmt *s)
{
	LocalSymTable *new_local_sym_table = NULL;
	fill1(&new_local_sym_table, s);
	buf_push(local_sym_tables, new_local_sym_table);
}

void dump_sym(Sym s);

void dump_sym_table()
{
	for (size_t i = 0; i < buf_len(local_sym_tables); i++)
	{
		printf("|%d|", i);
		for (size_t j = 0; j < buf_len(local_sym_tables[i]->sym_tables); j++)
		{
			printf("Local table -> ");
			for (size_t k = 0; k < buf_len(local_sym_tables[i]->sym_tables[j].syms); k++)
			{
				dump_sym(local_sym_tables[i]->sym_tables[j].syms[k]);
			}
		}
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
