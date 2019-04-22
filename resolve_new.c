typedef enum SymKind
{
	SYM_UNRESOLVED,
	SYM_RESOLVING,
	SYM_RESOLVED
} SymKind;

typedef struct SymTable
{
	SymKind kind;
	Decl *decl;
} SymTable;

SymTable *global_table;
SymTable **local_table;

void fill_symbol_table(Stmt *stmt)
{
	SymTable *new_local_table = NULL;
	switch (stmt->kind)
	{
	case STMT_DECL:
		buf_push(global_table, (SymTable) { SYM_UNRESOLVED, stmt->decl });
		break;
	case STMT_BLOCK:
		for (size_t i = 0; i < buf_len(stmt->block.stmts); i++)
		{
			switch (stmt->block.stmts[i]->kind)
			{
			case STMT_DECL:
				buf_push(new_local_table, (SymTable) { SYM_UNRESOLVED, stmt->block.stmts[i]->decl });
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

// FOR NOW, only resolve local tables

Decl *resolve_name(size_t index, const char *name)
{
	for (size_t i = index; i < buf_len(local_table); i++)
	{
		for (size_t j = 0; j < buf_len(local_table[i]); j++)
		{
			SymTable t = local_table[i][j];
			Decl *d = t.decl;
			if (strcmp(d->name, name) == 0) // TODO: str interning
			{
				if (t.kind == SYM_RESOLVING)
					fatal("cyclic dependency");

				return d;
			}
		}
	}
	return NULL;
}