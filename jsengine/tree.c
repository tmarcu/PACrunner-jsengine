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
#include "tree.h"
#include "jsengine/parser.h"
#include "spider.h"
#include "symbols.h"
#include "math_functions.h"
#include "reg_expr_methods.h"

struct ast *create_new_var(char *var_name)
{
	struct ast *ptr = jmalloc(sizeof(*ptr));

	ptr->type = VAR;
	ptr->data.var.name = strdup(var_name);
	ptr->data.var.type = 0;
	ptr->data.var.string = NULL;
	ptr->next = NULL;
	free(var_name);

	return ptr;
}

struct ast *create_new_var_dec(char *var_name)
{
	struct ast *ptr = jmalloc(sizeof(*ptr));

	ptr->type = VAR_INIT;
	ptr->data.var.name = strdup(var_name);
	ptr->data.var.string = NULL;
	ptr->next = NULL;

	return ptr;
}

struct ast *create_num(double value)
{
	struct ast *ptr = jmalloc(sizeof(*ptr));

	ptr->type = NUM;
	ptr->data.number = value;
	ptr->next = NULL;

	return ptr;
}

struct ast *new_binary_operator(struct ast *left_child,
				struct ast *right_child,
				int operation)
{
	struct ast *ptr = jmalloc(sizeof(*ptr));

	ptr->data.bin.left = left_child;
	ptr->data.bin.right = right_child;
	ptr->data.bin.op = operation;
	ptr->type = BINARY;
	ptr->next = NULL;

	return ptr;
}

struct ast *new_unary_operator(char *name, int operation)
{
	struct ast *ptr = jmalloc(sizeof(*ptr));

	ptr->data.bin.left = create_new_var(name);
	ptr->data.bin.right = 0;
	ptr->data.bin.op = operation;
	ptr->type = UNARY;
	ptr->next = NULL;

	return ptr;
}

struct ast *new_unary_expr(struct ast *left_child, int operation)
{
	struct ast *ptr = jmalloc(sizeof(*ptr));

	ptr->data.bin.left = left_child;
	ptr->data.bin.right = 0;
	ptr->data.bin.op = operation;
	ptr->type = UNARY;
	ptr->next = NULL;

	return ptr;
}

struct ast *create_cond_operator(struct ast *condition,
					struct ast_list *if_true,
					struct ast_list *if_false)
{
	struct ast *ptr = jmalloc(sizeof(*ptr));
	ptr->type = QUESTION;
	ptr->data.ifblock = jmalloc(sizeof(*ptr->data.ifblock));
	ptr->data.ifblock->cond = condition;
	ptr->data.ifblock->block = if_true;
	ptr->data.ifblock->else_block = if_false;
	ptr->next = NULL;

	return ptr;
}

struct ast_list *create_block(struct ast *node)
{
	struct ast_list *ptr = jmalloc(sizeof(*ptr));

	ptr->top = node;

	return ptr;
}

struct ast_list *add_back(struct ast_list *list, struct ast *node)
{
	struct ast *ptr = NULL;

	if (list->top != NULL)
		ptr = list->top;

	/* Should perhaps print error and return same list if list->top = NULL
	 * to avoid any segfaults accessing ptr->next if NULL */
	while (ptr->next != NULL)
		ptr = ptr->next;

	ptr->next = node;

	return list;
}

struct ast *create_loop_statement(struct ast *cond_state,
					struct ast_list *if_block, int op)
{
	struct ast *if_ptr = jmalloc(sizeof(*if_ptr));

	if_ptr->data.ifblock = jmalloc(sizeof(*if_ptr->data.ifblock));
	if_ptr->type = LOOP;
	if_ptr->data.ifblock->loop = op;
	if_ptr->next = NULL;
	if_ptr->data.ifblock->cond = cond_state;
	if_ptr->data.ifblock->block = if_block;
	if_ptr->data.ifblock->else_block = NULL;
	if_ptr->data.ifblock->assign = NULL;
	if_ptr->data.ifblock->stmt = NULL;

	return if_ptr;
}
/*
 * creates an if statement for an if statment without braces
 */
