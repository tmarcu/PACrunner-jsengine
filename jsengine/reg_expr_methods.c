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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include "symbols.h"
#include "tree.h"
#include "spider.h"
#include "parser.h"
#include "reg_expr_methods.h"
#include "array_methods.h"

static void alloc_esc_size(char *old_pat, int *size)
{
	unsigned int i;

	if (old_pat == NULL) {
		fprintf(stderr, "Pattern is NULL!\n");
		exit(EXIT_FAILURE);
	}

	*size = 0;
	for (i = 0; i < strlen(old_pat); i++) {
		switch (old_pat[i]) {
		case '\\':
			if (old_pat[i + 1] == '\\') {
				*size += 2;
				i++;
			} else if (old_pat[i + 1] == 'd') {
				*size += 5;
				i++;
			} else if (old_pat[i + 1] == 'D') {
				*size += 6;
				i++;
			} else if (old_pat[i + 1] == 'n') {
				*size += 2;
				i++;
			} else if (old_pat[i + 1] == 's') {
				*size += 8;
				i++;
			} else if (old_pat[i + 1] == 's') {
				*size += 9;
				i++;
			} else if (old_pat[i + 1] == 'w') {
				*size += 12;
				i++;
			} else if (old_pat[i + 1] == 'W') {
				*size += 13;
				i++;
			} else if (old_pat[i + 1] == '.') {
				*size += 2;
				i++;
			}
		break;
		case '+':
		case '?':
		case '(':
		case ')':
		case '|':
		case '{':
		case '}':
			*size += 2;
		break;
		default:
			*size += 1;
		break;
		}
	}
	*size += 1;
}

static void concat_new_pattern(char *old_pat, char *new_pat)
{
	unsigned int i;
	size_t size = 0;

	for (i = 0; i < strlen(old_pat); i++) {
		switch (old_pat[i]) {
		case '\\':
			if (old_pat[i + 1] == '\\') {
				strcat(new_pat, "\\\\");
				size += 2;
				i++;
			} else if (old_pat[i + 1] == 'd') {
				strcat(new_pat, "[0-9]");
				size += 5;
				i++;
			} else if (old_pat[i + 1] == 'D') {
				strcat(new_pat, "[^0-9]");
				size += 6;
				i++;
			} else if (old_pat[i + 1] == 'n') {
				strcat(new_pat, "\\n");
				size += 2;
				i++;
			} else if (old_pat[i + 1] == 's') {
				strcat(new_pat, "[ tab\\n]");
				size += 8;
				i++;
			} else if (old_pat[i + 1] == 'S') {
				strcat(new_pat, "[^ tab\\n]");
				size += 9;
				i++;
			} else if (old_pat[i + 1] == 'w') {
				strcat(new_pat, "[A-Za-z0-9_]");
				size += 12;
				i++;
			} else if (old_pat[i + 1] == 'W') {
				strcat(new_pat, "[^A-Za-z0-9_]");
				size += 13;
				i++;
			} else if (old_pat[i + 1] == '.') {
				strcat(new_pat, "\\.");
				size += 2;
				i++;
			}
		break;
		case '+':
		case '?':
		case '(':
		case ')':
		case '|':
		case '{':
		case '}':
			new_pat[size] = '\\';
			size++;
			new_pat[size] = old_pat[i];
			size++;
			
		break;
		default:
			new_pat[size] = old_pat[i];
			size++;
		break;
		}
	}
	new_pat[size] = '\0';
}

char *convert_pat(char *old)
{
	int size = 0;
	unsigned int i;
	char *new_str;

	for (i = 0; i < strlen(old); i++) {
		if (old[i] == '\\' && old[i + 1] == '\\')
			i++;
		size++;
	}

	new_str = jmalloc(sizeof(char) * size + 1);
	size = 0;

	for (i = 0; i < strlen(old); i++) {
		if (old[i] == '\\' && old[i + 1] == '\\')
			i++;
		new_str[size] = old[i];
		size++;
	}
	new_str[size] = '\0';

	return new_str;
}

