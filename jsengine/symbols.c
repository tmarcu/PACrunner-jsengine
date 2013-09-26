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
#include <stdint.h>

#include "symbols.h"
#include "math.h"
#include "tree.h"
#include "spider.h"
#include "jsengine/parser.h"
#include "math_functions.h"
#include "string_methods.h"
#include "array_methods.h"
#include "reg_expr_methods.h"
#include "boolean_methods.h"
#include "javascript_functions.h"

static int HASH_BITS = 9;

static struct hashtable *create_hashtable(int sz);
static void add_back_hash(struct hashtable *ht, uint32_t index, struct symbol *sym);
static void insert_hash(struct hashtable *ht, uint32_t index,
						struct symbol *sym);

static struct hashtable *create_hashtable(int sz)
{
	int i;
	struct hashtable *ht = jmalloc(sizeof(*ht));

	ht->table = jmalloc((sizeof(struct symbol *)) * sz);

	for (i = 0; i < sz; i++)
		ht->table[i] = NULL;

	ht->capacity = sz;
	ht->size = 0;
	ht->load = 0;

	return ht;
}

static void rehash_table(struct hashtable *ht, int sz)
{
	int i;
	int index;
	int cap = sz * 2;
	struct symbol *temp, *temp_next;
	struct hashtable *new_table = jmalloc(sizeof(*new_table));

	HASH_BITS++;

	new_table = create_hashtable(cap);

	for (i = 0; i < ht->capacity; i++) {
		temp = ht->table[i];
		while (temp != NULL) {
			temp_next = temp->next;
			index = hash_32(temp->name, HASH_BITS);
			if (index > cap)
				printf("index bigger than hash\n");
			insert_hash(new_table, index, temp);
			temp = temp_next;
		}
	}

	new_table->load = (double) new_table->size /
					(double) new_table->capacity;
	/*ht->capacity = temp->capacity;
	ht->size = temp->size;
	ht->load = temp->load;
	//memcpy(ht, temp, sizeof(*ht));
	*/

	free(ht->table);
	ht->table = new_table->table;
	ht->capacity = new_table->capacity;
	ht->load = new_table->load;

	free(new_table);
	/*free_sym_table(temp);*/
	printf("Rehashed table Copied:%d Size:%d New Load: %g\n",
					ht->size, ht->capacity, ht->load);
}

static void insert_hash(struct hashtable *ht, uint32_t index,
						struct symbol *sym)
{
	if (ht == NULL) {
		fprintf(stderr, "Hash table does not exist!\n");
		exit(EXIT_FAILURE);
	}

	if (ht->load >= 2) {
		rehash_table(ht, ht->capacity);
		/* Calculate new index since HASH_BITS has updated */
		index = hash_32(sym->name, HASH_BITS);
	}

	/* Shouldn't get here if our rehashing works correctly */
	if (index > (unsigned) ht->capacity) {
		fprintf(stderr, "Attempted to go out of bounds: %d/%d\n",
						index, ht->capacity);
		exit(EXIT_FAILURE);
	}

	sym->prev = NULL;
	sym->next = NULL;
	/* If element already exists in that location */
	if (ht->table[index] != NULL) {
		sym->next = ht->table[index];
		ht->table[index]->prev = sym;
	}
	ht->table[index] = sym;

	ht->size++;
	ht->load = (double) ht->size / (double) ht->capacity;
}



uint32_t hash_32(char *str, unsigned int bits)
{
	uint32_t hash = 0;
	uint32_t c;

	while ((c = *str++))
		hash += ((hash << 5) + c) * GOLDEN_RATIO_PRIME_32;

	/* High bits are more random, so use them. */
	return (hash >> (32 - bits))/5;
}


/*
 * used by the rehash function to preserve the hash tables scope
 * because it is only used by the hash table it does not make checks to
 * see if the load is too great or the index is greater than capacity
 */
static void add_back_hash(struct hashtable *ht, uint32_t index,
						struct symbol *sym)
{
	struct symbol *temp;
	temp = ht->table[index];
	sym->next = NULL;
	if (temp == NULL) {
		ht->table[index] = sym;
		return;
	}

	while (temp->next != NULL)
		temp = temp->next;

