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

#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H

struct ast *math_addtwo(struct ast_list *parameters, char *name);
struct ast *math_cos(struct ast_list *parameters, char *name);
struct ast *math_abs(struct ast_list *parameters, char *name);
struct ast *math_acos(struct ast_list *parameters, char *name);
struct ast *math_asin(struct ast_list *parameters, char *name);
struct ast *math_atan(struct ast_list *parameters, char *name);
struct ast *math_atan2(struct ast_list *parameters, char *name);
struct ast *math_ceil(struct ast_list *parameters, char *name);
struct ast *math_floor(struct ast_list *parameters, char *name);
struct ast *math_log(struct ast_list *parameters, char *name);
struct ast *math_pow(struct ast_list *parameters, char *name);
struct ast *math_round(struct ast_list *parameters, char *name);
struct ast *math_sqrt(struct ast_list *parameters, char *name);
struct ast *math_tan(struct ast_list *parameters, char *name);
struct ast *math_exp(struct ast_list *parameters, char *name);
struct ast *math_max(struct ast_list *parameters, char *name);
struct ast *math_min(struct ast_list *parameters, char *name);
struct ast *parse_int(struct ast_list *parameters, char *name);
struct ast *to_number(struct ast_list *parameters, char *name);
struct ast *is_nan(struct ast_list *parameters, char *name);

#endif
