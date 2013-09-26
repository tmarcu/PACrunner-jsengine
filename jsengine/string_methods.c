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
#include "symbols.h"
#include "tree.h"
#include "spider.h"
#include "jsengine/parser.h"
#include "string_methods.h"
#include "reg_expr_methods.h"

void string_setter(struct symbol *obj, char *new_str)
{
	assign_prop_str(obj->name, "value", new_str);
	assign_prop_num(obj->name, "length", (double) strlen(new_str));
}

void string_prototype(char *symbol_name, struct ast_list *arg_list)
{ 
	struct symbol *sym = get_symbol(symbol_name);
	struct ast *val = eval_expr(arg_list->top, NULL);

	(void) add_obj_property(sym, "value");
	assign_prop_str(symbol_name, "value", val->data.var.string);

	(void) add_obj_property(sym, "length");
	assign_prop_num(symbol_name, "length", (double) strlen(val->data.var.string));
	
	free_expr(val);
}

struct ast *string_concat(char *name, struct ast_list *arguments)
{
	struct symbol *object = get_symbol(name);
	char *obj_type = get_obj_type(name);
	char *new_string;
	struct ast *arg, *retval;
	char *string = get_string_value(object);

	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}

	arg = eval_expr(arguments->top, NULL);
	new_string = jmalloc(strlen(string) + strlen(arg->data.var.string) + 1);
	strcat(new_string, string);
	strcat(new_string, arg->data.var.string);
	new_string[strlen(string) + strlen(arg->data.var.string)] = '\0';
	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = STRING_LITERAL;
	retval->data.var.string = new_string;
	retval->next = NULL;
	free_expr(arg);

	return retval;
}

struct ast *string_char_at(char *name, struct ast_list *arguments)
{
	struct ast *arg, *retval;
	struct symbol *object = get_symbol(name);
	char *obj_type = get_obj_type(name);
	char *char_at;
	char temp_char;

	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}
	
	char *string = get_string_value(object);

	arg = eval_expr(arguments->top, NULL);
	temp_char = string[(int) arg->data.var.val];
	char_at = jmalloc(sizeof(char));
	*char_at = temp_char;
	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = STRING_LITERAL;
	retval->data.var.string = char_at;
	retval->next = NULL;
	
	free_expr(arg);
	
	return retval;
}

struct ast *string_char_code_at(char *name, struct ast_list *arguments)
{
	struct ast *arg, *retval;
	struct symbol *object = get_symbol(name);
	char *obj_type = get_obj_type(name);
	char temp_char;

	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}
	char *string = get_string_value(object);
	
	arg = eval_expr(arguments->top, NULL);
	temp_char = string[(int) arg->data.var.val];
	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = (double) temp_char;
	retval->next = NULL;
	
	free_expr(arg);
	
	return retval;
}

struct ast *string_contains(char *name, struct ast_list *arguments)
{
	struct symbol *object = get_symbol(name);
	struct ast *arg_one, *arg_two, *retval;
	char *obj_type = get_obj_type(name);

	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}
	char *string = get_string_value(object);
	
	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->next = NULL;
	arg_one = eval_expr(arguments->top, NULL);
	if (arguments->top->next == NULL) {
		if (strstr(string, arg_one->data.var.string) != NULL)
			retval->data.var.val = 1;
		else
			retval->data.var.val = 0;
	} else {
		
		arg_two = eval_expr(arguments->top->next, NULL);
		int index = (int) arg_two->data.var.val;
		if (strstr(&(string[index]), arg_one->data.var.string) != NULL)
			retval->data.var.val = 1;
		else
			retval->data.var.val = 0;
		free_expr(arg_two);
	}
	free_expr(arg_one);
	return retval;
}

int get_length(char *str)
{
	int i = 0;
	while (str[i] != '\0')
		i++;
	return i - 1;
}

