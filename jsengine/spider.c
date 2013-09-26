/*
 *  PACrunner JavaScript Engine
 *
 *  Copyright (C) 2013  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms and conditions of the GNU Lesser General Public
 *  License version 2.1 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "tree.h"
#include "jsengine/parser.h"
#include "spider.h"
#include "symbols.h"
#include "math_functions.h"
#include "string_methods.h"
#include "reg_expr_methods.h"
#include "array_methods.h"

struct ast *eval_inherent_function(char *name, struct ast_list *arguments)
{
	struct ast *(*fctn)(struct ast_list *);

	fctn = get_symbol_value(name);

	return (*fctn)(arguments);
}


struct ast *get_typeof(struct ast *tree)
{
	struct ast *item = NULL;
	struct ast *temp_ast = NULL;
	struct ast *retval = jmalloc(sizeof(*retval));
	struct symbol *inner = NULL;
	struct symbol *temp = NULL;

	if (tree == NULL)
		printf("empty\n");

	retval->type = VAR;


	if (tree->type != TYPEOF) {
		fprintf(stderr, "variable is not typeof\n");
		retval->data.string_literal = strdup("undefined");
		retval->next = NULL;
		return retval;
	}

	switch (tree->data.typeof_stmt->type) {
	case VAR:
		inner = get_symbol(tree->data.typeof_stmt->data.var.name);

		if (inner == NULL) {
			fprintf(stderr, "Symbol does not exist...\n");
			return NULL;
		}

		if (inner->value.var.type == NUM)
			retval->data.var.string = strdup("number");
		else if (inner->value.var.type == STRING_LITERAL)
			retval->data.var.string = strdup("string");
		else if (inner->value.declared_func->type == FUNCTION_DEC)
			retval->data.var.string = strdup("function");
		else if (inner->value.declared_func->type == ARRAY)
			retval->data.var.string = strdup("object");
		else
			printf("Type: %d Name: %s\n", inner->type, inner->name);
	break;
	case ARRAY_ITEM:
		item = eval_array_item(
				tree->data.typeof_stmt->data.arr_item.ast_index,
				temp_ast, temp, tree->data.typeof_stmt);
		if (item->data.var.type == STRING_LITERAL)
			retval->data.var.string = strdup("string");
		else if (item->data.var.type == NUM)
			retval->data.var.string = strdup("number");
		else if (item->data.var.type == BOOL)
			retval->data.var.string = strdup("boolean");
	break;
	case BOOL:
		retval->data.string_literal = strdup("boolean");
	case NUM:
		retval->data.string_literal = strdup("number");
	break;
	case STRING_LITERAL:
		retval->data.string_literal = strdup("string");
	break;
	default:
		retval->data.string_literal = strdup("undefined");
	}

	retval->data.var.type = STRING_LITERAL;
	retval->next = NULL;

	return retval;
}

struct ast *eval_init(struct ast *tree)
{
	struct ast *ret_val;

	add_symbol(tree->data.var.name, VAR_NUM);

	ret_val = jmalloc(sizeof(*ret_val));
	ret_val->type = VAR_INIT;
	ret_val->data.var.name = tree->data.var.name;
	ret_val->next = NULL;

	return ret_val;
}

double do_op(double left_val, double right_val, int op)
{
	switch (op) {
	case PLUS:
		return left_val + right_val;
	case MINUS:
		return left_val - right_val;
	case MULT:
		return left_val * right_val;
	case DIV:
		return left_val / right_val;
	case BITAND:
		return (int) floor(left_val) & (int) floor(right_val);
	case BITOR:
		return (int) floor(left_val) | (int) floor(right_val);
	case BITXOR:
		return (int) floor(left_val) ^ (int) floor(right_val);
	case MOD:
		return (int) floor(left_val) % (int) floor(right_val);
	case RSHIFT:
		return (int) floor(left_val) >> (int) floor(right_val);
	case LSHIFT:
		return (int) floor(left_val) << (int) floor(right_val);
	case LS:
		return (left_val < right_val ? 1 : 0);
	case GTR:
		return (left_val > right_val ? 1 : 0);
	case NEQL:
		return (left_val != right_val ? 1 : 0);
	case EQL:
		return (left_val == right_val ? 1 : 0);
	default:
		printf("error operator not recognized\n");
		return 0;
	}
}

void eval_var_str(struct ast *retval, struct symbol *sym)
{
	if (sym->value.var.type == STRING_LITERAL) {
		retval->data.var.type = STRING_LITERAL;
		if (sym->value.var.str != NULL)
			retval->data.var.string =
					strdup(sym->value.var.str);
		else
			retval->data.var.string = NULL;
	} else {
		retval->data.var.type = BOOL;
		retval->data.var.val = sym->value.var.val;
		retval->data.var.string = NULL;
		if (sym->value.var.str != NULL)
			retval->data.var.string =
					strdup(sym->value.var.str);
	}
}

static struct ast *eval_var(char *name)
{

	struct ast *retval = jmalloc(sizeof(*retval));
	struct symbol *temp = get_symbol(name);
	struct ast *temp_ast;

	retval->type = VAR;

	if (temp == NULL) {
		fprintf(stderr, "Did not get symbol\n");
		return NULL;
	}

	switch (temp->type) {
	case VAR_NUM:
		retval->data.var.type = NUM;
		retval->data.var.val = *(double *) get_symbol_value(name);
	break;
	case VAR_STRING:
		eval_var_str(retval, temp);
	break;
	case ARRAY:
		free(retval);
		retval = set_return_array(temp->value.array);
	break;
	case VAR_OBJECT:
		temp_ast = get_prop_value(name, "value");
		if (temp_ast->data.var.type == STRING_LITERAL ||
			temp_ast->data.var.type == NaN ||
			temp_ast->type == STRING_LITERAL) {
			retval->data.var.type = STRING_LITERAL;
			retval->data.var.string = strdup(
					temp_ast->data.var.string);
		} else if (temp_ast->data.var.type == NUM) {
			retval->data.var.type = NUM;
			retval->data.var.val = temp_ast->data.var.val;
		} else {
			printf("retval not recognized\n");
		}
		free_expr(temp_ast);
	break;
	case NULL_TYPE:
		retval->type = NULL_TYPE;
	break;
	case TYPEOF:
		printf("evl_var() typeof\n");
	break;
	default:
		fprintf(stderr, "Incompatable var type\n");
	break;
	}
	return retval;
}

static struct ast *eval_num(double num)
{
	struct ast *retval = jmalloc(sizeof(*retval));

	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = num;
	retval->data.var.name = NULL;
	retval->data.var.string = NULL;

	return retval;
}

struct ast *eval_bool(char *string, double val)
{
	struct ast *retval = jmalloc(sizeof(*retval));

	retval->type = VAR;
	retval->data.var.type = BOOL;
	retval->data.var.string = strdup(string);
	retval->data.var.val = val;

	return retval;
}

struct ast *eval_binary(char *name, struct ast *tree,
		struct ast *templeft, struct ast *tempright)
{
	struct ast *retval = jmalloc(sizeof(*retval));

	retval->type = VAR;
	retval->data.var.type = NUM;
	templeft = eval_expr(tree->data.bin.left, name);
	tempright = eval_expr(tree->data.bin.right, name);

	if (templeft->data.var.type == STRING_LITERAL) {
		retval->data.var.type = STRING_LITERAL;
		retval->data.var.string = do_string_add(
			templeft->data.var.string,
			tempright->data.var.string,
			tree->data.bin.op);
	} else {
		retval->data.var.val = do_op(templeft->data.var.val,
					tempright->data.var.val,
					tree->data.bin.op);
		retval->data.var.type = NUM;
	}

	free_expr(templeft);
	free_expr(tempright);

	return retval;
}

struct ast *eval_string_literal(char *string)
{
	struct ast *retval = jmalloc(sizeof(*retval));

	retval->type = VAR;
	retval->data.var.type = STRING_LITERAL;
	retval->data.var.string = strdup(string);

	return retval;
}

struct ast *eval_array_item(struct ast *index, struct ast *temp_ast,
				struct symbol *temp, struct ast *tree)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	struct ast *eval = eval_expr(index, NULL);
	int arr_index = (int) eval->data.var.val;
	retval->type = VAR;
	temp = get_symbol(tree->data.arr_item.name);
	temp_ast = temp->value.array->data.array.list[arr_index];
	copy_array_element(retval, temp_ast);

	return retval;
}

struct ast *eval_expr(struct ast *tree, char *name)
{
	struct ast *retval = NULL;
	struct ast *temp_ast = NULL;
	struct ast *templeft = NULL;
	struct ast *tempright = NULL;
	struct symbol *temp = NULL;

	if (tree == NULL) {
		retval = jmalloc(sizeof(*retval));
		retval->type = NULL_TYPE;
		return retval;
	}

	switch (tree->type) {
	case VAR:
		return eval_var(tree->data.var.name);
	case NUM:
		return eval_num(tree->data.number);
	case BOOL:
		return eval_bool(tree->data.var.string,
					tree->data.var.val);
	case BINARY:
		return eval_binary(name, tree, templeft, tempright);
	case FUNCTION:
		retval = eval_inherent_function(tree->data.func.name,
					tree->data.func.parameters);
		return retval;
	case CALL_FUNC:
		retval = eval_func(get_declared_func(
					tree->data.func.name),
					tree->data.func.parameters);
		return retval;
	case STRING_LITERAL:
		return eval_string_literal(tree->data.string_literal);
	case CALL_METHOD:
		retval = eval_method(tree->data.func.obj,
					tree->data.func.name,
					tree->data.func.parameters);
		return retval;
	case ARRAY_ITEM:
		return eval_array_item(tree->data.arr_item.ast_index,
					temp_ast, temp, tree);
	case ARRAY:
		printf("expression is an array\n");
	break;
	case QUESTION:
		retval = eval_cond_operator(tree, name);
		return retval;
	break;
	case OBJ_PROP:
		if (tree->data.func.obj == NULL)
			retval = get_prop_value(name, tree->data.func.name);
		else
			retval = get_prop_value(tree->data.func.obj,
						tree->data.func.name);

		return retval;
	break;
	case NULL_TYPE:
		retval = jmalloc(sizeof(*retval));
		retval->type = NULL_TYPE;

		return retval;
	break;
	case REG_EXP:
		retval = jmalloc(sizeof(*retval));
		retval->type = REG_EXP;
		retval->data.var.string = strdup(tree->data.var.string);

		return retval;
	break;
	case CALL_REG:
		retval = eval_reg_method(tree->data.func.obj,
					tree->data.func.name,
					tree->data.func.parameters);
		return retval;
	break;
	case UNARY:
		return eval_not_unary(tree, name);
	case TYPEOF:
		return get_typeof(tree);
	break;
	default:
		printf("expr not recognized\n");
	break;
	}

	return retval;
}

/*assumes it is passed a var*/