	temp->next = sym;
	sym->prev = temp;
}




void add_symbol(const char *name, int type)
{
	struct symbol *sym = jmalloc(sizeof(struct symbol));
	sym->name = strdup(name);
	sym->type = type;
	sym->prev = NULL;
	sym->next = NULL;

	insert_hash(symtable, hash_32((char *) name, HASH_BITS), sym);
}

struct symbol *get_symbol(const char *name)
{
	struct symbol *sym;

	if (symtable == NULL) {
		fprintf(stderr, "Table does not exist!!\n");
		exit(-49);
	}

	/* To handle declerations without 'var' prefix: */
	if (symtable->table[hash_32((char *) name, HASH_BITS)] != NULL) {
		sym = symtable->table[hash_32((char *)name, HASH_BITS)];
		while (sym != NULL && strcmp(sym->name, name) != 0)
			sym = sym->next;
		if (sym != NULL)
			return sym;

		add_symbol(name, VAR_NUM);
	} else {
		add_symbol(name, VAR_NUM);
	}
	return get_symbol(name);
}

void add_new_dec_function(struct ast *dec_fun)
{
	struct symbol *temp;
	add_symbol(dec_fun->data.func.name, VAR_FUNCTION);
	temp = get_symbol(dec_fun->data.func.name);
	temp->value.declared_func = dec_fun;
	temp = get_symbol(dec_fun->data.func.name);
}

void assign_var_num(const char *name, double assign_val)
{
	struct symbol *sym = get_symbol(name);

	if (sym == NULL) {
		fprintf(stderr, "Symbol does not exist\n");
		return;
	}
	sym->value.var.val = assign_val;
	sym->value.var.type = NUM;
	sym->type = VAR_NUM;
}

void free_dec_function(struct ast *dec_fun)
{
	free(dec_fun->data.func.name);
	free_block(dec_fun->data.func.statements);
	free_block(dec_fun->data.func.parameters);
	free(dec_fun);
}

struct ast *get_declared_func(char *name)
{
	struct symbol *ptr;

	ptr = get_symbol(name);
	if (ptr == NULL) {
		fprintf(stderr, "Error null pointer\n");
		exit(EXIT_FAILURE);
	}

	return ptr->value.declared_func;
}


void assign_var_bool(const char *name, double assign_val)
{
	struct symbol *sym = get_symbol(name);
	clean_sym(sym);
	if (sym == NULL) {
		fprintf(stderr, "Symbol does not exist\n");
		return;
	}
	if (sym->type == VAR_OBJECT && strcmp(sym->value.obj.type, "Boolean")
								== 0) {
		struct symbol *temp = get_property(sym, "value");
		temp->value.var.val = assign_val;
		if (temp->value.var.str != NULL)
			free(temp->value.var.str);
		temp->value.var.str = assign_val ? strdup("true") :
							strdup("false");
		return;
	}

	sym->value.var.val = assign_val;
	sym->value.var.type = BOOL;
	sym->type = VAR_STRING;
	sym->value.var.str = assign_val ? strdup("true") : strdup("false");
}

void setter_method_str(struct symbol *sym, char *new_str)
{
	void (*func)(struct symbol *, char *);
	struct object_method *temp;
	temp = obj_method(sym->value.obj.type, "setter");
	if (temp->fctn_ptr != NULL) {
		func = temp->fctn_ptr;
		func(sym, new_str);
	}
}

struct symbol *wrap_prim(struct symbol *prim)
{
	struct symbol *wrap = jmalloc(sizeof(*wrap));
	struct symbol *temp;
	if (prim->type == VAR_STRING) {
		wrap->type = VAR_OBJECT;
		wrap->value.obj.type = strdup("String");
		wrap->value.obj.property_list = add_obj_property(wrap, "value");
		wrap->value.obj.property_list->type = VAR_STRING;
		wrap->value.obj.property_list->value.var.str =
					strdup(prim->value.var.str);
		temp = add_obj_property(wrap, "length");
		wrap->value.obj.property_list->next = temp;
		temp->prev = wrap->value.obj.property_list;
		temp->value.var.val = (double) strlen(prim->value.var.str);
		temp->next = NULL;
	} else {
		printf("Primitive cannot be wrapped\n");
	}
	return wrap;
}

