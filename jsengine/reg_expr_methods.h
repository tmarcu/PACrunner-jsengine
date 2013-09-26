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

#ifndef REG_EXPR_METHODS_H
#define REG_EXPR_METHODS_H

void reg_prototype(char *symbol_name, struct ast_list *arg_list);
struct ast *reg_test(char *symbol_name, struct ast_list *arg_list);
struct ast *reg_exec(char *symbol_name, struct ast_list *arg_list);
struct ast *reg_string_match(char *symbol_name, char *string);
double reg_string_search(char *symbol_name, char *string);
char *reg_string_replace(char *string, struct ast *arg_one,
			 struct ast *arg_two);
char *convert_to_posix(char *old_pat);
char *convert_pat(char *old);
int contains(char *str, char flag);
int get_string_flags(char *flags);
char *get_prim_pattern(char *prim);
char *get_prim_flags(char *prim);
void get_pattern_flag(struct ast *arg_one, char *pattern, char *flags);

#endif
