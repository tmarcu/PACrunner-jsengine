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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <stdio.h>
#include <stdlib.h>

#include "jsengine/tree.h"
#include "jsengine/symbols.h"
#include "jsengine/parser.h"
#include "jsengine/spider.h"
#include "jsengine/javascript_functions.h"

#include "javascript.h"

#include "pacrunner.h"
#include "js.h"

struct ast_list **parse_files(char **file, int sz);
struct ast *execute_block(char *file, int filenum, struct ast_list *pblock);

static struct pacrunner_proxy *current_proxy = NULL;
static struct ast *jsobject = NULL;

void ast_string_arg(struct ast *arg, const char *str)
{
	arg->type = VAR;
	arg->data.var.type = STRING_LITERAL;
	arg->data.var.string = strdup(str);
}

static void create_object(void)
{
	struct ast_list **list = NULL;
	char *script = NULL;
	char **files = jmalloc(sizeof(*files) * 2);

	script = (char *) pacrunner_proxy_get_script(current_proxy);
	if (script == NULL)
		return;

	/* For compatibility with current jsengine setup, generally the engine
	 * is called with cmd line args, like "jsparser /path/to/script", hence
	 * the **files to represent argv */
	files[0] = strdup("JSENGINE");
	files[1] = strdup(script);

	sym_table_init();

	list = parse_files(files, 2);

	jsobject = execute_block(script, 0, list[0]);

	free(files[0]);
}

static void destroy_object()
{
	free(program_block);
}

static int jsengine_set_proxy(struct pacrunner_proxy *proxy)
{
	DBG("proxy %p", proxy);

	if (current_proxy)
		destroy_object();
 
	current_proxy = proxy;
 
	if (current_proxy)
		create_object();

	return 0;
}

static char *jsengine_execute(const char *url, const char *host)
{
	char *result;
	struct ast *url_ast = jmalloc(sizeof(*url_ast));
	struct ast *host_ast = jmalloc(sizeof(*host_ast));
	struct ast *result_ast = jmalloc(sizeof(*result_ast));
	struct ast_list *args = jmalloc(sizeof(*args));

	ast_string_arg(url_ast, url);

	ast_string_arg(host_ast, host);

	args->top = url_ast;

	url_ast->next = host_ast;

	result_ast = eval_func(get_declared_func("FindProxyForURL"), args);

	result = strdup(result_ast->data.return_val->data.var.string);
	printf("the return string is: %s\n", result);

	return result;
}


static struct pacrunner_js_driver jsengine_driver = {
	.name		= "jsengine",
	.priority	= PACRUNNER_JS_PRIORITY_DEFAULT,
	.set_proxy	= jsengine_set_proxy,
	.execute	= jsengine_execute,
};

static int jsengine_init(void)
{
	DBG("");

	return pacrunner_js_driver_register(&jsengine_driver);
}


static void jsengine_exit(void)
{
	DBG("");

	pacrunner_js_driver_unregister(&jsengine_driver);

	jsengine_set_proxy(NULL);
}

PACRUNNER_PLUGIN_DEFINE(jsengine, jsengine_init, jsengine_exit)