double eval_bitnot(struct ast *tree)
{
	switch (tree->data.var.type) {
	case BOOL:
	case NUM:
		if (tree->data.var.val != 0)
			return 0;
		return 1;
	case STRING_LITERAL:
		if (tree->data.var.string != NULL)
			return 0;
		return 1;
	default:
		printf("ERROR IN EVAL BITNOT\n");
	break;
	}
	return 0;
}

struct ast *eval_not_unary(struct ast *tree, char *obj)
{
	struct ast *left = eval_expr(tree->data.bin.left, obj);
	struct ast *retval = jmalloc(sizeof(*retval));

	retval->type = VAR;
	switch (tree->data.bin.op) {
	case NOT:
		retval->data.var.type = BOOL;
		retval->data.var.val = eval_bitnot(left);
	break;
	default:
		printf("This unary is not supported yet\n");
	break;
	}

	free_expr(left);

	return retval;
}

struct ast *eval_reg_method(char *regex, char *method, struct ast_list *args)
{
	struct ast *retval;
	char *name = strdup("__temp__");
	add_new_reg(name, regex);
	retval = eval_method(name, method, args);
	remove_sym(name);
	free(name);

	return retval;
}

void copy_array_element(struct ast *dest, struct ast *source)
{
	dest->type = VAR;
	char *src_str;
	switch (source->type) {
	case VAR:
		if (source->data.var.type == NUM) {
			dest->data.var.type = NUM;
			dest->data.var.val = source->data.var.val;
		} else {
			dest->data.var.type = STRING_LITERAL;
			src_str = source->data.var.string;
			dest->data.var.string = strdup(src_str);
		}
	break;
	case STRING_LITERAL:
		dest->data.var.type = STRING_LITERAL;
		src_str = source->data.string_literal;
		dest->data.var.string = strdup(src_str);
	break;
	case NUM:
		dest->data.var.type = NUM;
		dest->data.var.val = source->data.var.val;
	break;
	default:
		printf("cannot copy this element yet\n");
	break;
	}
	dest->next = NULL;
}

