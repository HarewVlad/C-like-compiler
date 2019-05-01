void type_check_expr(Typespec type, Expr *e)
{
	switch (e->kind)
	{
	case EXPR_INT:
		break;
	case EXPR_DOUBLE:
		if (type == VAR_INT && e->kind == EXPR_DOUBLE)
		{
			e->kind = EXPR_INT;
			e->int_val = (int)floor(e->double_val);
		}
		break;
	case EXPR_NAME:
		break;
	case EXPR_BINARY:
		type_check_expr(type, e->binary.left);
		type_check_expr(type, e->binary.right);
		break;
	default:
		assert(0);
		break;
	}
}

void type_check_stmt(Stmt *s)
{
	switch (s->kind)
	{
	case STMT_DECL:
		type_check_expr(s->decl->var.type, s->decl->var.expr);
		break;
	case STMT_ASIGN:
	{
		Decl *d = get_sym_decl(s->asign.name);
		type_check_expr(d->var.type, s->asign.e);
	}
		break;
	case STMT_BLOCK:
		for (size_t i = 0; i < buf_len(s->block.stmts); i++)
		{
			type_check_stmt(s->block.stmts[i]);
		}
		break;
	case STMT_IF:
		// TODO: check cond.
		if (s->if_stmt.then_block != NULL)
			type_check_stmt(s->if_stmt.then_block);
		if (s->if_stmt.else_ifs != NULL)
		{
			for (size_t i = 0; i < buf_len(s->if_stmt.else_ifs); i++)
			{
				type_check_stmt(s->if_stmt.else_ifs[i].block);
			}
		}
		if (s->if_stmt.else_block != NULL)
		{
			type_check_stmt(s->if_stmt.else_block);
		}
		break;
	default:
		assert(0);
		break;
	}
}