struct ast *string_ends_with(char *name, struct ast_list *arguments)
{
	struct symbol *object = get_symbol(name);
	char *obj_type = get_obj_type(name);
	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}
	struct ast *arg_one, *arg_two, *retval;
	int arg_length;
	
	char *string = get_string_value(object);
	
	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->next = NULL;
	arg_one = eval_expr(arguments->top, NULL);
	arg_length = get_length(arg_one->data.var.string);
	if (arguments->top->next == NULL) {
		int str_len = get_length(string);
		while (string[str_len] == arg_one->data.var.string[arg_length] && arg_length >= 0) {
			str_len--;
			arg_length--;
		}
		if (arg_length == -1)
			retval->data.var.val = 1;
		else
			retval->data.var.val = 0;
	} else {
		int top;
		arg_two = eval_expr(arguments->top->next, NULL);
		top = (int) arg_two->data.var.val;
		while (string[top] == arg_one->data.var.string[arg_length] && arg_length >= 0) {
			top--;
			arg_length--;
		}
		if (arg_length == -1)
			retval->data.var.val = 1;
		else
			retval->data.var.val = 0;
		free_expr(arg_two);
	}
	free_expr(arg_one);
	return retval;
}

struct ast *string_index_of(char *name, struct ast_list *arguments)
{
	struct symbol *object = get_symbol(name);
	char *obj_type = get_obj_type(name);
	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}
	
	char *string = get_string_value(object);
	
	struct ast *arg_one, *arg_two, *retval;
	char *substring;
	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->next = NULL;
	arg_one = eval_expr(arguments->top, NULL);
	if (arguments->top->next == NULL) {
		substring = strstr(string, arg_one->data.var.string);
		if (substring != NULL) {
			int i = 0;
			while ((char *) substring != (char *) &string[i]) {
				i++;
			}
			retval->data.var.val = (double) i;
		} else {
			retval->data.var.val = -1;
		}
	} else {
		arg_two = eval_expr(arguments->top->next, NULL);
		int index = (int) arg_two->data.var.val;
		substring = strstr(&(string[index]), arg_one->data.var.string);
		if (substring != NULL) {
			int i = 0;
			while (substring != &string[index + i]) {
				i++;
			}
			retval->data.var.val = (double) i;
		} else {
			retval->data.var.val = -1;
		}
		free_expr(arg_two);
	}
	free_expr(arg_one);
	return retval;
}

char *reverse_string(char *str, int size)
{
	char *ret_str = jmalloc(sizeof(char) * size);
	int i, k;
	i = size - 1;
	k = 0;
	while (k < size) {
		ret_str[k] = str[i];
		k++;
		i--;
	}
	return ret_str;
}
	
struct ast *string_last_index_of(char *name, struct ast_list *arguments)
{
	struct symbol *object = get_symbol(name);
	char *obj_type = get_obj_type(name);
	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}
	struct ast *arg_one, *arg_two, *retval;
	char *substring;
	char *test_string;
	char *search_string;
	
	char *string = get_string_value(object);
	
	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->next = NULL;
	arg_one = eval_expr(arguments->top, NULL);
	search_string = reverse_string(arg_one->data.var.string, strlen(arg_one->data.var.string));
	if (arguments->top->next == NULL) {
		test_string = reverse_string(string, strlen(string));
		substring = strstr(test_string, search_string);
		if (substring != NULL) {
			int i = 0;
			while ((char *) substring != (char *) &test_string[i] ) {
				i++;
			}
			retval->data.var.val = (double) (strlen(string) - i - 1);
		} else {
			retval->data.var.val = -1;
		}
	} else {
		arg_two = eval_expr(arguments->top->next, NULL);
		int index = (int) arg_two->data.var.val;
		test_string = reverse_string(string, index);
		substring = strstr(test_string, search_string);
		if (substring != NULL) {
			int i = 0;
			while ((char *) substring != (char *) &test_string[i]) {
				i++;
			}
			retval->data.var.val = (double) (strlen(string) - i - index - 1);
		} else {
			retval->data.var.val = -1;
		}
		free_expr(arg_two);
	}
	free(search_string);
	free(test_string);
	free_expr(arg_one);
	return retval;
}

/*If they are not exactly the same is should compare to where they would
 * be placed in the Hash table. The hash function is private right now
 * so strcmp is what we are going to use until we want to implement more
 */