struct ast *eval_cond_operator(struct ast *tree, char *name)
{
	struct ast *cur;
	struct ast *retval;

	if (eval_cond(tree->data.ifblock->cond)) {
		cur = tree->data.ifblock->block->top;
		while (cur->next != NULL) {
			retval = eval_statement(cur, name);
			if (retval->type == RETURN)
				return retval;
			free_expr(retval);
			cur = cur->next;
		}

		return eval_expr(cur, name);
	} else {
		cur = tree->data.ifblock->else_block->top;
		while (cur->next != NULL) {
			retval = eval_statement(cur, name);
			if (retval->type == RETURN)
				return retval;
			free_expr(retval);
			cur = cur->next;
		}

		return eval_expr(cur, name);
	}
}

char *do_string_add(char *str_one, char *str_two, int op)
{
	if (op != PLUS)
		return NULL;

	char *ret_str = (char *) jmalloc(sizeof(*ret_str) * (strlen(str_one) +
					 strlen(str_two)));
	strcpy(ret_str, str_one);
	strcat(ret_str, str_two);

	return ret_str;
}

struct ast *get_prop_value(char *obj_name, char *prop_name)
{
	struct symbol *temp;
	struct symbol *temp_obj;
	struct symbol *wrap;
	struct ast *retval;

	wrap = NULL;
	retval = jmalloc(sizeof(*retval));
	temp_obj = get_symbol(obj_name);
	if (temp_obj == NULL)
		printf("obj null\n");

	if (temp_obj->type != VAR_OBJECT) {
		wrap = wrap_prim(temp_obj);
		temp = get_property(wrap, prop_name);
	} else {
		temp = get_property(temp_obj, prop_name);
	}

	if (temp == NULL)
		printf("could not get prop\n");

	retval->type = VAR;

	if (temp->type == VAR_STRING) {
		retval->data.var.type = STRING_LITERAL;
		retval->data.var.string = strdup(temp->value.var.str);
	} else if (temp->type == VAR_NUM) {
		retval->data.var.type = NUM;
		retval->data.var.val = temp->value.var.val;
	} else if (temp->type == NULL_TYPE) {
		retval->type = NULL_TYPE;
	} else {
		printf("type is somthing else\n");
	}

	if (wrap != NULL)
		free_sym(wrap);

	retval->next = NULL;

	return retval;
}

int eval_conditional_expr(struct ast *tree)
{
	struct ast *expr = eval_expr(tree->data.bin.left, NULL);

	switch (expr->data.var.type) {
	case NUM:
		if (expr->data.var.val > 0)
			return 1;
	break;
	case BOOL:
		return expr->data.var.val;
	case STRING_LITERAL:
		return 1;
	}

	return 0;
}