struct ast *create_if_statement(struct ast *condition, struct ast *stmt)
{
	struct ast *if_ptr = jmalloc(sizeof(*if_ptr));
	struct ast_list *if_block = jmalloc(sizeof(*if_block));

	if_block->top = stmt;

	if_ptr->data.ifblock = jmalloc(sizeof(*if_ptr->data.ifblock));
	if_ptr->type = LOOP;
	if_ptr->data.ifblock->loop = IF;
	if_ptr->next = NULL;

	if_ptr->data.ifblock->cond = condition;
	if_ptr->data.ifblock->block = if_block;
	if_ptr->data.ifblock->else_block = NULL;
	if_ptr->data.ifblock->assign = NULL;
	if_ptr->data.ifblock->stmt = NULL;

	return if_ptr;
}

struct ast *create_else_if_statement(struct ast *condition,
					struct ast_list *if_block,
					struct ast_list *else_if_block)
{
	struct ast *ptr = jmalloc(sizeof(*ptr));

	ptr->type = LOOP;
	ptr->data.ifblock = jmalloc(sizeof(*ptr->data.ifblock));

	ptr->data.ifblock->loop = ELSE_IF;
	ptr->data.ifblock->cond = condition;
	ptr->data.ifblock->block = if_block;
	ptr->data.ifblock->else_if = else_if_block;
	ptr->data.ifblock->else_block = NULL;
	ptr->next = NULL;

	return ptr;
}

struct ast *create_elseif_else_loop(struct ast *condition,
						struct ast_list *if_block,
						struct ast_list *else_if_block,
						struct ast_list *else_stmts)
{
	struct ast *ptr = jmalloc(sizeof(*ptr));

	ptr->type = LOOP;
	ptr->data.ifblock = jmalloc(sizeof(*ptr->data.ifblock));

	ptr->data.ifblock->loop = ELSE_IF;
	ptr->data.ifblock->cond = condition;
	ptr->data.ifblock->block = if_block;
	ptr->data.ifblock->else_if = else_if_block;
	ptr->data.ifblock->else_block = else_stmts;
	ptr->next = NULL;

	return ptr;
}

struct ast *create_else_loop(struct ast *condition, struct ast_list *if_block,
				struct ast_list *else_stmts, int op) {
	struct ast *else_ptr = jmalloc(sizeof(*else_ptr));

	else_ptr->type = LOOP;
	else_ptr->data.ifblock = jmalloc(sizeof(*else_ptr->data.ifblock));

	else_ptr->data.ifblock->loop = op;
	else_ptr->next = NULL;
	else_ptr->data.ifblock->cond = condition;
	else_ptr->data.ifblock->block = if_block;
	else_ptr->data.ifblock->else_block = else_stmts;
	else_ptr->data.ifblock->assign = NULL;
	else_ptr->data.ifblock->stmt = NULL;

	return else_ptr;
}

struct ast *create_for_loop(struct ast *dec_state, struct ast *expr,
				struct ast *condition,
				struct ast_list *for_block, int op)
{
	struct ast *for_ptr = jmalloc(sizeof(*for_ptr));

	for_ptr->type = LOOP;
	for_ptr->data.ifblock = jmalloc(sizeof(*for_ptr->data.ifblock));

	for_ptr->data.ifblock->loop = op;
	for_ptr->data.ifblock->assign = dec_state;
	for_ptr->data.ifblock->stmt = expr;
	for_ptr->data.ifblock->cond = condition;
	for_ptr->data.ifblock->block = for_block;
	for_ptr->data.ifblock->else_block = NULL;
	for_ptr->next = NULL;

	return for_ptr;
}

struct ast *create_semi()
{
	struct ast *semi = jmalloc(sizeof(*semi));
	semi->type = SEMICOLON;
	semi->next = NULL;
	return semi;
}

struct ast_list *create_single_stmt(struct ast *stmt)
{
	struct ast_list *block = jmalloc(sizeof(*block));
	block->top = stmt;
	stmt->next = NULL;
	return block;
}

struct ast *create_return(struct ast *return_val)
{
	struct ast *ptr = jmalloc(sizeof(*ptr));

	ptr->type = RETURN;
	ptr->data.return_val = return_val;
	ptr->next = NULL;

	return ptr;
}

