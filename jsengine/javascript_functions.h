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

#ifndef JAVASCRIPT_H
#define JAVASCRIPT_H

struct ast *dns_domain_is(struct ast_list *args, char *object_name);
struct ast *dns_domain_levels(struct ast_list *args, char *object_name);
struct ast *is_in_net(struct ast_list *args, char *object_name);
struct ast *is_plain_hostname(struct ast_list *args,  char *object_name);
struct ast *local_host_or_domain_is(struct ast_list *args,  char *object_name);
struct ast *sh_exp_match(struct ast_list *args,  char *object_name);
struct ast *my_ip_address(struct ast_list *args,  char *object_name);
struct ast *dns_resolve(struct ast_list *args,  char *object_name);
struct ast *is_resolvable(struct ast_list *args,  char *object_name);
struct ast *weekday_range(struct ast_list *args,  char *object_name);
struct ast *time_range(struct ast_list *args,  char *object_name);
struct ast *date_range(struct ast_list *args,  char *object_name);
char *resolve(char *str);

#endif
