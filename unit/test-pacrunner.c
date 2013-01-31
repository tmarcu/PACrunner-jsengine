/*
 *
 *  PACrunner - Proxy configuration daemon
 *
 *  Copyright (C) 2010-2012  Intel Corporation. All rights reserved.
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

#include <glib.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>
#include <CUnit/Console.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "pacrunner.h"

enum test_suite_part {
	SUITE_TITLE    = 0,
	SUITE_PAC      = 1,
	SUITE_SERVERS  = 2,
	SUITE_EXCLUDES = 3,
	SUITE_CONFIG   = 4,
	SUITE_TESTS    = 5,
	SUITE_NOTHING  = 6,
};

enum cu_test_mode {
	CU_MODE_BASIC   = 0,
	CU_MODE_AUTO    = 1,
	CU_MODE_CONSOLE = 2,
};

struct pacrunner_test_suite {
	gchar *title;
	gchar *pac;
	gchar **servers;
	gchar **excludes;

	gboolean config_result;

	gchar **tests;
};

static struct pacrunner_test_suite *test_suite;
static gboolean verbose = FALSE;

static struct pacrunner_proxy *proxy;
static gboolean test_config;

static void free_pacrunner_test_suite(struct pacrunner_test_suite *suite)
{
	if (suite == NULL)
		return;

	g_free(suite->title);
	g_free(suite->pac);
	g_strfreev(suite->servers);
	g_strfreev(suite->excludes);
	g_strfreev(suite->tests);

	g_free(suite);
}

static gchar **_g_strappendv(gchar **str_array, const gchar *str)
{
	int length = 0;
	gchar **result;
	gchar *copy;

	if (str == NULL)
		return NULL;

	if (str_array != NULL)
		length = g_strv_length(str_array);

	result = g_try_malloc0(sizeof(gchar *) * (length + 2));
	if (result == NULL)
		return NULL;

	copy = g_strdup(str);
	if (copy == NULL) {
		g_free(result);
		return NULL;
	}

	if (str_array != NULL) {
		g_memmove(result, str_array, length * sizeof(gchar *));
		memset(str_array, 0, length * sizeof(gchar *));
	}

	result[length] = copy;

	return result;
}

static void print_test_suite(struct pacrunner_test_suite *suite)
{
	gchar **line;

	if (suite == NULL)
		return;

	printf("\nSuite: %s\n", suite->title);

	printf("\nPAC:\n%s\n", suite->pac);

	printf("\nServers:\n");
	if (suite->servers != NULL) {
		for (line = suite->servers; *line != NULL; line++)
			printf("%s\n", *line);
	} else
		printf("(none)\n");


	printf("\nExcludes:\n");
	if (suite->excludes != NULL) {
		for (line = suite->excludes; *line != NULL; line++)
			printf("%s\n", *line);
	} else
		printf("(none)\n");

	printf("\nConfig result: %s\n",
			suite->config_result == TRUE ? "Valid" : "Invalid");

	printf("\nTests:\n");
	if (suite->tests != NULL) {
		short test = 0;

		for (line = suite->tests; *line != NULL; line++) {
			if (test == 0) {
				printf("%s --> ", *line);
				test++;
			} else {
				printf("%s\n", *line);
				test = 0;
			}
		}

		printf("\n");
	} else
		printf("(none)\n");
}

static struct pacrunner_test_suite *read_test_suite(const char *path)
{
	enum test_suite_part part = SUITE_NOTHING;
	struct pacrunner_test_suite *suite;
	gchar *content = NULL;
	gchar **lines = NULL;
	gchar **array;
	gchar **line;

	suite = g_try_malloc0(sizeof(struct pacrunner_test_suite));
	if (suite == NULL)
		goto error;

	if (g_file_get_contents(path, &content, NULL, NULL) == FALSE)
		goto error;

	if (strlen(content) <= 0)
		goto error;

	lines = g_strsplit(content, "\n", 0);
	if (lines == NULL)
		goto error;

	for (line = lines; *line != NULL; line++) {
		if (strlen(*line) == 0)
			continue;

		if (*line[0] == '#')
			continue;

		if (*line[0] != '[') {
			switch (part) {
			case SUITE_TITLE:
				if (suite->title != NULL)
					goto error;

				suite->title = g_strdup(*line);

				if (suite->title == NULL)
					goto error;

				break;
			case SUITE_PAC:
				if (suite->pac == NULL)
					suite->pac = g_strdup_printf("%s\n",
									*line);
				else {
					gchar *oldpac = suite->pac;

					suite->pac = g_strdup_printf("%s%s\n",
								oldpac, *line);
					g_free(oldpac);
				}

				if (suite->pac == NULL)
					goto error;

				break;
			case SUITE_SERVERS:
				array = _g_strappendv(suite->servers, *line);
				if (array == NULL)
					goto error;

				g_free(suite->servers);
				suite->servers = array;

				break;
			case SUITE_EXCLUDES:
				array = _g_strappendv(suite->excludes, *line);
				if (array == NULL)
					goto error;

				g_free(suite->excludes);
				suite->excludes = array;

				break;
			case SUITE_CONFIG:
				if (strncmp(*line, "VALID", 5) == 0)
					suite->config_result = TRUE;
				else
					suite->config_result = FALSE;

				break;
			case SUITE_TESTS:
				array = _g_strappendv(suite->tests, *line);
				if (array == NULL)
					goto error;

				g_free(suite->tests);
				suite->tests = array;

				break;
			case SUITE_NOTHING:
			default:
				break;
			}

			continue;
		}

		if (strncmp(*line, "[title]", 7) == 0)
			part = SUITE_TITLE;
		else if (strncmp(*line, "[pac]", 5) == 0)
			part = SUITE_PAC;
		else if (strncmp(*line, "[servers]", 9) == 0)
			part = SUITE_SERVERS;
		else if (strncmp(*line, "[excludes]", 10) == 0)
			part = SUITE_EXCLUDES;
		else if (strncmp(*line, "[config]", 8) == 0)
			part = SUITE_CONFIG;
		else if (strncmp(*line, "[tests]", 7) == 0)
			part = SUITE_TESTS;
	}

	if (verbose == TRUE)
		print_test_suite(suite);

	if (suite->title == NULL || (suite->tests != NULL
			&& g_strv_length(suite->tests) % 2 != 0)
			|| (suite->servers == NULL && suite->pac == NULL))
		goto error;

	g_free(content);
	g_strfreev(lines);

	return suite;

error:
	g_free(content);
	g_strfreev(lines);

	free_pacrunner_test_suite(suite);

	return NULL;
}

static int test_suite_init(void)
{
	proxy = pacrunner_proxy_create("eth0");
	if (proxy == NULL)
		return -1;

	return 0;
}

static int test_suite_cleanup(void)
{
	if (test_config == TRUE) {
		if (pacrunner_proxy_disable(proxy) != 0)
			return -1;
	}

	pacrunner_proxy_unref(proxy);

	return 0;
}

static void test_pac_config(void)
{
	if (pacrunner_proxy_set_auto(proxy, NULL, test_suite->pac) == 0)
		test_config = TRUE;

	CU_ASSERT_TRUE(test_suite->config_result == test_config);
}

static void test_manual_config(void)
{
	if (pacrunner_proxy_set_manual(proxy, test_suite->servers,
						test_suite->excludes) == 0)
		test_config = TRUE;

	CU_ASSERT_TRUE(test_suite->config_result == test_config);
}

static void test_proxy_requests(void)
{
	gchar **test_strings;
	gboolean verify;
	gchar *result;
	gchar **test;
	gchar *msg;

	if (test_config == FALSE)
		return;

	if (verbose == TRUE)
		printf("\n");

	for (test = test_suite->tests; *test != NULL; test = test + 2) {
		gchar *test_result = *(test+1);

		test_strings = g_strsplit(*test, " ", 2);
		if (test_strings == NULL || g_strv_length(test_strings) != 2) {
			g_strfreev(test_strings);
			continue;
		}

		result = pacrunner_proxy_lookup(test_strings[0],
						test_strings[1]);
		g_strfreev(test_strings);

		verify = FALSE;

		if (strncmp(test_result, "DIRECT", 6) == 0) {
			if (result == NULL ||
					strncmp(result, "DIRECT", 6) == 0)
				verify = TRUE;
		} else {
			if (g_strcmp0(result, test_result) == 0)
				verify = TRUE;
		}

		if (verbose == TRUE) {
			if (verify == TRUE)
				msg = g_strdup_printf(
						"\tTEST: %s -> %s verified",
							*test, test_result);
			else
				msg = g_strdup_printf(
					"\tTEST: %s -> %s FAILED (%s)",
					*test, test_result,
					result == NULL ? "DIRECT" : result);

			printf("%s\n", msg);
			g_free(msg);
		}

		if (result != NULL)
			g_free(result);

		CU_ASSERT_TRUE(verify);
	}
}

static void run_test_suite(const char *test_file_path, enum cu_test_mode mode)
{
	CU_pTestRegistry cu_registry;
	CU_pSuite cu_suite;

	if (test_file_path == NULL)
		return;

	test_suite = read_test_suite(test_file_path);
	if (test_suite == NULL) {
		if (verbose == TRUE)
			printf("Invalid suite\n");
		return;
	}

	if (verbose == TRUE)
		printf("Valid suite\n");

	cu_registry = CU_create_new_registry();

	cu_registry = CU_set_registry(cu_registry);
	CU_destroy_existing_registry(&cu_registry);

	cu_suite = CU_add_suite(test_suite->title, test_suite_init,
						test_suite_cleanup);

	if (test_suite->pac != NULL)
		CU_add_test(cu_suite, "PAC config test", test_pac_config);
	else
		CU_add_test(cu_suite, "Manual config test",
						test_manual_config);

	if (test_suite->config_result == TRUE && test_suite->tests != NULL)
		CU_add_test(cu_suite, "Proxy requests test",
						test_proxy_requests);

	test_config = FALSE;

	switch (mode) {
	case CU_MODE_BASIC:
		CU_basic_set_mode(CU_BRM_VERBOSE);

		CU_basic_run_tests();

		break;
	case CU_MODE_AUTO:
		CU_set_output_filename(test_file_path);
		CU_list_tests_to_file();

		CU_automated_run_tests();

		break;
	case CU_MODE_CONSOLE:
		CU_console_run_tests();

		break;
	default:
		break;
	}

	free_pacrunner_test_suite(test_suite);
	test_suite = NULL;
}

static void find_and_run_test_suite(GDir *test_dir,
					const char *test_path,
					const char *file_path,
					enum cu_test_mode mode)
{
	const gchar *test_file;
	gchar *test_file_path;

	if (test_dir == NULL && test_path == NULL)
		return;

	if (CU_initialize_registry() != CUE_SUCCESS) {
		printf("%s\n", CU_get_error_msg());
		return;
	}

	if (test_dir != NULL) {
		for (test_file = g_dir_read_name(test_dir); test_file != NULL;
				test_file = g_dir_read_name(test_dir)) {
			test_file_path = g_strdup_printf("%s/%s",
						test_path, test_file);
			if (test_file_path == NULL)
				return;

			run_test_suite(test_file_path, mode);

			g_free(test_file_path);
		}
	} else {
		if (test_path != NULL)
			test_file_path = g_strdup_printf("%s/%s",
						test_path, file_path);
		else
			test_file_path = g_strdup(file_path);

		run_test_suite(test_file_path, mode);

		g_free(test_file_path);
	}

	CU_cleanup_registry();
}

static GDir *open_test_dir(gchar **test_path)
{
	GDir *test_dir;

	if (test_path == NULL || *test_path == NULL)
		return NULL;

	test_dir = g_dir_open(*test_path, 0, NULL);
	if (test_dir == NULL && g_path_is_absolute(*test_path) == FALSE) {
		gchar *current, *path;

		current = g_get_current_dir();

		path = g_strdup_printf("%s/%s", current, *test_path);
		g_free(*test_path);
		g_free(current);

		*test_path = path;

		test_dir = g_dir_open(*test_path, 0, NULL);
		if (test_dir == NULL)
			return NULL;
	}

	return test_dir;
}

static void print_usage()
{
	printf("Usage: test-pacrunner <options>\n"
		"\t--dir, -d directory: specify a directory where test"
		" suites are found\n"
		"\t--file, -f file: specify a test suite file\n"
		"\t--help, -h: print this help\n"
		"\t--mode, -m CUnit mode: basic (default), auto, console\n"
		"\t--verbose, -v: verbose output mode\n");
}

static struct option test_options[] = {
	{"dir",     1, 0, 'd'},
	{"file",    1, 0, 'f'},
	{"help",    0, 0, 'h'},
	{"mode",    1, 0, 'm'},
	{"verbose", 0, 0, 'v'},
	{ NULL }
};

int main(int argc, char *argv[])
{
	enum cu_test_mode mode = CU_MODE_BASIC;
	gchar *test_path = NULL;
	gchar *file_path = NULL;
	GDir *test_dir = NULL;
	int opt_index = 0;
	int c;

	if (argc < 2)
		goto error;

	while ((c = getopt_long(argc, argv, "d:hm:v",
			test_options, &opt_index)) != -1) {
		switch (c) {
		case 'd':
			if (file_path != NULL)
				goto error;

			test_path = g_strdup(optarg);

			break;
		case 'f':
			if (test_path != NULL)
				goto error;

			file_path = g_strdup(optarg);

			break;
		case 'h':
			print_usage();
			return EXIT_SUCCESS;

			break;
		case 'm':
			if (strncmp(optarg, "basic", 5) == 0)
				mode = CU_MODE_BASIC;
			else if (strncmp(optarg, "auto", 4) == 0)
				mode = CU_MODE_AUTO;
			else if (strncmp(optarg, "console", 6) == 0)
				mode = CU_MODE_CONSOLE;

			break;
		case 'v':
			verbose = TRUE;

			break;
		case '?':
		default:
			goto error;

		}
	}

	if (test_path == NULL && file_path == NULL)
		goto error;

	if (test_path != NULL) {
		test_dir = open_test_dir(&test_path);
		if (test_dir == NULL) {
			g_free(test_path);
			return EXIT_FAILURE;
		}
	}

	if (file_path != NULL)
		test_path = g_get_current_dir();

	__pacrunner_proxy_init();
	__pacrunner_js_init();
	__pacrunner_manual_init();
	__pacrunner_plugin_init(NULL, NULL);

	find_and_run_test_suite(test_dir, test_path, file_path, mode);

	__pacrunner_plugin_cleanup();
	__pacrunner_manual_cleanup();
	__pacrunner_js_cleanup();
	__pacrunner_proxy_cleanup();

	g_free(test_path);

	if (test_dir != NULL)
		g_dir_close(test_dir);

	return EXIT_SUCCESS;

error:
	print_usage();
	return EXIT_FAILURE;
}
