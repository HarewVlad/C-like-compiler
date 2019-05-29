
typedef uint8_t Register;
uint32_t free_regs_mask;

void init_regs()
{
	Register free_regs[] = { RAX, RCX, RDX, RBX, RSP, RBP, RSI, RDI, R8, R9, R10, R11, R12, R13, R14, R15 };
	for (size_t i = 0; i < sizeof(free_regs) / sizeof(*free_regs); i++)
	{
		free_regs_mask |= 1 << free_regs[i];
	}
}

Register alloc_reg()
{
	assert(free_regs_mask != 0);
	Register r = 0;
	uint32_t temp = 1;
	while (r != 0xffff)
	{
		if (free_regs_mask & temp)
		{
			free_regs_mask ^= temp;
			return r;
		}
		r++;
		temp <<= 1;
	}
}

void free_reg(Register r)
{
	free_regs_mask |= (1 << r);
}

void dump_reg(Register r)
{
	switch (r)
	{
	case RAX:
		printf("RAX");
		break;
	case RCX:
		printf("RCX");
		break;
	case RDX:
		printf("RDX");
		break;
	case RBX:
		printf("RBX");
		break;
	case RSP:
		printf("RSP");
		break;
	case RBP:
		printf("RBP");
		break;
	case RSI:
		printf("RSI");
		break;
	case RDI:
		printf("RDI");
		break;
	case R8:
		printf("R8");
		break;
	case R9:
		printf("R9");
		break;
	case R10:
		printf("R10");
		break;
	case R11:
		printf("R11");
		break;
	case R12:
		printf("R12");
		break;
	case R13:
		printf("R13");
		break;
	case R14:
		printf("R14");
		break;
	case R15:
		printf("R15");
		break;
	default:
		assert(0);
		break;
	}
}

void dump_op(TokenKind kind)
{
	switch (kind)
	{
	case TOKEN_ADD:
		printf("ADD ");
		break;
	case TOKEN_SUB:
		printf("SUB ");
		break;
	case TOKEN_MUL:
		printf("MUL ");
		break;
	case TOKEN_DIV:
		printf("DIV ");
		break;
	default:
		assert(0);
		break;
	}
}

typedef enum OperandKind
{
	OPERAND_EXPR,
	OPERAND_DECL,
} OperandKind;

typedef struct Operand
{
	Register r;
	OperandKind kind;
	union
	{
		Expr *expr;
		Decl *decl;
	};
} Operand;

Operand **operands;

Operand *get_operand(Decl *d)
{
	for (size_t i = 0; i < buf_len(operands); i++)
	{
		if (operands[i]->decl == d)
			return operands[i];
	}
	return NULL;
}

Operand *new_operand(Register r, Decl *decl)
{
	Operand *o = malloc(sizeof(Operand));
	o->r = r;
	o->decl = decl;
	return o;
}

void install_decl(Decl *d)
{
	switch (d->kind)
	{
	case DECL_VAR:
	{
		Register r = alloc_reg();
		Operand *o = new_operand(r, d);
		buf_push(operands, o);
	}
		break;
	default:
		assert(0);
		break;
	}
}

void install_stmt(Stmt *s)
{
	switch (s->kind)
	{
	case STMT_DECL:
		install_decl(s->decl);
		break;
	default:
		assert(0);
		break;
	}
}

void install_operand(Stmt *s)
{
	switch (s->kind)
	{
	case STMT_DECL:
		install_decl(s->decl);
		break;
	case STMT_BLOCK:
		for (size_t i = 0; i < buf_len(s->block.stmts); i++)
		{
			install_operand(s->block.stmts[i]);
		}
		break;
	case STMT_ASIGN:
		break;
	default:
		assert(0);
		break;
	}
}

Register dump_gen_expr(Expr *e)
{
	switch (e->kind)
	{
	case EXPR_INT:
	{
		Register r = alloc_reg();
		printf("MOV ");
		dump_reg(r);
		printf(", ");
		dump_expr(e);
		printf("\n");
		return r;
	}
	break;
	case EXPR_NAME: // TODO: refactor
	{
		Register r = alloc_reg();
		printf("MOV ");
		dump_reg(r);
		printf(", ");
		Decl *d = get_sym_decl(e->name);
		dump_expr(d->var.expr);
		printf("\n");
		return r;
	}
	break;
	case EXPR_BINARY:
	{
		Register left = dump_gen_expr(e->binary.left);
		Register right = dump_gen_expr(e->binary.right);
		dump_op(e->binary.op);
		dump_reg(left);
		printf(", ");
		dump_reg(right);
		printf("\n");
		free_reg(right);
		return left;
	}
	break;
	default:
		assert(0);
		break;
	}
}

void gen_stmt(Stmt *s)
{
	switch (s->kind)
	{
	case STMT_DECL:
	{
		Register r = dump_gen_expr(s->decl->var.expr);
		Operand *o = new_operand(r, s->decl);
		buf_push(operands, o);
	}
	break;
	case STMT_ASIGN: // TODO: refactor
	{
		Decl *d = get_sym_decl(s->asign.name);
		Operand *o = get_operand(d);
		free_reg(o->r);
		Register r = dump_gen_expr(s->asign.e);
		o->r = r;
	}
	break;
	case STMT_IF:
	{
		Register r = dump_gen_expr(s->if_stmt.cond);
		printf("CMP ");
		dump_reg(r);
		printf(", TRUE\n");
		printf("JNE end\n");
		gen_stmt(s->if_stmt.then_block);
		printf("end: ");
		/*for (size_t i = 0; i < buf_len(s->if_stmt.else_ifs); i++)
		{
			gen_stmt(s->if_stmt.else_ifs->block);
		}
		*/
		if (s->if_stmt.else_block != NULL)
			gen_stmt(s->if_stmt.else_block);
	}
		break;
	case STMT_BLOCK:
		for (size_t i = 0; i < buf_len(s->block.stmts); i++)
		{
			gen_stmt(s->block.stmts[i]);
		}
		break;
	default:
		assert(0);
		break;
	}
}
