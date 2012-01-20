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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

enum test_suite_part {
	SUITE_TITLE    = 0,
	SUITE_PAC      = 1,
	SUITE_SERVERS  = 2,
	SUITE_EXCLUDES = 3,
	SUITE_CONFIG   = 4,
	SUITE_TESTS    = 5,
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
	struct pacrunner_test_suite *suite;
	enum test_suite_part part;
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

static void run_test_suite(const char *test_file_path)
{
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

	free_pacrunner_test_suite(test_suite);
	test_suite = NULL;
}

static void find_and_run_test_suite(GDir *test_dir,
					const char *test_path,
					const char *file_path)
{
	const gchar *test_file;
	gchar *test_file_path;

	if (test_dir == NULL && test_path == NULL)
		return;

	if (test_dir != NULL) {
		for (test_file = g_dir_read_name(test_dir); test_file != NULL;
				test_file = g_dir_read_name(test_dir)) {
			test_file_path = g_strdup_printf("%s/%s",
						test_path, test_file);
			if (test_file_path == NULL)
				return;

			run_test_suite(test_file_path);

			g_free(test_file_path);
		}
	} else {
		if (test_path != NULL)
			test_file_path = g_strdup_printf("%s/%s",
						test_path, file_path);
		else
			test_file_path = g_strdup(file_path);

		run_test_suite(test_file_path);

		g_free(test_file_path);
	}
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
		"\t--verbose, -v: verbose output mode\n");
}

static struct option test_options[] = {
	{"dir",     1, 0, 'd'},
	{"file",    1, 0, 'f'},
	{"help",    0, 0, 'h'},
	{"verbose", 0, 0, 'v'},
	{ NULL }
};

int main(int argc, char *argv[])
{
	gchar *test_path = NULL;
	gchar *file_path = NULL;
	GDir *test_dir = NULL;
	int opt_index = 0;
	int c;

	if (argc < 2)
		goto error;

	while ((c = getopt_long(argc, argv, "d:hv",
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

	find_and_run_test_suite(test_dir, test_path, file_path);

	g_free(test_path);

	if (test_dir != NULL)
		g_dir_close(test_dir);

	return EXIT_SUCCESS;

error:
	print_usage();
	return EXIT_FAILURE;
}