struct ast *create_regular(char *reg)
{
	struct ast *exp = jmalloc(sizeof(*exp));
	exp->type = REG_EXP;
	exp->data.var.string = reg;
	exp->next = NULL;

	return exp;
}

struct ast *call_regular_method(char *regex, char *method,
					struct ast_list *arguments)
{
	struct ast *call = jmalloc(sizeof(*call));
	call->type = CALL_REG;
	call->data.func.obj = regex;
	call->data.func.name = method;
	call->data.func.parameters = arguments;
	call->next = NULL;

	return call;
}

struct ast *create_string_literal(char *string)
{
	struct ast *str = jmalloc(sizeof(*str));
	str->type = STRING_LITERAL;
	str->data.string_literal = string;
	str->next = NULL;

	return str;
}

struct ast *create_boolean(char *string)
{
	struct ast *bool = jmalloc(sizeof(*bool));
	bool->type = BOOL;
	bool->data.var.string = strdup(string);
	bool->data.var.val = strcmp(string, "true") ? 0 : 1;
	bool->next = NULL;

	return bool;
}

struct ast *create_new_function(char *func_name,
				struct ast_list *argument_list,
				struct ast_list *stmts)
{
	struct ast *ptr = jmalloc(sizeof(*ptr));

	ptr->type = FUNCTION_DEC;
	ptr->data.func.name = func_name;
	ptr->data.func.parameters = argument_list;
	ptr->data.func.statements = stmts;
	ptr->data.func.obj = NULL;
	ptr->next = NULL;

	return ptr;
}

struct ast *call_function(char *name, struct ast_list *var_list)
{
	struct ast *ptr = jmalloc(sizeof(*ptr));

	ptr->type = CALL_FUNC;
	ptr->data.func.name = name;
	ptr->data.func.parameters = var_list;
	ptr->data.func.statements = NULL;
	ptr->data.func.obj = NULL;
	ptr->next = NULL;

	return ptr;
}

struct ast *call_defined_func(char *name, struct ast_list *var_list)
{
	struct ast *ptr = jmalloc(sizeof(*ptr));
	ptr->type = FUNCTION;
	ptr->data.func.name = name;
	ptr->data.func.parameters = var_list;
	ptr->data.func.statements = NULL;
	ptr->data.func.obj = NULL;
	ptr->next = NULL;

	return ptr;
}

struct ast *create_typeof(struct ast *expr)
{
	struct ast *tvar = jmalloc(sizeof(*tvar));

	tvar->type = TYPEOF;
	tvar->data.typeof_stmt = expr;

	return tvar;
}

struct ast *call_obj_method(char *obj_name, char *method,
				struct ast_list *parameter_list)
{
	struct ast *call_obj = jmalloc(sizeof(*call_obj));

	call_obj->type = CALL_METHOD;
	call_obj->data.func.obj = obj_name;
	call_obj->data.func.name = method;
	call_obj->data.func.parameters = parameter_list;
	call_obj->next = NULL;

	return call_obj;
}

struct ast *create_null(void)
{
	struct ast *ptr = jmalloc(sizeof(*ptr));

	ptr->type = NULL_TYPE;
	ptr->next = NULL;

	return ptr;
}


struct ast *create_array(struct ast *array, struct ast_list *elements)
{
	struct ast *arr = jmalloc(sizeof(*arr));
	struct ast *temp;
	int num_elems = 0;
	int i;

	if (elements != NULL) {
		temp = elements->top;
		while (temp != NULL) {
			num_elems++;
			temp = temp->next;
		}
		arr->data.array.list = jmalloc((sizeof(*arr->data.array.list) *
								    num_elems));
		temp = elements->top;
		for (i = 0; i < num_elems; i++) {
			arr->data.array.list[i] = temp;
			temp = temp->next;
		}
		arr->data.array.size = num_elems;
		free(elements);
	} else {
		arr->data.array.list = jmalloc(sizeof(*arr->data.array.list));
	}
	if (array != NULL) {
		arr->name = strdup(array->data.var.name);
		free_expr(array);
	}
	arr->type = ARRAY;
	arr->next = NULL;

	return arr;
}

struct ast *create_array_item(char *name, struct ast *num)
{
	struct ast *item = jmalloc(sizeof(*item));

