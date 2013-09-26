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

#ifndef ARRAY_METHODS_H
#define ARRAY_METHODS_H

struct ast *array_reverse(char *name, struct ast_list arguments);
struct ast *array_concat(char *name, struct ast_list *arguments);
struct ast *array_join(char *name, struct ast_list *arguments);
struct ast *array_last_index_of(char *name, struct ast_list *arguments);
struct ast *array_index_of(char *name, struct ast_list *arguments);

void alert_print_array(struct ast *arg);

#endif
