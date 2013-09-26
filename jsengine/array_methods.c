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

void print_array(char *name)
{
	int i;
	struct symbol *arr = get_symbol(name);
	struct ast *arg;

	if (arr->type != ARRAY) {
		fprintf(stderr, "Object is not an array\n");
		return;
	}

	arg = arr->value.array;
	for (i = 0; i < arg->data.array.size; i++) {
		if (arg->data.array.list[i]->type == STRING_LITERAL)
			printf("%s,",
				arg->data.array.list[i]->data.string_literal);
		else
			printf("%g,", arg->data.array.list[i]->data.number);
	}
	printf("\n");
}

void alert_print_array(struct ast *arg)
{
	int i;

	if (arg->type != ARRAY) {
		fprintf(stderr, "Object is not an array\n");
		return;
	}

	for (i = 0; i < arg->data.array.size; i++) {
		if (arg->data.array.list[i]->data.var.type == STRING_LITERAL)
			printf("%s,", arg->data.array.list[i]->data.var.string);
		else
			printf("%g,", arg->data.array.list[i]->data.var.val);
	}
	printf("\n");
}

struct ast *array_reverse(char *name, struct ast_list *arguments)
{
	int i;
	struct ast *arg;
	struct ast *temp;
	struct symbol *arr = get_symbol(name);

	if (arr->type != ARRAY) {
		fprintf(stderr, "Object is not an array\n");
		return NULL;
	}

	arg = arr->value.array;
	for (i = 0; i < arg->data.array.size/2; i++) {
		temp = arg->data.array.list[arg->data.array.size-i-1];
		arg->data.array.list[arg->data.array.size-i-1] =
							arg->data.array.list[i];
		arg->data.array.list[i] = temp;
	}

	return NULL;
}

struct ast *array_concat(char *name, struct ast_list *arguments)
{
	int i;
	int j;
	int prevsize;
	struct ast *arg;
	struct ast *array1;
	struct ast *retval = jmalloc(sizeof(*retval));
	struct symbol *arr = get_symbol(name);

	if (arr->type != ARRAY) {
		fprintf(stderr, "Object is not an array\n");
		return NULL;
	}

	array1 = arr->value.array;

	arg = eval_expr(arguments->top, NULL);

	prevsize = array1->data.array.size;
	retval->data.array.size = array1->data.array.size +
							 arg->data.array.size;

	retval->data.array.list = jmalloc(sizeof(*retval->data.array.list) *
						retval->data.array.size);
	j = 0;
	for (i = 0; i < retval->data.array.size; i++) {
		retval->data.array.list[i] =
				jmalloc(sizeof(*retval->data.array.list[i]));
		retval->data.array.list[i] = array1->data.array.list[i];

		if (i >= prevsize) {
			retval->data.array.list[i] = arg->data.array.list[j];
			j++;
		}
	}

	retval->type = ARRAY;
	retval->next = NULL;

	return retval;
}

struct ast *array_join(char *name, struct ast_list *arguments)
{
	int i;
	char temp[16];
	char *string = jmalloc(100);
	struct ast *seperator;
	struct ast *array;
	struct ast *retval;
	struct ast **list;
	struct symbol *arr;

	arr = get_symbol(name);
	if (arr->type != ARRAY) {
		fprintf(stderr, "Object is not an array\n");
		return NULL;
	}

	array = arr->value.array;
	list = array->data.array.list;

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	seperator = jmalloc(sizeof(*seperator));

	if (arguments->top != NULL)
		seperator = arguments->top;
	else
		seperator->data.string_literal = strdup(",");

	for (i = 0; i < array->data.array.size; i++) {
		if (list[i]->data.var.type == STRING_LITERAL) {
			strcat(string, list[i]->data.var.string);
		} else if (list[i]->type == NUM) {
			sprintf(temp, "%g", list[i]->data.var.val);
			strcat(string, temp);
		}

		if (i < array->data.array.size - 1)
			strcat(string, seperator->data.string_literal);
	}

	retval->type = VAR;
	retval->data.var.type = STRING_LITERAL;
	retval->data.var.string = strdup(string);
	retval->next = NULL;

	free(string);

	return retval;
}

struct ast *array_last_index_of(char *name, struct ast_list *arguments)
{
	int i;
	int start = 0;
	struct ast *array;
	struct ast **list;
	struct ast *retval;
	struct ast *element;
	struct symbol *arr;

	arr = get_symbol(name);
	if (arr->type != ARRAY) {
		fprintf(stderr, "Object is not an array\n");
		return NULL;
	}

	array = arr->value.array;
	list = array->data.array.list;

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;

	if (arguments->top != NULL)
		element = eval_expr(arguments->top, NULL);
	else
		return NULL;

	if (arguments->top->next != NULL) {
		struct ast *temp = eval_expr(arguments->top->next, NULL);
		start = (int) temp->data.var.val;
		free_expr(temp);
	}

	/* start will set i to a negative number otherwise */
	if (start > array->data.array.size)
		start = 0;

	for (i = array->data.array.size - 1 - abs(start); i >= 0; i--) {
		if (strcmp(element->data.var.string,
					list[i]->data.var.string) == 0) {
			retval->data.var.val = i;
			break;
		} else if (i == 0) {
			retval->data.var.val = -1;
			break;
		}
	}

	retval->data.var.type = NUM;
	retval->next = NULL;

	return retval;
}

struct ast *array_index_of(char *name, struct ast_list *arguments)
{
	int i;
	int start = 0;
	struct ast *array;
	struct ast **list;
	struct ast *retval;
	struct ast *element;
	struct symbol *arr;

	arr = get_symbol(name);
	if (arr->type != ARRAY) {
		fprintf(stderr, "Object is not an array\n");
		return NULL;
	}

	array = arr->value.array;
	list = array->data.array.list;

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;

	if (arguments->top != NULL)
		element = eval_expr(arguments->top, NULL);
	else
		return NULL;

	if (arguments->top->next != NULL) {
		struct ast *temp = eval_expr(arguments->top->next, NULL);
		start = (int) temp->data.var.val;
		free_expr(temp);
	}

	/* start will set i to a negative number otherwise */
	if (start  > array->data.array.size)
		start = 0;

	for (i = 0 + abs(start); i < (int) array->data.array.size; i++) {
		if (strcmp(element->data.var.string,
					list[i]->data.var.string) == 0) {
			retval->data.var.val = i;
			break;
		}
	}

	if (i == (int) array->data.array.size)
		retval->data.var.val = -1;

	free_expr(element);
	retval->data.var.type = NUM;
	retval->next = NULL;

	return retval;
}
