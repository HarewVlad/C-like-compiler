void resolve_type(Typespec *left, Entity *entity)
{
	switch (entity->kind)
	{
	case ENTITY_INTEGRAL: // TODO: convert method, not just barely do this
		if (*left == VAR_INT && entity->expr->kind == EXPR_DOUBLE)
		{
			entity->expr->kind = EXPR_INT;
			entity->expr->int_val = (int)floor(entity->expr->double_val);
		}
		if (*left == VAR_CHAR && entity->expr->kind == EXPR_DOUBLE)
		{
			entity->expr->kind = EXPR_INT;
			entity->expr->int_val = (int)floor(entity->expr->double_val);
		}
		// ...
		break;
	case ENTITY_DECL:
		// ...
		break;
	case ENTITY_BINARY:
		resolve_type(left, entity->left);
		resolve_type(left, entity->right);
		break;
	case ENTITY_NONE:
		break;
	default:
		assert(0);
		break;
	}
}

void resolve_types()
{
	for (size_t i = 0; i < buf_len(entities); i++)
	{
		resolve_type(&entities[i]->decl->var.type, entities[i]->entity);
	}
}