struct ast *string_local_compare(char *name, struct ast_list *arguments)
{
	struct symbol *object = get_symbol(name);
	char *obj_type = get_obj_type(name);
	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}
	struct ast *arg_one, *retval;
	
	char *string = get_string_value(object);
	
	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->next = NULL;
	arg_one = eval_expr(arguments->top, NULL);
	retval->data.var.val = (double) strcmp(arg_one->data.var.string, string);
	free_expr(arg_one);
	
	return retval;
}

struct ast *string_print_hello(void)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = STRING_LITERAL;
	retval->next = NULL;
	retval->data.var.string = strdup("hello world");
	return retval;
}

int how_many_substrings(char *string, char *sub_string)
{
	int i, length, num_of_sub;
	char *temp;
	length = strlen(string);
	i = 0;
	num_of_sub = 0;
	temp = strstr(string, sub_string);
	while (temp != NULL && i < length) {
		num_of_sub++;
		while ((char *) temp != (char *) &string[i]) {
			i++;
		}
		i += strlen(sub_string);
		temp = strstr((char *) &string[i], sub_string);
	}
	return num_of_sub;
}
		
int is_reg_expr(struct ast *tree)
{
	if (tree->type == REG_EXP)
		return 1;
	if (tree->type == STRING_LITERAL)
		return 0;
		
	struct symbol *temp = get_symbol(tree->data.var.name);
	if (strcmp(temp->value.obj.type, "RegExp") == 0)
		return 1;
	
	printf("This is not a RegExp");
	return 0;
	
}

char *get_string_value(struct symbol *object)
{
	struct symbol *temp;
	if (object->type == VAR_OBJECT)
		temp = get_property(object, "value");
	else
		temp = object;
	return temp->value.var.str;
}
/*does not currently support regular expressions or specifying a string
 * as a paremeter or specifying a function as a parameter */
struct ast *string_replace(char *name, struct ast_list *arguments)
{
	struct symbol *object = get_symbol(name);
	char *obj_type = get_obj_type(name);
	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}
	struct ast *arg_one, *arg_two, *retval;
	char *substring;
	char *string = get_string_value(object);
		
	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = STRING_LITERAL;
	retval->next = NULL;
	retval->data.var.string = NULL;
	
	if (is_reg_expr(arguments->top) != 1) {
		arg_one = eval_expr(arguments->top, NULL);
		arg_two = eval_expr(arguments->top->next, NULL);
		substring = strstr(string, arg_one->data.var.string);
		if (substring != NULL) {
			int i, k, j, length;
			int contains;
			int str_size;
			int sub_str_len = strlen(arg_two->data.var.string);
			char *ret_string;
			contains = how_many_substrings(string, arg_one->data.var.string);
			str_size = strlen(string) + (contains *
				strlen(arg_two->data.var.string) - strlen(arg_one->data.var.string));
			ret_string = (char *) jmalloc(sizeof(*ret_string) * str_size); 
			i = 0;
			k = 0;
			length = strlen(arg_one->data.var.string);
			while (substring != NULL || i < length) {
				if ((char *) substring == (char *) &string[i]) {
					j = 0;
					while (j < sub_str_len) {
						ret_string[k] = arg_two->data.var.string[j];
						j++;
						k++;
					}
					i += strlen(arg_one->data.var.string);
					substring = strstr((char *) &string[i], arg_one->data.var.string);
				} else {
					ret_string[k] = string[i];
					i++;
					k++;
				}
			}
				
			retval->data.var.string = ret_string;
			free_expr(arg_one);
			free_expr(arg_two);
		}
	} else {
		retval->data.var.string = reg_string_replace(string, arguments->top, arguments->top->next);
		
	}
	return retval;
}