void assign_var_reg(char *name, char *reg)
{
	struct symbol *sym = get_symbol(name);
	clean_sym(sym);
	sym->type = REG_EXP;
	sym->value.var.str = strdup(reg);
}

void assign_var_str(const char *name, char *str, int index)
{
	struct symbol *sym = get_symbol(name);

	if (sym == NULL) {
		fprintf(stderr, "Symbol does not exist\n");
		return;
	}

	if (sym->type == ARRAY) {
		sym->value.array->type = VAR_STRING;
		struct ast *arr_el;
		arr_el = sym->value.array->data.array.list[index];
		if (arr_el->data.var.type == STRING_LITERAL) {
			if (arr_el->data.var.string != NULL)
				free(arr_el->data.var.string);
		}

		arr_el->data.var.string = strdup(str);
		arr_el->type = VAR;
		arr_el->data.var.type = STRING_LITERAL;
		return;
	} else if (sym->type == VAR_OBJECT) {
		setter_method_str(sym, str);
		return;
	} else {
		sym->type = VAR_STRING;
		if (sym->value.var.str != NULL)
			free(sym->value.var.str);
		sym->value.var.type = STRING_LITERAL;
		if (str != NULL)
			sym->value.var.str = strdup(str);
		else
			sym->value.var.str = NULL;
	}
}

void assign_var_array(const char *name, struct ast *arr)
{
	struct symbol *sym = get_symbol(name);
	int i;

	if (sym == NULL) {
		fprintf(stderr, "Symbol does not exist\n");
		return;
	}

	clean_sym(sym);

	sym->type = ARRAY;
	sym->value.array = jmalloc(sizeof(*sym->value.array));
	sym->value.array->data.array.list = jmalloc(
				sizeof(*sym->value.array->data.array.list)
						* arr->data.array.size);

	for (i = 0; i < arr->data.array.size; i++) {
		sym->value.array->data.array.list[i] = jmalloc(
			sizeof(*sym->value.array->data.array.list[i]));
		memcpy(sym->value.array->data.array.list[i],
				arr->data.array.list[i],
				sizeof(*arr->data.array.list[i]));
		if (arr->data.array.list[i]->data.var.type == STRING_LITERAL)
			sym->value.array->data.array.list[i]->data.var.string
			= strdup(arr->data.array.list[i]->data.var.string);
	}
	sym->value.array->data.array.size = arr->data.array.size;
	sym->value.array->type = ARRAY;
}

struct ast *copy_array_list(struct ast *arr)
{
	int i;
	struct ast *retval = jmalloc(sizeof(*retval));
	retval->data.array.list = jmalloc(sizeof(struct ast *) *
						arr->data.array.size);
	for (i = 0; i < arr->data.array.size; i++) {
		retval->data.array.list[i] =
			       eval_expr(arr->data.array.list[i], NULL);

	}
	retval->data.array.size = arr->data.array.size;
	retval->type = ARRAY;
	return retval;
}

struct ast *set_return_array(struct ast *arr)
{
	int i;
	struct ast *retval = jmalloc(sizeof(*retval));
	retval->type = ARRAY;
	retval->data.array.list = jmalloc(sizeof(*retval->data.array.list)
						* arr->data.array.size);
	for (i = 0; i < arr->data.array.size; i++) {
		retval->data.array.list[i] =
				jmalloc(sizeof(*retval->data.array.list[i]));
		memcpy(retval->data.array.list[i], arr->data.array.list[i],
							sizeof(struct ast));
		if (arr->data.array.list[i]->data.var.type == STRING_LITERAL)
			retval->data.array.list[i]->data.var.string =
			strdup(arr->data.array.list[i]->data.var.string);
	}
	retval->data.array.size = arr->data.array.size;
	return retval;
}

void assign_function_symbol(const char *name, void *func)
{
	struct symbol *sym = get_symbol(name);

	if (sym == NULL) {
		fprintf(stderr, "Symbol does not exist\n");
		return;
	}
	sym->type = VAR_FUNCTION;
	sym->value.func.func_ptr = func;
}