int eval_cond(struct ast *tree)
{
	struct ast *left_val = NULL;
	struct ast *right_val = NULL;


	if (tree->type != BINARY) {
		left_val = eval_expr(tree, NULL);
		switch (left_val->type) {
		case VAR:
		if (left_val->data.var.type ==
					STRING_LITERAL) {
			if (left_val->data.var.string != NULL) {
				free_expr(left_val);
				return 1;
			} else {
				free_expr(left_val);
				return 0;
			}
		} else {
			if (left_val->data.var.val != 0) {
				free_expr(left_val);
				return 1;
			}
			free_expr(left_val);
			return 0;
		}
		break;
		default:
			printf("this cond expr not supported yet\n");
		break;
		}
		free_expr(left_val);
	}

	switch (tree->data.bin.op) {
	case AND:
		if (eval_cond(tree->data.bin.left) &&
					eval_cond(tree->data.bin.right))
			return 1;
		return 0;
	case OR:
		if (eval_cond(tree->data.bin.left) ||
					eval_cond(tree->data.bin.right))
			return 1;
		return 0;
	case NOT:
		if (!eval_cond(tree->data.bin.left))
			return 1;
		return 0;
	case EQL:
		left_val = eval_expr(tree->data.bin.left, NULL);
		right_val = eval_expr(tree->data.bin.right, NULL);
		if (left_val->type == NULL_TYPE && right_val->type ==
								    NULL_TYPE) {
			free_expr(left_val);
			free_expr(right_val);
			return 1;
		} else if (left_val->data.var.type == STRING_LITERAL &&
			right_val->data.var.type == STRING_LITERAL) {
			if (strcmp(left_val->data.var.string,
					right_val->data.var.string) == 0) {
				free_expr(left_val);
				free_expr(right_val);
				return 1;
			}
		} else if (left_val->data.var.val ==
						right_val->data.var.val) {
			free_expr(left_val);
			free_expr(right_val);
			return 1;
		}
	break;
	case GTR:
		left_val = eval_expr(tree->data.bin.left, NULL);
		right_val = eval_expr(tree->data.bin.right, NULL);
		if (left_val->data.var.val > right_val->data.var.val) {
			free_expr(left_val);
			free_expr(right_val);
			return 1;
		}
	break;
	case LS:
		left_val = eval_expr(tree->data.bin.left, NULL);
		right_val = eval_expr(tree->data.bin.right, NULL);
		if (left_val->data.var.val < right_val->data.var.val) {
			free_expr(left_val);
			free_expr(right_val);
			return 1;
		}
	break;
	case GEQL:
		left_val = eval_expr(tree->data.bin.left, NULL);
		right_val = eval_expr(tree->data.bin.right, NULL);
		if (left_val->data.var.val >= right_val->data.var.val) {
			free_expr(left_val);
			free_expr(right_val);
			return 1;
		}
	break;
	case LEQL:
		left_val = eval_expr(tree->data.bin.left, NULL);
		right_val = eval_expr(tree->data.bin.right, NULL);
		if (left_val->data.var.val <= right_val->data.var.val) {
			free_expr(left_val);
			free_expr(right_val);
			return 1;
		}
	break;
	case NEQL:
		left_val = eval_expr(tree->data.bin.left, NULL);
		right_val = eval_expr(tree->data.bin.right, NULL);

		if (left_val->type == NULL_TYPE ||
						 right_val->type == NULL_TYPE) {
			if (left_val->type != right_val->type) {
				free_expr(left_val);
				free_expr(right_val);
				return 1;
			}
		} else if (left_val->data.var.type == STRING_LITERAL &&
			right_val->data.var.type == STRING_LITERAL) {
			if (strcmp(left_val->data.var.string,
					right_val->data.var.string) != 0) {
				free_expr(left_val);
				free_expr(right_val);
				return 1;
			}
		} else if ((int) left_val->data.var.val !=
						(int) right_val->data.var.val) {
			free_expr(left_val);
			free_expr(right_val);
			return 1;
		}
	break;
	case EXPR:
		return eval_conditional_expr(tree);
	break;
	default:
	break;
	}

	if (left_val != NULL)
		free_expr(left_val);
	if (right_val != NULL)
		free_expr(right_val);

	return 0;
}

struct ast *eval_loop(struct ast *tree)
{
	struct ast *return_val, *temp;

	return_val = NULL;
	switch (tree->data.ifblock->loop) {
	case IF:
		if (eval_cond(tree->data.ifblock->cond)) {
			return_val = statements_spider(
						tree->data.ifblock->block);
		}
		if (return_val != NULL) {
			if (return_val->type == RETURN)
				return return_val;
			else
				free_expr(return_val);
		}
	break;
	case WHILE:
		while (eval_cond(tree->data.ifblock->cond)) {
			return_val = statements_spider(
						tree->data.ifblock->block);
			if (return_val->type == RETURN)
				return return_val;
			else
				free_expr(return_val);
		}
	break;
	case ELSE_IF:
		if (eval_cond(tree->data.ifblock->cond)) {
			return_val = statements_spider(
						tree->data.ifblock->block);

			return return_val;
		}

		temp = tree->data.ifblock->else_if->top;
		while (temp != NULL &&
					!eval_cond(temp->data.ifblock->cond)) {
			temp = temp->next;
		}

		if (temp != NULL) {
			return_val = statements_spider(
						temp->data.ifblock->block);

			return return_val;
		}

		if (tree->data.ifblock->else_block != NULL) {
			return_val = statements_spider(
						tree->data.ifblock->else_block);

			return return_val;
		}
	break;
	case ELSE:
		if (eval_cond(tree->data.ifblock->cond))
			return_val = statements_spider(
						tree->data.ifblock->block);
		else
			return_val = statements_spider(
						tree->data.ifblock->else_block);

		if (return_val->type == RETURN)
			return return_val;
		else
			free_expr(return_val);
	break;
	case DO:
		do {
			return_val = statements_spider(
						tree->data.ifblock->block);
			if (return_val->type == RETURN)
				return return_val;
			else
				free_expr(return_val);
		} while (eval_cond(tree->data.ifblock->cond));
		return_val = jmalloc(sizeof(*return_val));
		return_val->type = NUM;
		return return_val;
	break;
	case FOR:
		eval_initialization(tree->data.ifblock->assign);
		while (eval_cond(tree->data.ifblock->cond)) {
			return_val = statements_spider(
						tree->data.ifblock->block);
			if (return_val->type == RETURN)
				return return_val;
			else
				free_expr(return_val);

			temp = eval_statement(tree->data.ifblock->stmt, NULL);
			free_expr(temp);
		}
	break;
	default:
		printf("loop not recognized\n");
	}

