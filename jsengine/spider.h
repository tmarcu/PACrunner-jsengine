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

#ifndef SPIDER_H
#define SPIDER_H

#include "symbols.h"

struct ast *eval_expr(struct ast *tree, char *name);
struct ast *eval_loop(struct ast *tree);
struct ast *eval_statement(struct ast *tree, char *name);
struct ast *statements_spider(struct ast_list *list);
struct ast *eval_assign(struct ast *tree, char *name);
struct ast *eval_return(struct ast *tree);
struct ast *eval_func(struct ast *tree, struct ast_list *parameters);
struct ast *eval_init(struct ast *tree);
struct ast *eval_method(char *obj, char *method, struct ast_list *parameters);
struct ast *eval_func_dec(struct ast *tree);
struct ast *eval_init_obj(struct ast *new_obj, struct ast *constructor);
struct ast *eval_switch(struct ast *expr, struct ast_list *case_list);
struct ast *eval_cond_operator(struct ast *tree, char *);
struct ast *eval_cond_statement(struct ast *tree, char *name);
struct ast *eval_reg_method(char *regex, char *method, struct ast_list *args);
struct ast *eval_not_unary(struct ast *tree, char *obj);
struct ast *eval_inherent_function(char *name, struct ast_list *arguments);
struct ast *eval_array_item(struct ast *index, struct ast *temp_ast,
				struct symbol *temp, struct ast *tree);

struct ast *get_declared_func(char *name);
struct ast *get_typeof(struct ast *tree);
struct ast *get_prop_value(char *obj_name, char *prop_name);

struct ast *statements_spider_func(struct ast_list *list, char *name);
struct ast_list *create_scope(void);

char *do_string_add(char *str_one, char *str_two, int op);

void add_parameter_vars(struct ast_list *var_name, struct ast_list *var_val);
void remove_parameter_vars(struct ast_list *parameters);
void eval_unary(struct ast *tree);
void add_new_dec_function(struct ast *dec_fun);
void eval_void_func(struct ast *tree, struct ast_list *parameters);
void eval_void_inherent_function(char *name, struct ast_list *arguments,
						char *obj_name);
void alert(struct ast_list *arguments, char *obj_name);
void free_dec_function(struct ast *dec_fun);
void scope_add_sym(struct ast_list *scope, struct ast *sym);
void free_scope(struct ast_list *scope);
void eval_init_obj_method(struct ast *tree);
void eval_void_method(char *obj, char *method_name, struct ast_list *parameters);
void copy_array_element(struct ast *dest, struct ast *source);
void eval_initialization(struct ast *init);

int is_assign(int op);
int compare_expr(struct ast *one, struct ast *two);
int eval_cond(struct ast *cond_tree);
double call_double_func(char *name, int arguments, struct ast_list *parameters);
double do_op(double left, double right, int op);


#endif