void *get_symbol_value(const char *name)
{
	struct symbol *sym = get_symbol(name);

	if (sym == NULL) {
		fprintf(stderr, "Symbol does not exist %s\n", name);
		return NULL;
	}
	if (sym->type == VAR_NUM)
		return &sym->value.var.val;
	else if (sym->type == VAR_STRING)
		return sym->value.var.str;
	else if (sym->type == VAR_FUNCTION)
		return sym->value.func.func_ptr;

	return NULL;
}



void add_array(char *name, struct ast *arr)
{
	struct symbol *sym = jmalloc(sizeof(*sym));

	sym->name = strdup(name);
	sym->type = ARRAY;
	sym->value.array = copy_array_list(arr);
	sym->prev = NULL;
	sym->next = NULL;

	insert_hash(symtable, hash_32((char *) name, HASH_BITS), sym);
}

void new_add_function(char *name, void *function_ptr)
{
	struct symbol *sym = jmalloc(sizeof(*sym));

	sym->name = strdup(name);
	sym->type = VAR_FUNCTION;
	sym->value.func.func_ptr = function_ptr;
	sym->prev = NULL;
	sym->next = NULL;

	insert_hash(symtable, hash_32((char *) name, HASH_BITS), sym);

}

double addtwo(double i, double g)
{
	return i + g;
}

double d_abs(double i)
{
	if (i < 0)
		return -i;
	else
		return i;
}

void remove_sym(char *name)
{
	struct symbol *sym = get_symbol(name);
	if (sym == NULL) {
		printf("symbol is null\n");
		return;
	};

	if (symtable->table[hash_32((char *)name, HASH_BITS)] == sym)
		symtable->table[hash_32((char *)name, HASH_BITS)] = sym->next;

	if (sym->prev != NULL)
		sym->prev->next = sym->next;
	if (sym->next != NULL)
		sym->next->prev = sym->prev;
	free_sym(sym);
}

struct object_method *obj_method(char *obj_type, char *method)
{
	struct object_dec *temp = object_list;
	while (temp != NULL) {
		if (strcmp(temp->type, obj_type) == 0) {
			struct object_method *method_itr;
			method_itr = temp->top;
			while (method_itr != NULL) {
				if (strcmp(method, method_itr->name) == 0)
					return method_itr;
				method_itr = method_itr->next;
			}
		}
		temp = temp->next;
	}
	fprintf(stderr, "did not find method\n");

	return NULL;
}

void object_set_prototype(char *proto_name, struct object_dec *parent)
{
	struct ast *temp_proto = get_declared_func(proto_name);
	struct symbol *temp = parent->property_list;
	while (temp != NULL && strcmp(temp->name, "prototype") != 0)
		temp = temp->next;

	if (temp == NULL)
		printf("ERROR OBJECT DOES NOT HAVE PROTOTYPE\n");

	temp->value.func.func_ptr = NULL;
	temp->value.func.args = temp_proto->data.func.parameters;
	temp->value.func.statements = temp_proto->data.func.statements;

	remove_sym(proto_name);
}

struct symbol *get_prototype(char *obj_name)
{
	struct object_dec *obj = get_object(obj_name);
	struct symbol *retval;
	retval = obj->property_list;
	while (retval != NULL && strcmp(retval->name, "prototype") != 0)
		retval = retval->next;

	if (retval == NULL)
		printf("Could not get prototype\n");

	return retval;
}

void print_props(char *sym_name)
{
	struct symbol *obj = get_symbol(sym_name);
	struct symbol *temp = obj->value.obj.property_list;
	printf("Object: %s\n", obj->name);
	while (temp != NULL) {
		printf("Prop: %s\n", temp->name);
		temp = temp->next;
	}
}

void eval_prototype(struct ast *obj_name, char *symbol_name)
{
	struct ast *retval;
	struct symbol *proto = get_prototype(obj_name->data.func.name);

	if (proto->value.func.func_ptr == NULL) {
		add_parameter_vars(proto->value.func.args,
					obj_name->data.func.parameters);
		retval = statements_spider_func(proto->value.func.statements,
								symbol_name);
		free_stmt(retval);
		remove_parameter_vars(proto->value.func.args);
	} else {
		void (*func_proto)(char *, struct ast_list *);
		func_proto = proto->value.func.func_ptr;
		(*func_proto)(symbol_name, obj_name->data.func.parameters);
	}
}