struct ast *string_splice(char *name, struct ast_list *arguments)
{
	struct symbol *object = get_symbol(name);
	char *obj_type = get_obj_type(name);
	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}
	struct ast *arg_one, *arg_two, *retval;
	char *ret_string;
	int start_index;
	int end_index;
	int i;
	
	char *string = get_string_value(object);
	
	arg_two = NULL;
	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = STRING_LITERAL;
	retval->next = NULL;
	arg_one = eval_expr(arguments->top, NULL);
	if (arguments->top->next == NULL) {
		end_index = strlen(string) - 1;
	} else {
		arg_two = eval_expr(arguments->top->next, NULL);
		end_index = (int) arg_two->data.var.val;
		if (end_index <= 0)
			end_index += strlen(string) - 1;
	}
	
	if (arg_one->data.var.val < 0)
		start_index = (int) arg_one->data.var.val + strlen(string);
	else
		start_index = (int) arg_one->data.var.val;
	
	ret_string = jmalloc(sizeof (*ret_string) * (end_index - start_index));
	i = 0;
	while (start_index <= end_index) {
		ret_string[i] = string[start_index];
		i++;
		start_index++;
	}
	retval->data.var.string = ret_string;
	free_expr(arg_one);
	if (arg_two != NULL)
		free_expr(arg_two);
		
	return retval;
}

struct ast *string_split(char *name, struct ast_list *arguments)
{
	struct symbol *object = get_symbol(name);
	unsigned int i = 0;
	unsigned int index = 0;
	int k;
	char *new_str;
	char *string = get_string_value(object);
	char *obj_type = get_obj_type(name);
	struct ast *arg_one, *arg_two, *retval, *arr_element, *temp;
	struct ast_list *array = jmalloc(sizeof(*array));
	
	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}

	array->top = NULL;

	arg_one = eval_expr(arguments->top, NULL);

	if (arguments->top->next == NULL) {
		
		char *substring = strstr(string, arg_one->data.var.string);
		while (substring != NULL || i < strlen(string)) {
			i = 0;
			while ((char *) substring != (char *) &string[index + i]
				&& (index + i) < strlen(string)){
				i++;
			}
			
			if (i == 0)
				break;
				
			new_str = jmalloc(sizeof(*new_str) * i);
			k = 0;
			i += index;
			while (index < i) {
				new_str[k] = string[index];
				index++;
				k++;
			}
			
			arr_element = jmalloc(sizeof(*arr_element));
			arr_element->type = VAR;
			arr_element->data.var.type = STRING_LITERAL;
			arr_element->next = NULL;
			arr_element->data.var.string = new_str;
			temp = array->top;
			
			if (temp == NULL) {
				array->top = arr_element;
			} else {
				while (temp->next != NULL) {
					temp = temp->next;
				}
				temp->next = arr_element;
			}
			index += strlen(arg_one->data.var.string);
			substring = strstr((char *) &string[index], arg_one->data.var.string);
			
		}
		
	} else {
		char *substring = strstr(string, arg_one->data.var.string);
		arg_two = eval_expr(arguments->top->next, NULL);
		int num_elements = 0;
		while (substring != NULL && arg_two->data.var.val >= num_elements) {
			i = 0;
			while ((char *) substring != (char *) &string[index + i]) {
				i++;
			}
			new_str = jmalloc(sizeof(*new_str) * i);
			k = 0;
			i += index;
			while (index <= i) {
				new_str[k] = string[index];
				index++;
				k++;
			}
			arr_element = jmalloc(sizeof(*arr_element));
			arr_element->type = VAR;
			arr_element->data.var.type = STRING_LITERAL;
			arr_element->next = NULL;
			arr_element->data.var.string = new_str;
			temp = array->top;
			if (temp != NULL) {
				array->top = arr_element;
			} else {
				while (temp->next != NULL) {
					temp = temp->next;
				}
				temp->next = arr_element;
			}
			index += strlen(arg_one->data.var.string);
			substring = strstr((char *) &string[index], arg_one->data.var.string);
			num_elements++;
		}
		free_expr(arg_two);
	}
	retval = create_array(NULL, array);
	free_expr(arg_one);
	
	return retval;
}