void reg_prototype(char *symbol_name, struct ast_list *arg_list)
{
	struct symbol *sym = get_symbol(symbol_name);
	struct ast *val = eval_expr(arg_list->top, NULL);
	char *real_pat;

	(void) add_obj_property(sym, "pattern");
	(void) add_obj_property(sym, "flags");
	(void) add_obj_property(sym, "lastIndex");
	(void) add_obj_property(sym, "flags");

	real_pat = convert_pat(val->data.var.string);
	assign_prop_str(symbol_name, "pattern", real_pat);

	free_expr(val);
	free(real_pat);

	if (arg_list->top->next != NULL) {
		val = eval_expr(arg_list->top->next, NULL);

		assign_prop_str(symbol_name, "flags", val->data.var.string);
		free_expr(val);
	}
	assign_prop_num(symbol_name, "lastIndex", 0);
}

char *convert_to_posix(char *old_pat)
{
	int size;
	char *new_pat;

	alloc_esc_size(old_pat, &size);
	new_pat = jmalloc(sizeof(char) * size);

	concat_new_pattern(old_pat, new_pat);

	return new_pat;
}

void get_sym_reg(char *symbol_name, char *pattern, char *flags)
{
	struct symbol *temp = get_symbol(symbol_name);
	if (temp->type == REG_EXP) {
		pattern = get_prim_pattern(temp->value.var.str);
		flags = get_prim_flags(temp->value.var.str);
	} else if (temp->type == VAR_OBJECT) {
		struct symbol *pattern_sym = get_property(temp, "pattern");
		struct symbol *flags_sym = get_property(temp, "flags");
		pattern = strdup(pattern_sym->value.var.str);
		if (flags_sym->value.var.str != NULL)
			flags = strdup(flags_sym->value.var.str);
		else
			flags = NULL;
	}
}

char *get_pattern(char *symbol_name)
{
	struct symbol *temp = get_symbol(symbol_name);
	if (temp->type == REG_EXP)
		return get_prim_pattern(temp->value.var.str);
	else {
		struct symbol *pattern_sym = get_property(temp, "pattern");
		return strdup(pattern_sym->value.var.str);
	}
}

char *get_flags(char *symbol_name)
{
	struct symbol *temp = get_symbol(symbol_name);
	if (temp->type == REG_EXP)
		return get_prim_flags(temp->value.var.str);
	else {
		struct symbol *flags_sym = get_property(temp, "flags");
		if (flags_sym->value.var.str != NULL)
			return strdup(flags_sym->value.var.str);
		else
			return NULL;
	}
}

struct ast *reg_test(char *symbol_name, struct ast_list *arg_list)
{
	int i = 0;
	int match_size = 0;
	int last_index = 0;
	int flag;
	char *flags;
	char *pattern;
	char *posix_pat;
	char *match_str;
	struct symbol *temp;
	struct symbol *obj = get_symbol(symbol_name);
	struct ast *retval = jmalloc(sizeof(*retval));
	struct ast *match = eval_expr(arg_list->top, NULL);
	regex_t *reg = jmalloc(sizeof(*reg));

	pattern = get_pattern(symbol_name);
	posix_pat = convert_to_posix(pattern);
	flags = get_flags(symbol_name);
	flag = get_string_flags(flags);

	if (regcomp(reg, posix_pat, flag) != 0)
		printf("ERROR COMPILING REGULAR EXPRESSION\n");

	if (contains(flags, 'g')) {
		temp = get_property(obj, "lastIndex");
		last_index = (int) temp->value.var.val;
		i = last_index;
	}
	match_str = (char *) &match->data.var.string[i];

	retval->type = VAR;
	retval->next = NULL;
	retval->data.var.type = NUM;
	retval->data.var.val = 1;

	match_size = reg->re_nsub + 1;
	regmatch_t match_array[match_size];

	if (regexec(reg, match_str, match_size, match_array, 0) != 0) {
		assign_prop_num(obj->name, "lastIndex", 0);
		retval->data.var.val = 0;
	}

	i = 0;
	while (match_array[i].rm_so != -1 && i < match_size)
		i++;

	if (contains(flags, 'g')) {
		assign_prop_num(obj->name, "lastIndex", (double)
				(match_array[i].rm_eo + last_index));
	}

	free_expr(match);
	regfree(reg);
	free(reg);
	free(posix_pat);
	if (flags != NULL)
		free(flags);
	free(pattern);

	return retval;
}