	return_val = jmalloc(sizeof(*return_val));
	return_val->type = NUM;

	return return_val;
}



void eval_initialization(struct ast *init)
{
	struct ast *temp;

	switch (init->type) {
		case SEMICOLON:
		break;
		case BINARY:
			if (is_assign(init->data.bin.op)) {
				temp = eval_assign(init, NULL);
				free_stmt(temp);
			} else {
				temp = eval_expr(init, NULL);
				free_expr(temp);
			}
		break;
		default:
			temp = eval_expr(init, NULL);
			free_expr(temp);
		break;
	}
}

struct ast *eval_func_dec(struct ast *tree)
{
	struct ast *return_val;

	add_new_dec_function(tree);
	return_val = jmalloc(sizeof(*return_val));
	return_val->type = VAR_INIT;
	return_val->data.var.name = strdup(tree->data.func.name);
	return_val->next = NULL;

	return return_val;
}

struct ast *array_dec(struct ast *tree)
{
	struct ast *ret_val;

	ret_val = jmalloc(sizeof(*ret_val));
	ret_val->type = VAR_INIT;
	ret_val->data.var.name = strdup(tree->name);
	ret_val->next = NULL;

	return ret_val;
}

struct ast *eval_statement(struct ast *tree, char *name)
{
	struct ast *return_val;

	switch (tree->type) {
	case BINARY:
		return_val = eval_assign(tree, name);
		return return_val;
	break;
	case UNARY:
		eval_unary(tree);
	break;
	case VAR_INIT:
		return_val = eval_init(tree);
		return return_val;
	break;
	case LOOP:
		return_val = eval_loop(tree);
		return return_val;
	case RETURN:
		return_val = eval_return(tree);
		return return_val;
	case TYPEOF:
		get_typeof(tree);
	break;
	case FUNCTION_DEC:
		return_val = eval_func_dec(tree);
		return return_val;
	break;
	case CALL_FUNC:
		eval_void_func(get_declared_func(tree->data.func.name),
				tree->data.func.parameters);
	break;
	case ARRAY:
		add_array(tree->name, tree);
		return array_dec(tree);
	break;
	case FUNCTION:
		eval_void_inherent_function(tree->data.func.name,
			tree->data.func.parameters, name);
	break;
	case NEW:
		return_val = eval_init_obj(tree->data.bin.left,
						tree->data.bin.right);
		return return_val;
	break;
	case METHOD_DEC:
		eval_init_obj_method(tree);
	break;
	case CALL_METHOD:
		eval_void_method(tree->data.func.obj,
					tree->data.func.name,
					tree->data.func.parameters);
	break;
	case SWITCH:
		return_val = eval_switch(tree->data.ifblock->cond,
					 tree->data.ifblock->block);
		return return_val;
	break;
	case BREAK:
		return_val = jmalloc(sizeof(*return_val));
		return_val->type = BREAK;
		return return_val;
	break;
	case QUESTION:
		return_val = eval_cond_statement(tree, name);
		return return_val;
	break;
	default:
		printf("statement not recognized %d %d\n", tree->type,
								LOOP);
	break;
	}

	return_val = jmalloc(sizeof(struct ast));
	return_val->type = VAR;

	return return_val;
}

struct ast *eval_cond_statement(struct ast *tree, char *name)
{
	struct ast *retval;

	if (eval_cond(tree->data.ifblock->cond))
		retval = statements_spider_func(tree->data.ifblock->block,
						name);
	else
		retval = statements_spider_func(tree->data.ifblock->else_block,
						name);

	return retval;
}

struct ast *eval_switch(struct ast *expr, struct ast_list *case_list)
{
	struct ast *temp;
	struct ast *temp_two;
	struct ast *current_case;
	struct ast *retval;

	current_case = case_list->top;
	temp = eval_expr(expr, NULL);

	while (current_case != NULL || current_case->type != DEFAULT) {
		temp_two = eval_expr(current_case->data.ifblock->cond, NULL);
		if (compare_expr(temp, temp_two))
			break;

		free_expr(temp_two);
		current_case = current_case->next;
	}

	retval = jmalloc(sizeof(*retval));
	if (current_case == NULL) {
		retval->type = NUM;
		return retval;
	}

	retval->type = VAR;
	while (current_case != NULL) {
		if (retval != NULL) {
			free_expr(retval);
			retval = NULL;
		}

		if (current_case->data.ifblock->block != NULL) {
			retval = statements_spider(
				current_case->data.ifblock->block);
			if (retval->type == RETURN ||
						retval->type == BREAK) {
				if (retval->type == BREAK)
					retval->type = NUM;
				return retval;
			}
		}
		current_case = current_case->next;
	}

	return retval;
}

int compare_expr(struct ast *one, struct ast *two)
{
	if (one->type != two->type)
		return 0;

	if (one->data.var.type != two->data.var.type)
		return 0;

	if (one->data.var.type == NUM) {
		if (one->data.var.val == two->data.var.val)
			return 1;
		return 0;
	} else if (one->data.var.type == STRING_LITERAL) {
		if (strcmp(one->data.var.string, two->data.var.string) == 0)
			return 1;
		return 0;
	} else {
		printf("compare expressions not supported\n");
	}
	return 0;
}

void eval_init_obj_method(struct ast *tree)
{
	add_dec_obj_method(tree->data.func.obj, tree->data.func.name,
			tree->data.func.parameters, tree->data.func.statements);
}