int object_has_prototype(struct object_dec *parent)
{
	struct symbol *list;
	list = parent->property_list;
	while (list != NULL && strcmp(list->name, "prototype") != 0)
		list = list->next;


	if (list == NULL || (list->value.func.func_ptr == NULL &&
				list->value.func.statements == NULL))
		return 0;

	return 1;
}

void add_object(char *name, char *obj_name)
{
	struct symbol *temp;
	struct object_dec *parent;
	add_symbol(name, VAR_OBJECT);
	temp = get_symbol(name);
	parent = get_object(obj_name);
	if (parent == NULL) {
		create_new_object(obj_name);
		parent = get_object(obj_name);
		object_set_prototype(obj_name, parent);
	} else if (object_has_prototype(parent) == 0) {
		object_set_prototype(obj_name, parent);
	}
	temp->value.obj.type = strdup(obj_name);
	temp->value.obj.property_list = NULL;
}

void add_new_reg(char *var_name, char *prim_pattern)
{
	add_symbol(var_name, VAR_OBJECT);
	char *temp_pat, *temp_flags;
	struct symbol *sym = get_symbol(var_name);
	sym->value.obj.property_list = NULL;
	(void) add_obj_property(sym, "pattern");
	(void) add_obj_property(sym, "flags");
	(void) add_obj_property(sym, "lastIndex");
	temp_pat = get_prim_pattern(prim_pattern);
	temp_flags = get_prim_flags(prim_pattern);
	sym->value.obj.type = strdup("RegExp");
	assign_prop_str(var_name, "pattern", temp_pat);
	if (temp_flags != NULL) {
		assign_prop_str(var_name, "flags", temp_flags);
		free(temp_flags);
	}

	assign_prop_num(var_name, "lastIndex", 0);
	free(temp_pat);
}

char *get_obj_type(char *obj_name)
{
	struct symbol *temp = get_symbol(obj_name);
	switch (temp->type) {
	case VAR_NUM:
		return "Number";
	break;
	case VAR_STRING:
		return "String";
	break;
	case VAR_OBJECT:
		return temp->value.obj.type;
	break;
	case ARRAY:
		return "Array";
	break;
	case REG_EXP:
		return "RegExp";
	break;
	default:
		printf("object not recognized\n");
		return NULL;
	break;
	}
}

void *get_obj_method(char *obj_name, char *method)
{
	struct object_method *temp = obj_method(get_obj_type(obj_name), method);
	return temp;
}

void create_new_object(char *obj_name)
{
	struct object_dec *new_obj = jmalloc(sizeof(*new_obj));
	struct symbol *prop_list = jmalloc(sizeof(*prop_list));
	prop_list->name = strdup("prototype");
	prop_list->prev = NULL;
	prop_list->next = NULL;
	new_obj->type = strdup(obj_name);
	new_obj->top = NULL;
	new_obj->property_list = prop_list;
	if (object_list == NULL) {
		object_list = new_obj;
		new_obj->next = NULL;
	} else {
		new_obj->next = object_list;
		object_list = new_obj;
	}
}

void add_method(char *obj_type, void *method_ptr, char *method_name)
{
	struct object_method *new_method = jmalloc(sizeof(*new_method));

	new_method->fctn_ptr = method_ptr;
	new_method->name = strdup(method_name);
	new_method->statements = NULL;
	new_method->args = NULL;
	struct object_dec *object_itr = object_list;
	while (object_itr != NULL) {
		if (strcmp(object_itr->type, obj_type) == 0) {
			new_method->next = object_itr->top;
			object_itr->top = new_method;
			return;
		}
		object_itr = object_itr->next;
	}
	fprintf(stderr, "Could not find object\n");
}

void add_dec_obj_method(char *obj_name, char *method_name,
		struct ast_list *args, struct ast_list *statements)
{
	struct object_dec *object_itr;
	struct object_method *new_method = jmalloc(sizeof(*new_method));
	new_method->fctn_ptr = NULL;
	new_method->name = strdup(method_name);
	new_method->args = args;
	new_method->statements = statements;

