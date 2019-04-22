
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

typedef struct Operand
{
	Register r;
	Decl *decl;
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
		Register r = alloc_reg();
		Operand *o = new_operand(r, d);
		buf_push(operands, o);
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

Expr *gen_op(Register r, TokenKind op, Expr *left, Expr *right)
{
	int a_value = left->int_val;
	int b_value = right->int_val;

	printf("MOV ");
	dump_reg(r);
	printf(", %d\n", a_value);
	emit_r_i(mov, r, a_value);

	switch (op)
	{
	case '+':
		printf("ADD ");
		dump_reg(r);
		printf(", %d\n", b_value);
		emit_r_i(add, r, b_value);
		return new_expr_int(a_value + b_value);
		break;
	case '-':
		printf("SUB ");
		dump_reg(r);
		printf(", %d\n", b_value);
		emit_r_i(sub, r, b_value);
		return new_expr_int(a_value - b_value);
		break;
	case '*':
		printf("MUL ");
		dump_reg(r);
		printf(", %d\n", b_value);
		// emit_r_i(mul, r, b_value);
		return new_expr_int(a_value * b_value);
		break;
	case '/':
		printf("DIV ");
		dump_reg(r);
		printf(", %d\n", b_value);
		// emit_r_i(div, r, b_value);
		return new_expr_int(a_value / b_value);
		break;
	default:
		assert(0);
		break;
	}
}

Expr *gen_expr(Operand *o, Expr *e)
{
	switch (e->kind)
	{
	case EXPR_INT:
		return e;
		break;
	case EXPR_NAME:
		Decl *d = get_sym(e->name);
		Operand *new_operand = get_operand(d);
		return gen_expr(o, d->var.expr);
		break;
	case EXPR_BINARY:
		Expr *a = gen_expr(o, e->binary.left);
		Expr *b = gen_expr(o, e->binary.right);
		return gen_op(o->r, e->binary.op, a, b);
		break;
	default:
		assert(0);
		break;
	}
}

void gen_decl(Operand *o)
{
	Expr *e = gen_expr(o, o->decl->var.expr);
	printf("MOV ");
	dump_reg(o->r);
	printf(", %d\n", dump_expr_value(e));
}

void gen_asign(Operand *o, Expr *e)
{
	o->decl->var.expr = e;
	gen_decl(o);
}

void gen(Stmt *s)
{
	switch (s->kind)
	{
	case STMT_DECL:
	{
		Operand *o = get_operand(s->decl);
		gen_decl(o);
	}
		break;
	case STMT_BLOCK:
		for (size_t i = 0; i < buf_len(s->block.stmts); i++)
		{
			gen(s->block.stmts[i]);
		}
		break;
	case STMT_ASIGN: // TODO: think about that
	{
		Decl * d = get_sym(s->asign.name);
		Operand *o = get_operand(d);
		gen_asign(o, s->asign.e);
	}
		break;
	default:
		assert(0);
		break;
	}
}