struct ast *eval_init_obj(struct ast *new_obj, struct ast *constructor)
{
	struct ast *retval = jmalloc(sizeof(*retval));

	retval->type = VAR_INIT;
	retval->data.var.name = strdup(new_obj->data.var.name);
	add_object(new_obj->data.var.name, constructor->data.func.name);

	eval_prototype(constructor, new_obj->data.var.name);

	return retval;
}

void eval_constructor(struct ast *constructor, char *name)
{
	struct ast *temp, *retval;

	temp = get_declared_func(constructor->data.func.name);
	add_parameter_vars(temp->data.func.parameters,
				constructor->data.func.parameters);
	if (temp->data.func.statements == NULL) {
		fprintf(stderr, "statements null\n");
		return;
	}

	retval = statements_spider_func(temp->data.func.statements, name);
	free_expr(retval);
	remove_parameter_vars(temp->data.func.parameters);
}


struct ast *statements_spider(struct ast_list *list)
{
	struct ast *cur = NULL;
	struct ast *return_val;

	if (list != NULL && list->top != NULL)
		cur = list->top;

	while (cur != NULL) {
		return_val = eval_statement(cur, NULL);
		if (return_val->type == RETURN || return_val->type == BREAK)
			return return_val;

		free_expr(return_val);
		cur = cur->next;
	}

	return_val = jmalloc(sizeof(*return_val));
	return_val->type = NUM;

	return return_val;
}

struct ast *statements_spider_func(struct ast_list *list, char *name)
{
	struct ast *cur = NULL;
	struct ast_list *scope;

	scope = create_scope();

	if (list != NULL && list->top != NULL)
		cur = list->top;
	struct ast *return_val;

	while (cur != NULL) {
		return_val = eval_statement(cur, name);
		if (return_val->type == RETURN) {
			free_scope(scope);

			return return_val;
		} else if (return_val->type == VAR_INIT) {
			scope_add_sym(scope, return_val);
		}
		free_expr(return_val);
		cur = cur->next;
	}

	return_val = jmalloc(sizeof(*return_val));
	return_val->type = NUM;
	free_scope(scope);

	return return_val;
}

struct ast_list *create_scope(void)
{
	struct ast_list *temp;
	temp = (struct ast_list *) jmalloc(sizeof(*temp));
	temp->top = NULL;

	return temp;
}

void scope_add_sym(struct ast_list *scope, struct ast *sym)
{
	struct ast *new_sym;

	new_sym = jmalloc(sizeof(*new_sym));
	new_sym->type = VAR;
	new_sym->data.var.name = strdup(sym->data.var.name);
	new_sym->next = scope->top;
	scope->top = new_sym;
}

void free_scope(struct ast_list *scope)
{
	struct ast *temp, *temp_next;
	temp = scope->top;
	while (temp != NULL) {
		temp_next = temp->next;
		remove_sym(temp->data.var.name);
		free_expr(temp);
		temp = temp_next;
	}

	free(scope);
}

void add_parameter_vars(struct ast_list *var_name, struct ast_list *var_val)
{
	if (var_name == NULL || var_val == NULL)
		return;
	if (var_name == NULL && var_val != NULL) {
			printf("ERROR: VOID FUNCTION CALLED WITH ARGS");
			return;
	}

	struct ast *temp_name;
	struct ast *temp_val;
	struct ast *temp;
	temp_name = var_name->top;
	temp_val = var_val->top;

	while (temp_name != NULL) {
		temp = eval_expr(temp_val, NULL);
		if (temp->data.var.type == NUM) {
			add_symbol(temp_name->data.var.name, 0);
			assign_var_num(temp_name->data.var.name,
					temp->data.var.val);
		} else {
			add_symbol(temp_name->data.var.name, 1);
			assign_var_str(temp_name->data.var.name,
						temp->data.var.string, 0);
		}

		temp_name = temp_name->next;
		temp_val = temp_val->next;
		free_expr(temp);
	}
	/*
	free(temp_name);
	free(temp_val);
	free(temp);*/
}

void remove_parameter_vars(struct ast_list *parameters)
{
	if (parameters == NULL)
		return;

	struct ast *temp;

	temp = parameters->top;

	while (temp != NULL) {
		remove_sym(temp->data.var.name);
		temp = temp->next;
	}
}

struct ast *eval_func(struct ast *tree, struct ast_list *parameters)
{
	struct ast *val, *temp;

	add_parameter_vars(tree->data.func.parameters, parameters);

	val = statements_spider_func(tree->data.func.statements, NULL);

	remove_parameter_vars(tree->data.func.parameters);

	temp = val->data.return_val;

	free(val);

	return temp;
}

void eval_void_inherent_function(char *name, struct ast_list *arguments,
								char *obj_name)
{
	void(*fctn)(struct ast_list *, char *);

	fctn = get_symbol_value(name);
	(*fctn)(arguments, obj_name);
}

void eval_void_func(struct ast *tree, struct ast_list *parameters)
{
	add_parameter_vars(tree->data.func.parameters, parameters);

	(void) statements_spider_func(tree->data.func.statements, NULL);

	remove_parameter_vars(tree->data.func.parameters);
}

struct ast *eval_method(char *obj, char *method_name,
						struct ast_list *parameters)
{
	struct object_method *method;
	struct ast *retval;

	method = get_obj_method(obj, method_name);
	if (method->fctn_ptr != NULL) {
		struct ast *(*fctn)(char *, struct ast_list *);
		fctn = method->fctn_ptr;
		retval = (*fctn)(obj, parameters);
		return retval;
	} else {
		struct ast *val, *temp;
		add_parameter_vars(method->args, parameters);

		val = statements_spider_func(method->statements, obj);

		remove_parameter_vars(method->args);

		temp = val->data.return_val;
		free(val);
		return temp;
	}
}

