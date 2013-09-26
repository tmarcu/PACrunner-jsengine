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
#include <ctype.h>
#include "symbols.h"
#include "tree.h"
#include "spider.h"
#include "parser.h"

/* List for parsing parseInt() radices */
static char *excludes[52] = {"", "",
		"23456789", /* radix 2*/
		"3456789",  /* radix 3...so on */
		"456789",
		"56789",
		"6789",
		"789",
		"89",
		"9",
		"",
		"bBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ",
		"cCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ",
		"dDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ",
		"eEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ",
		"fFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ",
		"gGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ",
		"hHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ",
		"iIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ",
		"jJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ",
		"kKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ",
		"lLmMnNoOpPqQrRsStTuUvVwWxXyYzZ",
		"mMnNoOpPqQrRsStTuUvVwWxXyYzZ",
		"nNoOpPqQrRsStTuUvVwWxXyYzZ",
		"oOpPqQrRsStTuUvVwWxXyYzZ",
		"pPqQrRsStTuUvVwWxXyYzZ",
		"qQrRsStTuUvVwWxXyYzZ",
		"rRsStTuUvVwWxXyYzZ",
		"sStTuUvVwWxXyYzZ",
		"tTuUvVwWxXyYzZ",
		"uUvVwWxXyYzZ",
		"vVwWxXyYzZ",
		"wWxXyYzZ",
		"xXyYzZ",
		"yYzZ",
		"zZ",
};

struct ast *math_addtwo(struct ast_list *parameters, char *name)
{
	struct ast *left;
	struct ast *right;
	struct ast *retval = jmalloc(sizeof(*retval));

	left = eval_expr(parameters->top, NULL);
	right = eval_expr(parameters->top->next, NULL);

	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = left->data.var.val + right->data.var.val;

	free_expr(left);
	free_expr(right);

	return retval;
}

struct ast *math_cos(struct ast_list *parameters, char *name)
{
	struct ast *arg;
	struct ast *retval;

	arg =  eval_expr(parameters->top, NULL);

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = cos(arg->data.var.val);

	free_expr(arg);

	return retval;
}

struct ast *math_abs(struct ast_list *parameters, char *name)
{
	struct ast *arg;
	struct ast *retval;

	arg =  eval_expr(parameters->top, NULL);

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;

	if (arg->data.var.val < 0)
		retval->data.var.val = -arg->data.var.val;
	else
		retval->data.var.val = arg->data.var.val;

	free_expr(arg);

	return retval;
}

struct ast *math_acos(struct ast_list *parameters, char *name)
{
	struct ast *arg;
	struct ast *retval;

	arg =  eval_expr(parameters->top, NULL);

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = acos(arg->data.var.val);

	free_expr(arg);

	return retval;
}

struct ast *math_asin(struct ast_list *parameters, char *name)
{
	struct ast *arg;
	struct ast *retval;

	arg =  eval_expr(parameters->top, NULL);

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = asin(arg->data.var.val);

	free_expr(arg);

	return retval;
}

struct ast *math_atan(struct ast_list *parameters, char *name)
{
	struct ast *arg;
	struct ast *retval;

	arg =  eval_expr(parameters->top, NULL);

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = atan(arg->data.var.val);

	free_expr(arg);

	return retval;
}

struct ast *math_atan2(struct ast_list *parameters, char *name)
{
	struct ast *arg;
	struct ast *argtwo;
	struct ast *retval;

	arg =  eval_expr(parameters->top, NULL);
	argtwo = eval_expr(parameters->top->next, NULL);

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = atan2(arg->data.var.val, argtwo->data.var.val);

	free_expr(arg);
	free_expr(argtwo);

	return retval;
}

struct ast *math_ceil(struct ast_list *parameters, char *name)
{
	struct ast *arg;
	struct ast *retval;

	arg =  eval_expr(parameters->top, NULL);

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = ceil(arg->data.var.val);

	free_expr(arg);

	return retval;
}

struct ast *math_floor(struct ast_list *parameters, char *name)
{
	struct ast *arg;
	struct ast *retval;

	arg =  eval_expr(parameters->top, NULL);

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = floor(arg->data.var.val);

	free_expr(arg);

	return retval;
}

struct ast *math_log(struct ast_list *parameters, char *name)
{
	struct ast *arg;
	struct ast *retval;

	arg =  eval_expr(parameters->top, NULL);

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = log(arg->data.var.val);

	free_expr(arg);

	return retval;
}

struct ast *math_pow(struct ast_list *parameters, char *name)
{
	struct ast *arg;
	struct ast *power;
	struct ast *retval;

	arg =  eval_expr(parameters->top, NULL);
	power = eval_expr(parameters->top->next, NULL);

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = pow(arg->data.var.val, power->data.var.val);

	free_expr(arg);
	free_expr(power);

	return retval;
}

struct ast *math_round(struct ast_list *parameters, char *name)
{
	struct ast *arg;
	struct ast *retval;

	arg =  eval_expr(parameters->top, NULL);

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = round(arg->data.var.val);

	free_expr(arg);

	return retval;
}

struct ast *math_sqrt(struct ast_list *parameters, char *name)
{
	struct ast *arg;
	struct ast *retval;

	arg =  eval_expr(parameters->top, NULL);

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = sqrt(arg->data.var.val);

	free_expr(arg);

	return retval;
}