	item->type = ARRAY_ITEM;
	item->data.arr_item.name = name;
	item->data.arr_item.ast_index = num;
	item->next = NULL;

	return item;
}

void free_block(struct ast_list *block)
{
	/*struct ast *cur_top;
	struct ast *next_top;

	cur_top = block->top;
	while (cur_top != NULL) {
		next_top = cur_top->next;
		free_stmt(cur_top);
		cur_top = next_top;
	}*/

	free(block);
}

void free_stmt(struct ast *top)
{
	switch (top->type) {
	case BINARY:
		free_binary(top);
	break;
	case UNARY:
		free_unary(top);
	break;
	case VAR:
		free_var(top);
	break;
	case VAR_INIT:
		free_var(top);
	break;
	case LOOP:
		free_loop(top);
	break;
	case RETURN:
		free_return(top);
	break;
	case FUNCTION_DEC:
		free_function_dec(top);
	break;
	case CALL_FUNC:
		free_function_call(top);
	break;
	case FUNCTION:
		free_function_call(top);
	break;
	case ARRAY:
		free_array(top);
	break;
	case NEW:
		free_binary(top);
	break;
	case METHOD_DEC:
		free_method_dec(top);
	break;
	case SWITCH:
		free_switch(top);
	break;
	case BREAK:
		free(top);
	break;
	case CASE:
		free_case(top);
	break;
	case DEFAULT:
		if (top->data.ifblock->block != NULL)
			free_block(top->data.ifblock->block);
		free(top);
	break;
	case QUESTION:
		free_cond_operator(top);
	break;
	case NUM:
		free(top);
	break;
	default:
		/* FIX: the node->type variable does not get set somewhere,
		 * thus we are seeing (garbage) int values that are not part
		 * of our define tokens */
		if (top->type < 400)
			printf("Cannot free statement %d\n", top->type);
	break;
	}
}

void free_cond_operator(struct ast *tree)
{
	free_cond(tree->data.ifblock->cond);
	free_block(tree->data.ifblock->block);
	free_block(tree->data.ifblock->else_block);
	free(tree);
}

void free_case(struct ast *top)
{
	free_expr(top->data.ifblock->cond);
	if (top->data.ifblock->block != NULL)
		free_block(top->data.ifblock->block);
	free(top);
}

void free_switch(struct ast *tree)
{
	free_expr(tree->data.ifblock->cond);
	free_block(tree->data.ifblock->block);
}

void free_method_dec(struct ast *tree)
{
	free(tree->data.func.obj);
	free(tree->data.func.name);
	free_block(tree->data.func.statements);
	free_var_list(tree->data.func.parameters);
	free(tree);
}


void free_array(struct ast *arr)
{
	int i;
	if (arr->name != NULL)
		free(arr->name);
	for (i = 0; i < arr->data.array.size; i++)
		free_expr(arr->data.array.list[i]);

	free(arr->data.array.list);
	free(arr);
}

void free_var_list(struct ast_list *var_list)
{
	if (var_list == NULL)
		return;
	struct ast *cur, *cur_next;

	cur = var_list->top;
	while (cur != NULL) {
		cur_next = cur->next;
		free_expr(cur);
		cur = cur_next;
	}

	free(var_list);
}

void free_function_call(struct ast *tree)
{
	if (tree->data.func.name != NULL)
		free(tree->data.func.name);

	free_var_list(tree->data.func.parameters);
	free(tree);
}

void free_arg_list(struct ast_list *arg_list)
{
	struct ast *cur;
	struct ast *cur_next;

	cur = arg_list->top;
	while (cur != NULL) {
		cur_next = cur->next;
		free_expr(cur);
		cur = cur_next;
	}

	free(arg_list);
}

void free_function_dec(struct ast *tree)
{
	free_block(tree->data.func.statements);
	if (tree->data.func.parameters != NULL)
		free_arg_list(tree->data.func.parameters);
	free(tree->data.func.name);
	free(tree);
}

void free_binary(struct ast *tree)
{
	if (tree->data.bin.left->type == BINARY)
		free_binary(tree->data.bin.left);
	else
		free_expr(tree->data.bin.left);
	if (tree->data.bin.right != NULL) {
		if (tree->data.bin.right->type == BINARY)
			free_binary(tree->data.bin.right);
		else
			free_expr(tree->data.bin.right);
	}
	free(tree);
}