	object_itr = object_list;

	while (object_itr != NULL) {
		if (strcmp(object_itr->type, obj_name) == 0) {
			new_method->next = object_itr->top;
			object_itr->top = new_method;
			return;
		}
		object_itr = object_itr->next;
	}

	create_new_object(obj_name);
	add_dec_obj_method(obj_name, method_name, args, statements);
}


void add_property(char *obj_type, int prop_type, char *prop_name)
{
	struct symbol *new_prop = jmalloc(sizeof(*new_prop));
	struct object_dec *object_itr = object_list;

	new_prop->type = prop_type;
	new_prop->name = strdup(prop_name);

	while (object_itr != NULL) {
		if (strcmp(object_itr->type, obj_type) == 0) {
			new_prop->next = object_itr->property_list;
			object_itr->property_list = new_prop;
			return;
		}
		object_itr = object_itr->next;
	}
	printf("could not find object\n");
}

void assign_var_null(char *sym_name)
{
	struct symbol *sym = get_symbol(sym_name);
	clean_sym(sym);

	sym->type = NULL_TYPE;
}

void assign_prop_null(char *obj_name, char *prop_name)
{
	struct symbol *obj_sym, *prop_sym;
	obj_sym = get_symbol(obj_name);
	prop_sym = get_property(obj_sym, prop_name);
	if (prop_sym == NULL)
		prop_sym = add_obj_property(obj_sym, prop_name);


	if (prop_sym->type == ARRAY && prop_sym->value.array != NULL)
		free_array(prop_sym->value.array);
	else if (prop_sym->type == VAR_OBJECT) {
		free_property_list(prop_sym->value.obj.property_list);
		if (prop_sym->value.obj.type != NULL)
			free(prop_sym->value.obj.type);
	}

	prop_sym->type = NULL_TYPE;
}


void assign_prop_num(char *obj_name, char *prop_name, double assign_val)
{
	struct symbol *obj_sym, *prop_sym;
	obj_sym = get_symbol(obj_name);
	prop_sym = get_property(obj_sym, prop_name);
	if (prop_sym == NULL)
		prop_sym = add_obj_property(obj_sym, prop_name);

	prop_sym->type = VAR_NUM;
	prop_sym->value.var.val = assign_val;
	prop_sym->value.var.type = NUM;
}

void assign_prop_str(char *obj_name, char *prop_name, char *assign_str)
{
	struct symbol *obj_sym, *prop_sym;
	obj_sym = get_symbol(obj_name);
	prop_sym = get_property(obj_sym, prop_name);
	if (prop_sym == NULL)
		prop_sym = add_obj_property(obj_sym, prop_name);

	clean_sym(prop_sym);
	prop_sym->type = VAR_STRING;
	prop_sym->value.var.type = STRING_LITERAL;
	prop_sym->value.var.str = strdup(assign_str);
}

struct symbol *get_property(struct symbol *obj_sym, char *prop_name)
{
	struct symbol *cur;
	cur = obj_sym->value.obj.property_list;
	while (cur != NULL) {
		if (strcmp(prop_name, cur->name) == 0)
			return cur;

		cur = cur->next;
	}
	return NULL;
}

struct symbol *add_obj_property(struct symbol *obj_sym, char *prop_name)
{
	struct symbol *new_sym = jmalloc(sizeof(*new_sym));
	new_sym->type = VAR_NUM;
	new_sym->name = strdup(prop_name);
	new_sym->next = obj_sym->value.obj.property_list;
	obj_sym->value.obj.property_list = new_sym;
	return new_sym;
}