void eval_void_method(char *obj, char *method_name, struct ast_list *parameters)
{
	struct object_method *method;

	method = get_obj_method(obj, method_name);

	if (method->fctn_ptr != NULL) {
		struct ast *(*fctn)(char *, struct ast_list *);
		fctn = method->fctn_ptr;
		(*fctn)(obj, parameters);
		return;
	} else {
		struct ast *val;
		add_parameter_vars(method->args, parameters);

		val = statements_spider_func(method->statements, obj);

		remove_parameter_vars(method->args);

		free_expr(val);
	}
}

static struct ast *eval_assign_varinit(char *var_name, struct ast *left,
					struct ast *right,
					struct ast *new_sym)
{
	switch(right->type) {
	case ARRAY:
		add_symbol(var_name, ARRAY);
		assign_var_array(var_name, right);
	break;
	case NULL_TYPE:
		add_symbol(var_name, VAR_NUM);
		assign_var_null(var_name);
	break;
	case REG_EXP:
		add_new_reg(var_name, right->data.var.string);
	break;
	}

	/* Handle inner var types */
	switch (right->data.var.type) {
	case NUM:
		add_symbol(var_name, VAR_NUM);
		assign_var_num(var_name, right->data.var.val);
	break;
	case STRING_LITERAL:
	case NaN:
		add_symbol(var_name, VAR_STRING);
		assign_var_str(var_name, right->data.var.string, 0);
	break;
	case BOOL:
		add_symbol(var_name, BOOL);
		assign_var_bool(var_name, right->data.var.val);
	break;
	}

	free_expr(right);
	new_sym->type = VAR_INIT;
	new_sym->data.var.name = var_name;
	new_sym->next = NULL;
	return new_sym;
}

struct ast *eval_assign(struct ast *tree, char *name)
{
	struct ast *right, *left;
	struct ast *new_sym;
	struct symbol *temp_sym;
	char *str = NULL;
	char *obj_name = NULL;
	char *var_name = NULL;

	left = tree->data.bin.left;
	new_sym = jmalloc(sizeof(*new_sym));

	if (tree->data.bin.right->type != ARRAY)
		right = eval_expr(tree->data.bin.right, name);
	else if (tree->data.bin.right->type == ARRAY)
		right = tree->data.bin.right;

	if (left->type != OBJ_PROP && left->type != ARRAY_ITEM)
		var_name = strdup(left->data.var.name);
	else if (left->data.func.obj == NULL)
		obj_name = name;
	else
		obj_name = left->data.func.obj;

	if (left->type == ARRAY_ITEM)
		var_name = strdup(left->data.arr_item.name);

	if (left->type == VAR_INIT)
		return eval_assign_varinit(var_name, left, right, new_sym);

	switch (tree->data.bin.op) {
	case ASSIGN:
		if (var_name != NULL) {
			switch(right->type) {
			case NULL_TYPE:
				assign_var_null(var_name);
			break;
			case ARRAY:
				assign_var_array(var_name, right);
			break;
			case REG_EXP:
				add_symbol(var_name, REG_EXP);
				assign_var_reg(var_name,
						right->data.var.string);
			break;
			}

			/* Handle inner var types */
			switch(right->data.var.type) {
			case NUM:
				assign_var_num(var_name,
						right->data.var.val);
			break;
			case STRING_LITERAL:
				assign_var_str(var_name,
					right->data.var.string,
					left->data.arr_item.index);
			break;
			case BOOL:
				assign_var_bool(var_name,
						right->data.var.val);
			break;
			}
		} else {
			if (right->type == NULL_TYPE)
				assign_prop_null(obj_name,
						left->data.func.name);
			else if (right->data.var.type == NUM) {
				assign_prop_num(obj_name,
						left->data.func.name,
						right->data.var.val);
			} else if (right->data.var.type ==
						STRING_LITERAL) {
				assign_prop_str(obj_name,
						left->data.func.name,
						right->data.var.string);
			}
		}
	break;
	case PLUS_ASSIGN:
		temp_sym = get_symbol(var_name);
		if (temp_sym->type == VAR_NUM) {
			assign_var_num(var_name, *(double *)
					get_symbol_value(var_name) +
					right->data.var.val);
		} else {
			str = do_string_add(temp_sym->value.var.str,
				right->data.var.string, PLUS);
			assign_var_str(var_name, str, 0);
		}
	break;
	case MINUS_ASSIGN:
		assign_var_num(var_name, *(double *)
					get_symbol_value(var_name) -
					right->data.var.val);
	break;
	case MULT_ASSIGN:
		assign_var_num(var_name, (*(double *)
				get_symbol_value(var_name) *
				right->data.var.val));
	break;
	case DIV_ASSIGN:
		assign_var_num(var_name, (*(double *)
					get_symbol_value(var_name) /
					right->data.var.val));
	break;
	case MOD_ASSIGN:
		assign_var_num(var_name, ((int) floor(*(double *)
				get_symbol_value(var_name)) %
				(int) floor(right->data.var.val)));
	break;
	case AND_ASSIGN:
		assign_var_num(var_name, ((int) floor(*(double *)
				get_symbol_value(var_name)) &
				(int) floor(right->data.var.val)));
	break;
	case OR_ASSIGN:
		assign_var_num(var_name, ((int) floor(*(double *)
				get_symbol_value(var_name)) |
				(int) floor(right->data.var.val)));
	break;
	case XOR_ASSIGN:
		assign_var_num(var_name, ((int) floor(*(double *)
				get_symbol_value(var_name)) ^
				(int) floor(right->data.var.val)));
	break;
	case LEFTSHIFT_ASSIGN:
		assign_var_num(var_name, ((int) floor(*(double *)
				get_symbol_value(var_name)) <<
				(int) floor(right->data.var.val)));
	break;
	case RIGHTSHIFT_ASSIGN:
		assign_var_num(var_name, ((int) floor(*(double *)
				get_symbol_value(var_name)) >>
				(int) floor(right->data.var.val)));
	break;
	default:
		printf("assignment operator not recognized\n");
	break;
	}
	if (var_name != NULL)
		free(var_name);
	free_expr(right);
	new_sym->type = NUM;
	new_sym->next = NULL;