struct ast *reg_exec(char *symbol_name, struct ast_list *arg_list)
{
	int i;
	int j;
	int k;
	int match_size = 0;
	int last_index = 0;
	int flag = 0;
	char *array_str;
	char *posix_pat;
	char *match_str;
	char *pattern, *flags;
	struct ast *arr_element;
	struct ast *retval = jmalloc(sizeof(*retval));
	struct symbol *obj = get_symbol(symbol_name);
	regex_t *reg = jmalloc(sizeof(*reg));
	struct ast *match;

	pattern = get_pattern(symbol_name);
	posix_pat = convert_to_posix(pattern);
	flags = get_flags(symbol_name);
	flag = get_string_flags(flags);

	if (regcomp(reg, posix_pat, flag) != 0)
		printf("ERROR COMPILING REGULAR EXPRESSION\n");

	if (arg_list->top == NULL)
		printf("arg list is null\n");

	match = eval_expr(arg_list->top, NULL);
	i = 0;

	if (contains(flags, 'g')) {
		struct symbol *temp = get_property(obj, "lastIndex");
		last_index = (int) temp->value.var.val;
		i = last_index;
	}

	match_str = (char *) &match->data.var.string[i];
	retval->next = NULL;
	match_size = (int) reg->re_nsub + 1;
	regmatch_t match_array[match_size];

	for (i = 0; i < match_size; i++) {
		match_array[i].rm_so = -1;
		match_array[i].rm_eo = -1;
	}

	k = regexec(reg, match_str, match_size, match_array, 0);
	if (k != 0) {
		//printf("This does not match!\n");
		retval->type = NULL_TYPE;
		assign_prop_num(obj->name, "lastIndex", 0);
		return retval;
	}

	i = 0;
	while (match_array[i].rm_so != -1 && i < match_size)
		i++;


	retval->type = ARRAY;
	retval->data.array.size = i;
	retval->data.array.list = jmalloc(sizeof(struct ast *) * i);

	i = 0;

	while (match_array[i].rm_so != -1 && i < match_size) {
		array_str = jmalloc(sizeof(char) * (match_array[i].rm_eo
						 - match_array[i].rm_so + 1));
		k = 0;
		for (j = match_array[i].rm_so; j < match_array[i].rm_eo; j++) {
			array_str[k] = match_str[j];
			k++;
		}
		array_str[k] = '\0';

		arr_element = jmalloc(sizeof(*arr_element));
		arr_element->type = VAR;
		arr_element->data.var.type = STRING_LITERAL;
		arr_element->data.var.string = array_str;

		retval->data.array.list[i] = arr_element;
		i++;
	}
	if (contains(flags, 'g')) {
		i--;
		assign_prop_num(obj->name, "lastIndex", (double)
				(match_array[i].rm_eo + last_index));
	}

	regfree(reg);
	free(reg);
	free_expr(match);
	free(posix_pat);
	if (flags != NULL)
		free(flags);
	free(pattern);

	return retval;
}

int contains(char *str, char flag)
{
	unsigned int i;

	if (str == NULL)
		return 0;

	for (i = 0; i < strlen(str); i++) {
		if (str[i] == flag)
			return 1;
	}

	return 0;
}

int get_last_index(struct symbol *obj, char *flags)
{
	int last_index = 0;
	if (contains(flags, 'g')) {
		struct symbol *temp = get_property(obj, "lastIndex");
		last_index = (int) temp->value.var.val;
	}
	return last_index;
}


struct ast *reg_string_match(char *symbol_name, char *string)
{
	int i;
	int j;
	int k;
	int match_size;
	int flag;
	int last_index = 0;
	char *array_str;
	char *match_str;
	char *flags;
	char *pattern;
	char *posix_pat;
	struct ast *temp;
	struct ast *arr_element;
	struct ast *temp_list;
	struct ast *temp_list_next;
	struct symbol *obj = get_symbol(symbol_name);
	struct ast *retval = jmalloc(sizeof(*retval));
	regex_t *reg = jmalloc(sizeof(*reg));

	pattern = get_pattern(symbol_name);
	flags = get_flags(symbol_name);
	posix_pat = convert_to_posix(pattern);

	assign_prop_num(obj->name, "lastIndex", 0);

	flag = get_string_flags(flags);

	if (regcomp(reg, posix_pat, flag) != 0)
		printf("ERROR COMPILING REGULAR EXPRESSION\n");

