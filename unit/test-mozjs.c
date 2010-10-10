/*
 *
 *  PACrunner - Proxy configuration daemon
 *
 *  Copyright (C) 2010  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include <glib.h>

#include "pacrunner.h"

#define MULTIPLE_COUNT 10

static void test_single_init(void)
{
	__pacrunner_mozjs_init();
	__pacrunner_mozjs_cleanup();
}

static void test_multiple_init(void)
{
	int i;

	for (i = 0; i < MULTIPLE_COUNT; i++) {
		__pacrunner_mozjs_init(); 
		__pacrunner_mozjs_cleanup();
	}
}

static void test_single_execute_without_pac(void)
{
	const char *result;

	__pacrunner_mozjs_init();

	result = __pacrunner_mozjs_execute("http://www.example.com/",
							"www.example.com");

	__pacrunner_mozjs_cleanup();
}

static void test_multiple_execute_without_pac(void)
{
	const char *result;
	int i;

	__pacrunner_mozjs_init();

	for (i = 0; i < MULTIPLE_COUNT; i++)
		result = __pacrunner_mozjs_execute("http://www.example.com/",
							"www.example.com");

	__pacrunner_mozjs_cleanup();
}

int main(int argc, char **argv)
{
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/mozjs/single-init", test_single_init);
	g_test_add_func("/mozjs/multiple-init", test_multiple_init);
	g_test_add_func("/mozjs/single-execute-without-pac",
					test_single_execute_without_pac);
	g_test_add_func("/mozjs/multiple-execute-without-pac",
					test_multiple_execute_without_pac);

	return g_test_run();
}