struct ast *math_tan(struct ast_list *parameters, char *name)
{
	struct ast *arg;
	struct ast *retval;

	arg =  eval_expr(parameters->top, NULL);

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = tan(arg->data.var.val);

	free_expr(arg);

	return retval;
}

struct ast *math_exp(struct ast_list *parameters, char *name)
{
	struct ast *arg;
	struct ast *retval;

	arg =  eval_expr(parameters->top, NULL);

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = exp(arg->data.var.val);

	free_expr(arg);

	return retval;
}

struct ast *math_max(struct ast_list *parameters, char *name)
{
	struct ast *largest;
	struct ast *temp;
	struct ast *trash;
	struct ast *list;

	largest =  eval_expr(parameters->top, NULL);
	list = parameters->top;

	while (list->next != NULL) {
		temp = eval_expr(list->next, NULL);
		if (temp->data.var.val > largest->data.var.val) {
			trash = largest;
			largest = temp;
			temp = trash;
		}
		free_expr(temp);
		list = list->next;
	}

	return largest;
}

struct ast *math_min(struct ast_list *parameters, char *name)
{
	struct ast *smallest;
	struct ast *temp;
	struct ast *trash;
	struct ast *list;

	smallest =  eval_expr(parameters->top, NULL);
	list = parameters->top;

	while (list->next != NULL) {
		temp = eval_expr(list->next, NULL);
		if (temp->data.var.val < smallest->data.var.val) {
			trash = smallest;
			smallest = temp;
			temp = trash;
		}
		free_expr(temp);
		list = list->next;
	}

	return smallest;
}

struct ast *to_number(struct ast_list *parameters, char *name)
{
	struct ast *args;
	struct ast *retval;

	args = eval_expr(parameters->top, NULL);
	if (args == NULL)
		return NULL;

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR_NUM;
	retval->data.var.type = NUM;

	switch (args->data.var.type) {
	case NUM:
		retval->data.var.val = args->data.var.val;
	break;
	case BOOL:
		retval->data.var.val = (double) args->data.var.val;
	break;
	case STRING_LITERAL:
		retval->data.var.val = (double) strtol(args->data.var.string,
							NULL, 10);
		if (retval->data.var.val != 0)
			break;
	case NaN:
	default:
		retval->type = VAR;
		retval->data.var.type = NaN;
		retval->data.var.string = strdup("NaN");
	break;
	}

	retval->next = NULL;
	free_expr(args);

	return retval;
}

struct ast *is_nan(struct ast_list *parameters, char *name)
{
	struct ast *args;
	struct ast *retval;

	args = parameters->top;
	if (args == NULL)
		return NULL;

	retval = to_number(parameters, name);

	if (retval->data.var.type != NaN &&
	    retval->data.var.type != STRING_LITERAL) {
		retval->type = BOOL;
		retval->data.var.type = STRING_LITERAL;
		retval->data.var.string = strdup("false");
	} else {
		retval->type = BOOL;
		retval->data.var.type = STRING_LITERAL;
		retval->data.var.string = strdup("true");
	}

	return retval;
}

struct ast *parse_int(struct ast_list *parameters, char *name)
{
	struct ast *args;
	struct ast *retval;
	struct ast *nstr;
	struct ast *other_nstr;
	char *tok;
	char *ntok;
	int sign = 1;
	int radix = 10;

	args = parameters->top;
	if (args == NULL)
		return NULL;

	retval = jmalloc(sizeof(*retval));
	retval->type = VAR;

	nstr = eval_expr(args, NULL);
	ntok = NULL;

	tok = strtok(nstr->data.var.string, " ,");
	if (tok[0] == '-' && isalpha(tok[1]) == 0) {
		sign -= 2;
		if (isdigit(tok[1]))
			tok++;
		else
			tok = strtok(NULL, " ");
	} else if (tok[0] == '+' && isalpha(tok[1]) == 0) {
		if (isdigit(tok[1]))
			tok++;
		else
			tok = strtok(NULL, " ");
	}

	if (args->next != NULL) {
		other_nstr = eval_expr(args->next, NULL);
		if (other_nstr->data.var.val < 2 ||
						  other_nstr->data.var.val > 36)
			tok[0] = ' ';
		else
			radix = other_nstr->data.var.val;
		free_expr(other_nstr);
	}

	/* Handle hexadecimal, without using a stripPrefix flag */
	if ((strncmp(tok, "0x", 2) == 0 ||
	     strncmp(tok, "0X", 2) == 0)) {
		if (radix != 16) {
			retval->data.var.type = NaN;
			retval->data.var.string = strdup("NaN");
			goto RET;
		}
		retval->data.var.type = NUM;

		tok += 2;
		tok = strtok(tok, excludes[radix]);
		ntok = strdup(tok);
		snprintf(ntok, strlen(tok) + 3, "0x%s", tok);
		retval->data.var.val = (double) strtol(ntok, NULL, 16) * sign;

		goto RET;
	}

	if (isdigit(tok[0]) == 0) {
		retval->data.var.type = NaN;
		retval->data.var.string = strdup("NaN");
	} else {
		retval->data.var.type = NUM;
		tok = strtok(tok, excludes[radix]);
		retval->data.var.val = (double) strtol(tok, NULL, radix) * sign;
	}

RET:
	free_expr(nstr);
	if (ntok != NULL)
		free(ntok);
	return retval;
}