	return new_sym;
}

void eval_unary(struct ast *tree)
{
	if (tree->data.bin.op == INC) {
		assign_var_num(tree->data.bin.left->data.var.name, (*(double *)
		     get_symbol_value(tree->data.bin.left->data.var.name)) + 1);
		return;
	} else if (tree->data.bin.op == DEC) {
		assign_var_num(tree->data.bin.left->data.var.name, (*(double *)
		     get_symbol_value(tree->data.bin.left->data.var.name)) - 1);
		return;
	} else if (tree->data.bin.op == BITNOT) {
		assign_var_num(tree->data.bin.left->data.var.name, ~(int)
			 floor(*(double *)
			 get_symbol_value(tree->data.bin.left->data.var.name)));
		return;
	} else if (tree->data.bin.op == NEG) {
		assign_var_num(tree->data.bin.left->data.var.name,
			  (-1) * *(double *)
			  get_symbol_value(tree->data.bin.left->data.var.name));
	}
}

int is_assign(int op)
{
	switch(op) {
		case ASSIGN:
		case PLUS_ASSIGN:
		case MINUS_ASSIGN:
		case MULT_ASSIGN:
		case DIV_ASSIGN:
		case MOD_ASSIGN:
		case AND_ASSIGN:
		case OR_ASSIGN:
		case XOR_ASSIGN:
		case LEFTSHIFT_ASSIGN:
		case RIGHTSHIFT_ASSIGN:
			return 1;
		break;
		default:
			return 0;
		break;
	}
	return 0;
}

static char *format_str(char *str)
{
	unsigned int i;
	int k = 0;
	char *stripped = jmalloc(strlen(str));

	for (i = 0; i < strlen(str); i++) {
		if (str[k] == '\\') {
			switch (str[k + 1]) {
			case 'n':
				stripped[i] = '\n';
				k += 2;
			continue;
			case 't':
				stripped[i] = '\t';
				k += 2;
			continue;
			case 'r':
				stripped[i] = '\r';
				k += 2;
			continue;
			case '0':
				stripped[i] = '\0';
				k += 2;
			continue;
			case '\"':
				stripped[i] = '\"';
				k += 2;
			continue;
			default:
				stripped[i] = str[k];
			break;
			}
		}
		stripped[i] = str[k];
		k++;
	}

	return stripped;
}

struct ast *eval_return(struct ast *tree)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	struct ast *return_data;

	retval->type = RETURN;

	return_data = eval_expr(tree->data.return_val, NULL);

	retval->data.return_val = return_data;

	if (retval->data.return_val->data.var.type == STRING_LITERAL)
		retval->data.return_val->data.var.string =
			format_str(retval->data.return_val->data.var.string);

	return retval;
}



double call_double_func(char *name, int arguments, struct ast_list *parameters)
{
	switch (arguments) {
	double (*fptr_void)(void);
	double (*fptr_one)(double);
	double (*fptr_two)(double, double);
	double (*fptr_three)(double, double, double);
	case 0:
		fptr_void = get_symbol_value(name);
		return (*fptr_void)();
	case 1:
		fptr_one = get_symbol_value(name);
		return (*fptr_one)
			((eval_expr(parameters->top, NULL))->data.var.val);
	case 2:
		fptr_two = get_symbol_value(name);
		return (*fptr_two)
			((eval_expr(parameters->top, NULL))->data.var.val,
			(eval_expr(parameters->top->next, NULL))->
								data.var.val);
	case 3:
		fptr_three = get_symbol_value(name);
		return (*fptr_three)
			((eval_expr(parameters->top, NULL))->data.var.val,
			(eval_expr(parameters->top->next, NULL))->
								data.var.val,
			(eval_expr(parameters->top->next->next, NULL))->
								data.var.val);
	default:
		printf("Too many arguments in function\n");

		return 0;
	}
}


void alert(struct ast_list *arguments, char *obj_name)
{
	struct ast *print_val;
	if (arguments->top == NULL)
		return;

	if (obj_name == NULL)
		print_val = eval_expr(arguments->top, NULL);
	else
		print_val = eval_expr(arguments->top, obj_name);

	if (print_val->type == ARRAY)
		alert_print_array(print_val);
	else if (print_val->type == NULL_TYPE)
		printf("null\n");
	else if (print_val->type == VAR) {
		switch (print_val->data.var.type) {
		case STRING_LITERAL:
			print_val->data.var.string =
					 format_str(print_val->data.var.string);
			printf("%s", print_val->data.var.string);
		break;
		case NUM:
			printf("%g\n", print_val->data.var.val);
		break;
		case BOOL:
			printf("%s\n", print_val->data.var.string);
		break;
		}
	} else
		printf("unrecognized character\n");
	free_expr(print_val);
}