	struct ast *list;
	list = NULL;
	retval->next = NULL;
	retval->type = NULL_TYPE;
	i = 0;
	do {
		last_index = get_last_index(obj, flags);

		match_str = (char *) &string[last_index];
		match_size = (int) reg->re_nsub + 1;
		regmatch_t match_array[match_size];

		for (i = 0; i <= match_size; i++) {
			match_array[i].rm_so = -1;
			match_array[i].rm_eo = -1;
		}

		k = regexec(reg, match_str, match_size, match_array, 0);

		if (k != 0) {
			assign_prop_num(obj->name, "lastIndex", 0);
			break;
		}

		i = match_size - 1;
		
		temp_list = NULL;
		while (i >= 0) {
			if (match_array[i].rm_so != -1) {
				array_str = jmalloc(sizeof(char) *
						(match_array[i].rm_eo -
						match_array[i].rm_so + 1));
				k = 0;

				for (j = match_array[i].rm_so; j <
						    match_array[i].rm_eo; j++) {
					array_str[k] = match_str[j];
					k++;
				}
				array_str[k] = '\0';

				arr_element = jmalloc(sizeof(*arr_element));
				arr_element->type = VAR;
				arr_element->data.var.type = STRING_LITERAL;
				arr_element->data.var.string = array_str;
				arr_element->next = temp_list;
				temp_list = arr_element;
			}
			i--;
		}
		while (temp_list != NULL) {
			temp_list_next = temp_list->next;
			temp_list->next = list;
			list = temp_list;
			temp_list = temp_list_next;
		}

		if (contains(flags, 'g')) {
			i = 0;
			while (i < match_size && match_array[i].rm_so != -1)
				i++;

			i--;
			assign_prop_num(obj->name, "lastIndex", (double)
					(match_array[i].rm_eo + last_index));
		}
	} while (contains(flags, 'g'));

	if (list == NULL) {
		regfree(reg);
		free(reg);
		free(posix_pat);
		if (flags != NULL)
			free(flags);
		free(pattern);
		return retval;
	}

	temp = list;
	for (i = 0; temp != NULL; i++)
		temp = temp->next;

	retval->data.array.list = jmalloc(sizeof(struct ast *) * i);
	retval->type = ARRAY;
	retval->data.array.size = i;
	i--;
	while (i >= 0) {
		retval->data.array.list[i] = list;
		list = list->next;
		i--;
	}

	assign_prop_num(obj->name, "lastIndex", 0);

	regfree(reg);
	free(reg);
	free(posix_pat);
	if (flags != NULL)
		free(flags);
	free(pattern);

	return retval;
}

double reg_string_search(char *symbol_name, char *string)
{
	double retval;
	char *pattern, *flags;
	regex_t *reg = jmalloc(sizeof(*reg));
	pattern = get_pattern(symbol_name);
	flags = get_flags(symbol_name);
	char *posix_pat = convert_to_posix(pattern);

	int flag = get_string_flags(flags);
	if (regcomp(reg, posix_pat, flag) != 0)
		printf("ERROR COMPILING REGULAR EXPRESSION\n");

	retval = -1;

	regmatch_t match_array[1];

	if (regexec(reg, string, 1, match_array, 0) != 0)
		printf("Does not match!\n");
	else
		retval = (double) match_array[0].rm_so;

	regfree(reg);
	free(reg);
	free(posix_pat);
	if (flags != NULL)
		free(flags);
	free(pattern);
	return retval;
}

int get_string_flags(char *flags)
{
	int retval = 0;
	int i;
	if (flags == NULL)
		return retval;

	int length = strlen(flags);
	for (i = 0; i < length; i++) {
		if (flags[i] == 'i')
			retval = retval | REG_ICASE;
		if (flags[i] == 'm')
			retval = retval | REG_NEWLINE;
	}
	return retval;
}

char *get_prim_pattern(char *prim)
{
	int size = 0;
	int i;
	char *pat;

	for (i = 1; prim[i] != '/'; i++)
		size++;

	pat = (char *) jmalloc(sizeof(*pat) * size + 1);

	for (i = 1; prim[i] != '/'; i++)
		pat[i - 1] = prim[i];

	pat[size] = '\0';
	return pat;
}