void free_expr(struct ast *tree)
{
	if (tree == NULL)
		return;
	switch (tree->type) {
	case BOOL:
		if (tree->data.var.string != NULL)
			free(tree->data.var.string);
		free(tree);
	break;
	case VAR:
		if (tree->name != NULL)
			free(tree->name);
		if (tree->data.var.name != NULL)
			free(tree->data.var.name);
		if (tree->data.var.string != NULL) {
			free(tree->data.var.string);
			tree->data.var.string = NULL;
		}
		free(tree);
	break;
	case BINARY:
		free_binary(tree);
	break;
	case STRING_LITERAL:
		free(tree->data.string_literal);
		free(tree);
	break;
	case FUNCTION:
		free_function_call(tree);
	break;
	case CALL_FUNC:
		free_function_call(tree);
	break;
	case ARRAY_ITEM:
		free(tree->data.arr_item.name);
		free(tree);
	break;
	case CALL_METHOD:
		free_call_method(tree);
	break;
	case OBJ_PROP:
		free(tree->data.func.name);
		free(tree);
	break;
	case ARRAY:
		free_array(tree);
	break;
	case VAR_INIT:
		if (tree->data.var.name != NULL)
			free(tree->data.var.name);

		free(tree);
	break;
	case QUESTION:
		free_cond_expr(tree);
	break;
	case REG_EXP:
		if (tree->data.var.string != NULL)
			free(tree->data.var.string);
		free(tree);
	break;
	default:
		if (tree != NULL)
			free(tree);
	break;
	}
}

void free_cond_expr(struct ast *tree)
{
	struct ast *cur, *cur_next;
	free_cond(tree->data.ifblock->cond);
	cur = tree->data.ifblock->block->top;
	while (cur->next != NULL) {
		cur_next = cur->next;
		free_stmt(cur);
		cur = cur_next;
	}
	free_expr(cur);
	cur = tree->data.ifblock->else_block->top;
	while (cur->next != NULL) {
		cur_next = cur->next;
		free_stmt(cur);
		cur = cur_next;
	}
	free_expr(cur);
	free(tree);
}

void free_call_method(struct ast *tree)
{
	free(tree->data.func.obj);
	free(tree->data.func.name);
	free_var_list(tree->data.func.parameters);
	free(tree);
}


void free_unary(struct ast *tree)
{
	free_expr(tree->data.bin.left);
	free(tree);
}

void free_var(struct ast *tree)
{
	if (tree->data.var.name != NULL)
		free(tree->data.var.name);

	if (tree->data.var.string != NULL)
		free(tree->data.var.string);

	free(tree);
}

void free_cond(struct ast *tree)
{
	if (tree->type == BINARY) {
		if (tree->data.bin.left != NULL)
			free_cond(tree->data.bin.left);
		if (tree->data.bin.right != NULL)
			free_cond(tree->data.bin.right);
		free(tree);
	} else if (tree->type == UNARY) {
		free_cond(tree->data.bin.left);
		free(tree);
	} else {
		free_expr(tree);
	}
}

void free_initialization(struct ast *tree)
{
	switch (tree->type) {
		case SEMICOLON:
			free(tree);
		break;
		case BINARY:
			if (is_assign(tree->data.bin.op))
				free_stmt(tree);
			else
				free_expr(tree);
		break;
		default:
			free_expr(tree);
		break;
	}
}