void free_sym(struct symbol *sym)
{
	if (sym == NULL)
		return;
	switch (sym->type) {
	case VAR_NUM:
		if (sym->name != NULL)
			free(sym->name);
		free(sym);
	break;
	case VAR_STRING:
		if (sym->name != NULL)
			free(sym->name);
		if (sym->value.var.str != NULL)
			free(sym->value.var.str);
		free(sym);
	break;
	case VAR_FUNCTION:
		if (sym->name != NULL)
			free(sym->name);
		free(sym);
	break;
	case FUNCTION_DEC:
		if (sym->name != NULL)
			free(sym->name);
		free(sym);
	break;
	case ARRAY:
		if (sym->name != NULL)
			free(sym->name);
		if (sym->value.array != NULL)
			free_array(sym->value.array);
		free(sym);
	break;
	case VAR_OBJECT:
		if (sym->name != NULL)
			free(sym->name);
		free_property_list(sym->value.obj.property_list);
		if (sym->value.obj.type != NULL)
			free(sym->value.obj.type);
		free(sym);
	break;
	case BOOL:
		if (sym->name != NULL)
			free(sym->name);
		if (sym->value.var.str != NULL)
			free(sym->value.var.str);
		free(sym);
	break;
	case NULL_TYPE:
		free(sym);
	break;
	default:
		printf("Symbol type: %d cannot be freed yet\n", sym->type);
	break;
	}
}

/*
 * gets rid off all data the symbol points to except for name
 * and next/prev
 */
void clean_sym(struct symbol *sym)
{
	if (sym->type == 0)
		return;
	switch (sym->type) {
	case VAR_STRING:
		if (sym->value.var.str != NULL)
			free(sym->value.var.str);
	break;
	case ARRAY:
		if (sym->value.array != NULL)
			free_array(sym->value.array);
	break;
	case VAR_OBJECT:
		free_property_list(sym->value.obj.property_list);
		if (sym->value.obj.type != NULL)
			free(sym->value.obj.type);
	break;
	default:
	break;
	}
}

void free_property_list(struct symbol *list)
{
	if (list == NULL)
		return;
	struct symbol *temp, *temp_next;
	temp = list;
	while (temp != NULL) {
		temp_next = temp->next;
		free_sym(temp);
		temp = temp_next;
	}
}

void free_obj_methods(struct object_method *method)
{
	struct object_method *temp, *temp_next;
	temp = method;
	while (temp != NULL) {
		temp_next = temp->next;
		if (temp->name != NULL)
			free(temp->name);
		free(temp);
		temp = temp_next;
	}
}

void free_obj_prop(struct symbol *list)
{
	struct symbol *temp, *temp_next;
	temp = list;
	while (temp != NULL) {
		temp_next = temp->next;
		free_sym(temp);
		temp = temp_next;
	}
}


void free_obj(struct object_dec *obj)
{
	free_obj_methods(obj->top);
	free_obj_prop(obj->property_list);
	if (obj->type != NULL)
		free(obj->type);
	free(obj);
}

void free_obj_list(void)
{
	struct object_dec *temp, *temp_next;
	temp = object_list;
	while (temp != NULL) {
		temp_next = temp->next;
		free_obj(temp);
		temp = temp_next;
	}
}

void free_sym_table(struct hashtable *st)
{
	int i;

	for (i = 0; i < st->capacity; i++)
			free_sym_table_link(st->table[i]);


	free(st->table);
	free(st);
	free_obj_list();
}

void free_sym_table_link(struct symbol *list)
{
	struct symbol *temp, *temp_next;
	temp = list;
	while (temp != NULL) {
		temp_next = temp->next;
		free_sym(temp);
		temp = temp_next;
	}
}

struct object_dec *get_object(char *obj_name)
{
	struct object_dec *itr;
	itr = object_list;
	while (itr != NULL && strcmp(itr->type, obj_name) != 0)
		itr = itr->next;


	if (itr != NULL)
		return itr;
	else
		fprintf(stderr, "Object not found\n");
	return NULL;
}

/*sets property for object prototype, not an object declared by
 * the author of the code
 */

void set_property(char *obj_name, struct symbol *prop, char *prop_name)
{
	struct object_dec *obj;
	struct symbol *temp;

	obj = get_object(obj_name);
	temp = obj->property_list;

	while (temp != NULL && strcmp(temp->name, prop_name) != 0)
		temp = temp->next;

	if (temp == NULL)
		return;

	if (prop->value.func.func_ptr != NULL) {
		temp->value.func.func_ptr = prop->value.func.func_ptr;
		temp->value.func.args = NULL;
		temp->value.func.statements = NULL;
	} else {
		temp->value.func.func_ptr = NULL;
		temp->value.func.args = prop->value.func.args;
		temp->value.func.statements = prop->value.func.statements;
	}
	free_sym(prop);
}