char *get_prim_flags(char *prim)
{
	int start;
	int size;
	unsigned int i;
	char *flag;
	for (i = 1; prim[i] != '/'; i++);
		/* find index */
	i++;

	size = strlen(prim) - i;
	if (size == 0)
		return NULL;

	flag = (char *) jmalloc(sizeof(*flag) * size + 1);

	start = 0;
	while (i < strlen(prim)) {
		flag[start] = prim[i];
		start++;
		i++;
	}

	flag[size] = '\0';

	return flag;
}

void get_pattern_flag(struct ast *arg_one, char *pattern, char *flags)
{
	if (arg_one->type == REG_EXP) {
		pattern = get_prim_pattern(arg_one->data.var.string);
		flags = get_prim_flags(arg_one->data.var.string);
	} else {
		struct symbol *obj = get_symbol(arg_one->data.var.name);
		struct symbol *pattern_sym = get_property(obj, "pattern");
		struct symbol *flags_sym = get_property(obj, "flags");
		pattern = strdup(pattern_sym->value.var.str);
		if (flags_sym->value.var.str != NULL)
			flags = strdup(flags_sym->value.var.str);
		else
			flags = NULL;
	}
}

char *get_arg_pattern(struct ast *arg_one)
{
	if (arg_one->type == REG_EXP)
		return get_prim_pattern(arg_one->data.var.string);
	else {
		struct symbol *obj = get_symbol(arg_one->data.var.name);
		struct symbol *pattern_sym = get_property(obj, "pattern");
		return strdup(pattern_sym->value.var.str);
	}
}

char *get_arg_flags(struct ast *arg_one)
{
	if (arg_one->type == REG_EXP)
		return get_prim_flags(arg_one->data.var.string);
	else {
		struct symbol *obj = get_symbol(arg_one->data.var.name);
		struct symbol *flags_sym = get_property(obj, "flags");
		if (flags_sym->value.var.str != NULL)
			return strdup(flags_sym->value.var.str);
		else
			return NULL;
	}
}

char *reg_string_replace(char *string, struct ast *arg_one, struct ast *arg_two)
{
	char *pattern;
	char *flags;
	char *posix_pat;
	int flag = 0;
	int last_index;
	int k;
	int j;
	int has_g;
	int total = 0;
	unsigned int i;
	unsigned int length;
	regex_t reg;
	char *match_str;
	char *replace;
	char *ret_string;
	struct ast *temp;

	pattern = get_arg_pattern(arg_one);
	flags = get_arg_flags(arg_one);

	flag = get_string_flags(flags);

	posix_pat = convert_to_posix(pattern);
	if (regcomp(&reg, posix_pat, flag) != 0)
		printf("ERROR COMPILING REGULAR EXPRESSION\n");

	length = strlen(string);
	int match_begin[length];
	int match_end[length];

	for (i = 0; i < length; i++) {
		match_begin[i] = -1;
		match_end[i] = -1;
	}

	i = 0;
	last_index = 0;
	has_g = contains(flags, 'g');

	do {
		match_str = (char *) &string[last_index];

		regmatch_t match_array[1];
		match_array[0].rm_so = -1;

		k = regexec(&reg, match_str, 1, match_array, 0);

		if (match_array[0].rm_so == -1)
			break;

		match_begin[i] = match_array[0].rm_so;
		match_end[i] = match_array[0].rm_eo;
		last_index += match_array[0].rm_eo;

		i++;
	} while (has_g);
	length = 0;

	i = 0;
	k = 0;
	temp = eval_expr(arg_two, NULL);
	replace = temp->data.var.string;

	while (match_begin[k] != -1) {
		length += match_begin[k];
		length += strlen(replace);
		i += match_end[k];
		k++;
	}

	length += strlen(string) - i;
	length++;
	ret_string = (char *) jmalloc(sizeof(*ret_string) * length);
	k = 0;
	j = 0;
	i = 0;

	while (i < strlen(string)) {
		if (i != (unsigned) match_begin[k] + total) {
			ret_string[j] = string[i];
			j++;
			i++;
		} else {
			strcat(ret_string, replace);
			j += strlen(replace);
			i += match_end[k] - match_begin[k];
			total += match_end[k];
			k++;
		}

	}
	ret_string[length - 1] = '\0';

	free_expr(temp);
	regfree(&reg);
	free(posix_pat);
	if (flags != NULL)
		free(flags);
	free(pattern);

	return ret_string;
}
