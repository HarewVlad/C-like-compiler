typedef enum SymKind
{
	SYM_UNRESOLVED,
	SYM_RESOLVING,
	SYM_RESOLVED
} SymKind;

typedef struct Sym
{
	SymKind kind;
	Decl *decl;
} Sym;

Sym *global_table;
Sym **local_table;

void fill_symbol_table(Stmt *stmt)
{
	Sym *new_local_table = NULL;
	switch (stmt->kind)
	{
	case STMT_DECL:
		buf_push(global_table, (Sym) { SYM_UNRESOLVED, stmt->decl });
		break;
	case STMT_BLOCK:
		for (size_t i = 0; i < buf_len(stmt->block.stmts); i++)
		{
			switch (stmt->block.stmts[i]->kind)
			{
			case STMT_DECL:
				buf_push(new_local_table, (Sym) { SYM_UNRESOLVED, stmt->block.stmts[i]->decl });
				break;
			case STMT_BLOCK:
				fill_symbol_table(stmt->block.stmts[i]);
				break;
			}
		}
		buf_push(local_table, new_local_table);
		break;
	default:
		assert(0);
		break;
	}
}

Decl *get_sym(const char *name)
{
	Decl *d = NULL;
	for (size_t i = 0; i < buf_len(local_table); i++)
	{
		for (size_t j = 0; j < buf_len(local_table[i]); j++)
		{
			d = local_table[i][j].decl;
			if (strcmp(d->name, name) == 0) // TODO: str interning
			{
				return d;
			}
		}
	}
	return d;
}

// FOR NOW, only resolve local table

typedef enum EntityKind
{
	ENTITY_NONE, // TODO: ENTITY_NULL ???
	ENTITY_INTEGRAL,
	ENTITY_DECL,
	ENTITY_BINARY,
} EntityKind;

typedef struct Entity Entity;

struct Entity
{
	EntityKind kind;
	Typespec type;
	union
	{
		Decl *decl;
		Expr *expr;
		struct
		{
			TokenKind op;
			Entity *left;
			Entity *right;
		};
	};
};

Entity *new_entity(EntityKind kind)
{
	Entity *e = malloc(sizeof(Entity));
	e->kind = kind;
	return e;
}

Entity *new_entity_none()
{
	Entity *e = new_entity(ENTITY_NONE);
	return e;
}

Entity *new_entity_integral(Expr *expr)
{
	Entity *e = new_entity(ENTITY_INTEGRAL);
	e->expr = expr;
	return e;
}

Entity *new_entity_decl(Decl *decl)
{
	Entity *e = new_entity(ENTITY_DECL);
	e->decl = decl;
	return e;
}

Entity *new_entity_binary(TokenKind op, Entity *left, Entity *right)
{
	Entity *e = new_entity(ENTITY_BINARY);
	e->op = op;
	e->left = left;
	e->right = right;
	return e;
}

typedef struct EntityComplete // TODO: mb fuck that shit?
{
	Decl *decl;
	Entity *entity;
} EntityComplete;

EntityComplete **entities;

EntityComplete *new_entity_complete(Decl *decl, Entity *entity)
{
	EntityComplete *e_complete = malloc(sizeof(EntityComplete));
	e_complete->decl = decl;
	e_complete->entity = entity;
	return e_complete;
}

Sym *resolve_name(size_t index, const char *name)
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


void resolve_decl(size_t index_table, Sym *s, Decl *d);

void resolve_expr(size_t index_table, Sym *s, Expr *e)
{
	switch (e->kind)
	{
	case EXPR_INT: case EXPR_CHAR: case EXPR_DOUBLE:
		s->kind = SYM_RESOLVED;
		break;
	case EXPR_NAME:
		s->kind = SYM_RESOLVING;
		Sym *new_s = resolve_name(index_table, e->name);
		if (new_s != NULL)
		{
			resolve_decl(index_table, new_s, new_s->decl);
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

void resolve_decl(size_t index_table, Sym *s, Decl *d)
{
	switch (d->kind)
	{
	case DECL_VAR:
		resolve_expr(index_table, s, d->var.expr);
		break;
	default:
		assert(0);
		break;
	}
}

void resolve_decls()
{
	for (size_t i = 0; i < buf_len(local_table); i++)
	{
		for (size_t j = 0; j < buf_len(local_table[i]); j++)
		{
			Sym s = local_table[i][j];
			Decl *d = local_table[i][j].decl;
			resolve_decl(i, &s, d);
		}
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
			dump_decl(local_table[i][j].decl);
		}
		printf(")");
	}
}
