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
#include "symbols.h"
#include "tree.h"
#include "spider.h"
#include "parser.h"

void boolean_prototype(char *symbol_name, struct ast_list *arg_list)
{
	struct ast *val;
	struct symbol *sym = get_symbol(symbol_name);

	if (arg_list->top != NULL)
		val = eval_expr(arg_list->top, NULL);
	else {
		fprintf(stderr, "BOOL: No expression to evaluate\n");
		exit(EXIT_FAILURE);
	}

	(void) add_obj_property(sym, "value");

	if (val->data.var.type == STRING_LITERAL)
		assign_prop_str(symbol_name, "value", val->data.var.string);
	else if (val->data.var.type == NUM)
		assign_prop_num(symbol_name, "value", val->data.var.val);

	free_expr(val);
}

struct ast *bool_valueof(char *name, struct ast_list *arguments)
{
	struct ast *retval;
	struct symbol *obj_val;
	struct symbol *object = get_symbol(name);
	char *obj_type = get_obj_type(name);

	if (strcmp(obj_type, "Boolean") != 0)
		return NULL;

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = BOOL;

	obj_val = get_property(object, "value");
	if (obj_val->value.var.type == STRING_LITERAL)
		retval->data.var.string = strdup(obj_val->value.var.str);
	else
		retval->data.var.string = object->value.var.val ?
					  strdup("true") : strdup("false");
	retval->next = NULL;

	return retval;
}

struct ast *bool_tostring(char *name, struct ast_list *arguments)
{
	struct ast *retval;
	struct symbol *obj_val;
	struct symbol *object = get_symbol(name);
	char *obj_type = get_obj_type(name);

	if (strcmp(obj_type, "Boolean") != 0)
		return NULL;

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = STRING_LITERAL;

	obj_val = get_property(object, "value");
	if (obj_val->value.var.type == STRING_LITERAL)
		retval->data.var.string = strdup(obj_val->value.var.str);
	else if (obj_val->value.var.type == NUM)
		retval->data.var.string = obj_val->value.var.val ?
					  strdup("true") : strdup("false");

	retval->next = NULL;

	return retval;
}