void free_loop(struct ast *tree)
{
	switch (tree->data.ifblock->loop) {
	case IF:
		free_cond(tree->data.ifblock->cond);
		free_block(tree->data.ifblock->block);
		free(tree->data.ifblock);
		free(tree);
	break;
	case WHILE:
		free_cond(tree->data.ifblock->cond);
		free_block(tree->data.ifblock->block);
		free(tree);
	break;
	case ELSE:
		free_cond(tree->data.ifblock->cond);
		free_block(tree->data.ifblock->block);
		free_block(tree->data.ifblock->else_block);
		free(tree->data.ifblock);
		free(tree);
	break;
	case DO:
		free_cond(tree->data.ifblock->cond);
		free_block(tree->data.ifblock->block);
		free(tree);
	break;
	case FOR:
		free_cond(tree->data.ifblock->cond);
		free_block(tree->data.ifblock->block);
		free_initialization(tree->data.ifblock->assign);
		free_stmt(tree->data.ifblock->stmt);
		free(tree);
	break;
	case ELSE_IF:
		free_cond(tree->data.ifblock->cond);
		free_block(tree->data.ifblock->block);
		free_else_if(tree->data.ifblock->else_if);
		if (tree->data.ifblock->else_block != NULL)
			free_block(tree->data.ifblock->else_block);
		free(tree);
	break;
	default:
		printf("this type %d is not supported to be freed\n",
			tree->data.ifblock->loop);
	break;
	}
}

void free_else_if(struct ast_list *list)
{
	struct ast *cur;
	struct ast *cur_next;

	cur = list->top;
	while (cur != NULL) {
		cur_next = cur->next;
		free_cond(cur->data.ifblock->cond);
		free_block(cur->data.ifblock->block);
		free(cur);
		cur = cur_next;
	}

	free(list);
}

void free_return(struct ast *tree)
{
	free_expr(tree->data.return_val);
	free(tree);
}

struct ast *create_obj_method(char *obj_name, char *method_name,
			struct ast_list *args, struct ast_list *stmts)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	retval->type = METHOD_DEC;
	retval->data.func.obj = obj_name;
	retval->data.func.name = method_name;
	retval->data.func.parameters = args;
	retval->data.func.statements = stmts;
	retval->next = NULL;

	return retval;
}

struct ast *create_get_obj_prop(char *obj_name, char *prop_name)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	retval->type = OBJ_PROP;
	retval->data.func.obj = obj_name;
	retval->data.func.parameters = NULL;
	retval->data.func.statements = NULL;
	retval->data.func.name = strdup(prop_name);
	retval->next = NULL;

	return retval;
}

struct ast *create_obj_prop(char *prop_name)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	retval->type = OBJ_PROP;
	retval->data.func.obj = NULL;
	retval->data.func.parameters = NULL;
	retval->data.func.statements = NULL;
	retval->data.func.name = strdup(prop_name);
	retval->next = NULL;

	return retval;
}

struct ast *init_obj(struct ast *new_obj, struct ast *constructor)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	retval->type = NEW;
	retval->data.bin.left = new_obj;
	retval->data.bin.right = constructor;
	retval->next = NULL;

	return retval;
}

struct ast *create_switch_statement(struct ast *expr,
					struct ast_list *case_stmts)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	retval->type = SWITCH;
	retval->data.ifblock = jmalloc(sizeof(*retval->data.ifblock));

	retval->data.ifblock->cond = expr;
	retval->data.ifblock->block = case_stmts;
	retval->next = NULL;

	return retval;
}

struct ast *create_break(void)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	retval->type = BREAK;
	retval->next = NULL;

	return retval;
}

struct ast *create_null_case(struct ast *expr)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	retval->type = CASE;
	retval->data.ifblock = jmalloc(sizeof(*retval->data.ifblock));

	retval->data.ifblock->cond = expr;
	retval->data.ifblock->block = NULL;
	retval->next = NULL;

	return retval;
}

struct ast *create_case(struct ast *expr, struct ast_list *stmts)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	retval->type = CASE;
	retval->data.ifblock = jmalloc(sizeof(*retval->data.ifblock));

	retval->data.ifblock->cond = expr;
	retval->data.ifblock->block = stmts;
	retval->next = NULL;

	return retval;
}

struct ast *create_null_default(void)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	retval->type = DEFAULT;
	retval->data.ifblock = jmalloc(sizeof(*retval->data.ifblock));

	retval->data.ifblock->cond = NULL;
	retval->data.ifblock->block = NULL;
	retval->next = NULL;

	return retval;
}

struct ast *create_default(struct ast_list *stmts)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	retval->type = DEFAULT;
	retval->data.ifblock = jmalloc(sizeof(*retval->data.ifblock));

	retval->data.ifblock->cond = NULL;
	retval->data.ifblock->block = stmts;
	retval->next = NULL;

	return retval;
}
