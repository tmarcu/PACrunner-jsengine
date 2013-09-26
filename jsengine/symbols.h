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

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <stdint.h>

#define VAR_NUM		0
#define VAR_STRING	1
#define VAR_FUNCTION	2
#define VAR_OBJECT	3

/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL

struct function_info {
	void *func_ptr;
	struct ast_list *statements;
	struct ast_list *args;
	int arg_num;
	int data_type;
};

struct object_method {
	void *fctn_ptr;
	struct ast_list *statements;
	struct ast_list *args;
	struct object_method *next;
	char *name;
};

struct object_dec {
	char *type;
	struct object_method *top;
	struct symbol *property_list;
	struct object_dec *next;
};

struct symvar {
	int type;
	double val;
	char *str;
};

struct symbol {
	char *name;
	int type;
	union {
		struct ast *declared_func;
		struct ast *array;
		struct symvar var;
		struct function_info func;
		struct object_dec obj;
	} value;
	struct symbol *next;
	struct symbol *prev;
};


struct hashtable {
	int size;
	int capacity;
	double load;
	struct symbol **table;
};

/* Malloc wrapper to avoid glib */
extern inline void *jmalloc(size_t size)
{
	void *ptr = calloc(1, size);

	if (ptr == NULL) {
		fprintf(stderr, "Could not allocate memory!\n");
		exit(EXIT_FAILURE);
	}

	return ptr;
}

void add_symbol(const char *name, int type);
struct symbol *get_symbol(const char *name);
void assign_var_symbol(const char *name, double val);
void assign_function_symbol(const char *name, void *func);
void assign_var_str(const char *name, char *str, int index);
void assign_var_array(const char *name, struct ast *arr);
void *get_symbol_value(const char *name);
double addtwo(double val, double num);
void sym_table_init(void);
void remove_sym(char *name);
struct object_method *obj_method(char *type, char *method);
char *get_obj_type(char *obj_name);
void *get_obj_method(char *obj_name, char *method);
void create_new_object(char *obj_type);
void add_method(char *obj_type, void *method_ptr, char *method_name);
void add_property(char *obj_type, int prop_type, char *prop_name);
void add_array(char *name, struct ast *);
void new_add_function(char *name, void *function_ptr);
void free_sym_table(struct hashtable *st);
void free_sym(struct symbol *sym);
int has_sym(char *name);

uint32_t hash_32(char *str, unsigned int bits);
void assign_prop_num(char *obj_name, char *prop_name, double assign_val);
void assign_prop_str(char *obj_name, char *prop_name, char *assign_str);
struct symbol *get_property(struct symbol *obj_sym, char *prop_name);
struct symbol *add_obj_property(struct symbol *obj_sym, char *prop_name);
void add_dec_obj_method(char *obj_name, char *method_name,
			struct ast_list *args, struct ast_list *statements);
void add_object (char *name, char *obj_name);
void free_property_list(struct symbol *list);
void free_sym_table_link(struct symbol *list);
struct object_dec *get_object(char *obj_name);
void eval_prototype(struct ast *obj_name, char *symbol_name);
void print_obj_props(char *obj_name);
struct ast *copy_array_list(struct ast *arr);
void setter_method_str(struct symbol *obj, char *new_str);
struct symbol *wrap_prim(struct symbol *prim);
void clean_sym(struct symbol *sym);
struct ast *set_return_array(struct ast *array);
void assign_var_bool(const char *name, double assign_val);
void free_dec_function(struct ast *dec_fun);
struct ast *get_declared_func(char *name);
void add_new_reg(char *var_name, char *prim_pattern);
void assign_var_num(const char *name, double assign_val);
void assign_var_null(char *sym_name);
void assign_var_reg(char *name, char *reg);
void assign_prop_null(char *obj_name, char *prop_name);

struct hashtable *symtable;

struct object_dec *object_list;

#endif
