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

#ifndef TREE_H
#define TREE_H

struct __attribute__((packed)) variable {
	double val;
	char *name;
	char *string;
	int type;
};

struct binary_op {
	struct ast *left;
	struct ast *right;
	int op;
};

struct __attribute__((packed)) if_statement {
	struct ast *cond;
	struct ast_list *block;
	struct ast_list *else_block;
	struct ast_list *else_if;
	struct ast *stmt;
	struct ast *assign;
	int loop;
};

struct function {
	char *name;
	struct ast_list *parameters;
	struct ast_list *statements;
	char *obj;

};

struct array_item {
	char *name;
	struct ast *ast_index;
	int index;
};

struct array_block {
	struct ast **list;
	int size;
};

struct __attribute__((packed)) ast {
	union {
		char *string_literal;
		double number;
		struct ast *return_val;
		struct ast *typeof_stmt;
		struct if_statement *ifblock;
		struct array_block array;
		struct array_item arr_item;
		struct variable var;
		struct binary_op bin;
		struct function func;
	} data;
	struct ast *next;
	char *name;
	int type;

};

struct ast_list {
	struct ast *top;
};

struct ast_list *create_block(struct ast *node);
struct ast_list *add_back(struct ast_list *list, struct ast *node);

struct ast *create_new_var(char *var_name);
struct ast *create_new_var_dec(char *var_name);
struct ast *create_num(double num);
struct ast *create_string_literal(char *string);
struct ast *create_boolean(char *string);
struct ast *create_null();
struct ast *create_semi();
struct ast *create_null_case(struct ast *expr);
struct ast *create_null_default(void);
struct ast *create_typeof(struct ast *expr);
struct ast *create_array(struct ast *array, struct ast_list *elements);
struct ast *create_array_item(char *name, struct ast *num);
struct ast *create_return(struct ast *return_val);
struct ast_list *create_single_stmt(struct ast *stmt);
struct ast *create_switch_statement(struct ast *expr,
					struct ast_list *case_stmts);
struct ast *create_case(struct ast *expr, struct ast_list *stmts);
struct ast *create_break(void);
struct ast *create_default(struct ast_list *stmts);

struct ast *init_obj(struct ast *new_obj, struct ast *constructor);
struct ast *create_obj_method(char *obj_name, char *method_name,
			struct ast_list *args, struct ast_list *stmts);
struct ast *create_get_obj_prop(char *obj_name, char *prop_name);
struct ast *create_obj_prop(char *prop_name);
struct ast *create_new_function(char *name, struct ast_list *argument_list,
					struct ast_list *stmts);


struct ast *new_binary_operator(struct ast *left_child,
					struct ast *right_child, int op);
struct ast *new_unary_operator(char *name, int op);
struct ast *new_unary_expr(struct ast *left_child, int operation);

struct ast *call_function(char *name, struct ast_list *var_list);
struct ast *call_defined_func(char *name, struct ast_list *var_list);
struct ast *call_obj_method(char *obj_name, char *method,
					struct ast_list *parameter_list);

struct ast *create_cond_operator(struct ast *condition,
					struct ast_list *if_true,
					struct ast_list *if_false);
struct ast *create_if_statement(struct ast *condition, struct ast *stmt);
struct ast *create_else_loop(struct ast *condition,
					struct ast_list *if_block,
					struct ast_list *else_block, int op);
struct ast *create_elseif_else_loop(struct ast *condition,
					struct ast_list *if_block,
					struct ast_list *else_if_block,
					struct ast_list *else_stmts);
struct ast *create_else_if_statement(struct ast *condition,
					struct ast_list *if_block,
					struct ast_list *else_if_block);
struct ast *create_for_loop(struct ast *dec_state, struct ast *expr,
					struct ast *condition,
					struct ast_list *block, int op);
struct ast *create_loop_statement(struct ast *cond_state,
					struct ast_list *block, int op);

struct ast *create_regular(char *reg);
struct ast *call_regular_method(char *regex, char *method,
					struct ast_list *arguments);

void free_array(struct ast *arr);
void free_binary(struct ast *tree);
void free_block(struct ast_list *block);
void free_call_method(struct ast *tree);
void free_case(struct ast *top);
void free_cond(struct ast *tree);
void free_cond_expr(struct ast *tree);
void free_cond_operator(struct ast *tree);
void free_expr(struct ast *tree);
void free_else_if(struct ast_list *list);
void free_function_call(struct ast *tree);
void free_function_dec(struct ast *tree);
void free_loop(struct ast *tree);
void free_method_dec(struct ast *tree);
void free_return(struct ast *tree);
void free_stmt(struct ast *top);
void free_switch(struct ast *tree);
void free_unary(struct ast *tree);
void free_var(struct ast *tree);
void free_var_list(struct ast_list *var_list);

struct ast_list *program_block;

#endif
