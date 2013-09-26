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

#ifndef STRING_METHODS_H
#define STRING_METHODS_H

struct ast *string_concat(char *name, struct ast_list *arguments);
struct ast *string_char_at(char *name, struct ast_list *arguments);
struct ast *string_char_code_at(char *name, struct ast_list *argument);
struct ast *string_contains(char *name, struct ast_list *arguments);
struct ast *string_ends_with(char *name, struct ast_list *arguments);
struct ast *string_index_of(char *name, struct ast_list *arguments);
struct ast *string_last_index_of(char *name, struct ast_list *arguments);
struct ast *string_local_compare(char *name, struct ast_list *arguments);
struct ast *string_print_hello(void);
struct ast *string_replace(char *name, struct ast_list *arguments);
struct ast *string_splice(char *name, struct ast_list *arguments);
struct ast *string_split(char *name, struct ast_list *arguments);
struct ast *string_substr(char *name, struct ast_list *arguments);
struct ast *string_substring(char *name, struct ast_list *arguments);
struct ast *string_match(char *name, struct ast_list *arguments);
struct ast *string_search(char *name, struct ast_list *arguments);

void string_prototype(char *symbol_name, struct ast_list *arg_list);
void string_setter(struct symbol *obj, char *new_str);

char *get_string_value(struct symbol *object);

#endif