struct symbol *create_function_prop(void *proto)
{
	struct symbol *ret = jmalloc(sizeof(*ret));
	ret->type = VAR_FUNCTION;
	ret->value.func.func_ptr = proto;
	ret->value.func.args = NULL;
	ret->value.func.statements = NULL;

	return ret;
}

void print_obj_props(char *obj_name)
{
	struct object_dec *obj = get_object(obj_name);
	struct symbol *temp = obj->property_list;
	while (temp != NULL) {
		printf("prop %s\n", temp->name);
		temp = temp->next;
	}
}

void sym_table_init(void)
{
	symtable = create_hashtable(110);

	new_add_function("Math.addtwo", math_addtwo);
	new_add_function("Math.cos", math_cos);
	new_add_function("Math.abs", math_abs);
	new_add_function("Math.acos", math_acos);
	new_add_function("Math.asin", math_asin);
	new_add_function("Math.atan", math_atan);
	new_add_function("Math.atan2", math_atan2);
	new_add_function("Math.ceil", math_ceil);
	new_add_function("Math.floor", math_floor);
	new_add_function("Math.log", math_log);
	new_add_function("Math.pow", math_pow);
	new_add_function("Math.round", math_round);
	new_add_function("Math.sqrt", math_sqrt);
	new_add_function("Math.tan", math_tan);
	new_add_function("Math.exp", math_exp);
	new_add_function("Math.max", math_max);
	new_add_function("Math.min", math_min);
	new_add_function("parseInt", parse_int);
	new_add_function("toNumber", to_number);
	new_add_function("isNaN", is_nan);
	new_add_function("alert", alert);
	new_add_function("dnsDomainIs", dns_domain_is);
	new_add_function("dnsDomainLevels", dns_domain_levels);
	new_add_function("isInNet", is_in_net);
	new_add_function("isPlainHostName", is_plain_hostname);
	new_add_function("localHostOrDomainIs", local_host_or_domain_is);
	new_add_function("shExpMatch", sh_exp_match);
	new_add_function("myIpAddress", my_ip_address);
	new_add_function("dnsResolve", dns_resolve);
	new_add_function("isResolvable", is_resolvable);
	new_add_function("weekdayRange", weekday_range);
	new_add_function("timeRange", time_range);
	new_add_function("dateRange", date_range);

	create_new_object("Array");
	add_method("Array", array_reverse, "reverse");
	add_method("Array", array_concat, "concat");
	add_method("Array", array_join, "join");
	add_method("Array", array_last_index_of, "lastIndexOf");
	add_method("Array", array_index_of, "indexOf");

	create_new_object("String");
	set_property("String", create_function_prop(string_prototype),
							"prototype");
	add_property("String", VAR_STRING, "value");
	add_method("String", string_concat, "concat");
	add_method("String", string_char_at, "charAt");
	add_method("String", string_char_code_at, "charCodeAt");
	add_method("String", string_contains, "contains");
	add_method("String", string_ends_with, "endsWith");
	add_method("String", string_index_of, "indexOf");
	add_method("String", string_last_index_of, "lastIndexOf");
	add_method("String", string_local_compare, "localCompare");
	add_method("String", string_print_hello, "printHello");
	add_method("String", string_replace, "replace");
	add_method("String", string_splice, "splice");
	add_method("String", string_split, "split");
	add_method("String", string_substr, "substr");
	add_method("String", string_substring, "substring");
	add_method("String", string_setter, "setter");
	add_method("String", string_match, "match");
	add_method("String", string_search, "search");

	create_new_object("RegExp");
	set_property("RegExp", create_function_prop(reg_prototype),
							"prototype");
	add_property("RegExp", VAR_NUM, "lastIndex");
	add_method("RegExp", reg_test, "test");
	add_method("RegExp", reg_exec, "exec");


	create_new_object("Boolean");
	set_property("Boolean", create_function_prop(boolean_prototype),
								"prototype");
	add_property("Boolean", VAR_STRING, "value");
	add_method("Boolean", bool_valueof, "valueOf");
	add_method("Boolean", bool_tostring, "toString");
}