struct ast *string_substr(char *name, struct ast_list *arguments)
{
	struct ast *arg_one, *arg_two, *retval;
	struct symbol *object = get_symbol(name);
	int start;
	unsigned int length;
	unsigned int i;
	char *new_str;
	char *string;
	char *obj_type = get_obj_type(name);

	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}
	
	string = get_string_value(object);
	
	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = STRING_LITERAL;
	retval->next = NULL;
	arg_one = eval_expr(arguments->top, NULL);
	start = (int) arg_one->data.var.val;
	
	if (start < 0)
		start = (start + strlen(object->value.var.str));
	if (arguments->top->next == NULL) {
		length = strlen(string);
		new_str = jmalloc(sizeof(*new_str) * (length - start));
	} else {
		arg_two = eval_expr(arguments->top->next, NULL);
		length = (int) arg_two->data.var.val;
		if ((start + length) <= strlen(string))
			new_str = jmalloc(sizeof(*new_str) * length);
		else
			new_str = jmalloc(sizeof(*new_str) * (strlen(string) - start));
		free_expr(arg_two);
	}
	i = 0;
	while (i < length && (i + start) < strlen(string)) {
		new_str[i] = string[start + i];
		i++;
	}
	
	retval->data.var.string = new_str;
	
	free_expr(arg_one);
	
	return retval;
}
	
struct ast *string_substring(char *name, struct ast_list *arguments)
{
	
	
	struct symbol *object = get_symbol(name);
	char *obj_type = get_obj_type(name);
	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}
	struct ast *arg_one, *arg_two, *retval;
	char *new_str;
	int start, end, i, length;
	
	char *string = get_string_value(object);
	
	length = strlen(string);
	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = STRING_LITERAL;
	retval->next = NULL;
	arg_one = eval_expr(arguments->top, NULL);
	start = (int) arg_one->data.var.val;
	if (arguments->top->next == NULL)
		end = strlen(string);
	else {
		arg_two = eval_expr(arguments->top->next, NULL);
		end = (int) arg_two->data.var.val;
		free_expr(arg_two);
	}
	
	if (start < 0)
		start = 0;
	if (end < 0)
		end = 0;
	if (start > length)
		start = length;
	if (end > length)
		end = length;
	
	if (start > end) {
		int temp = start;
		start = end;
		end = temp;
	}
	
	new_str = jmalloc(sizeof(*new_str) * (end - start));
	
	for (i = 0; i + start < end; i++) {
		new_str[i] = string[start + i];
	}
	
	retval->data.var.string = new_str;
	
	free_expr(arg_one);
	
	return retval;
}

struct ast *string_match(char *name, struct ast_list *arguments)
{
	struct symbol *object = get_symbol(name);
	char *obj_type = get_obj_type(name);
	struct ast *retval = NULL;
	char *str = get_string_value(object);

	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}

	//check for inherent reg expr eventually and wrap it
	if (arguments->top->type == VAR) {
		retval = reg_string_match(arguments->top->data.var.name, str);
	} else if (arguments->top->type == REG_EXP) {
		char *name = strdup("__temp__");
		struct ast *reg = arguments->top;
		add_new_reg(name, reg->data.var.string);
		retval = reg_string_match(name, str);
		remove_sym(name);
		free(name);
	} else
		printf("this type is not supported\n");
	
	return retval;
}

struct ast *string_search(char *name, struct ast_list *arguments)
{
	struct symbol *object = get_symbol(name);
	char *obj_type = get_obj_type(name);
	if(strcmp(obj_type, "String") != 0) {
		return NULL;
	}

	struct ast *retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = BOOL;
	retval->next = NULL;
	char *str = get_string_value(object);
	//gonna have to wrap the regular expression
	if (arguments->top->type == VAR) {
		retval->data.var.val = reg_string_search(arguments->top->data.var.name, str);
	} else if (arguments->top->type == REG_EXP) {
		char *name = strdup("__temp__");
		struct ast *reg = arguments->top;
		add_new_reg(name, reg->data.var.string);
		retval->data.var.val = reg_string_search(name, str);
		remove_sym(name);
		free(name);
	} else
		printf("this type does not work right now\n");


	return retval;
}
