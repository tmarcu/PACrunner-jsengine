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
#include "tree.h"
#include "symbols.h"
#include "parser.h"
#include "jsparse.h"

static void test_files(int numargs, char **files, struct ast **ret)
{
	int i;

	printf("\n**TEST RESULTS\n");
	for (i = 0; i < numargs; i++) {
		if (ret[i]->data.return_val->data.var.type == NUM &&
				ret[i]->data.return_val->data.var.val != 0) {
			fprintf(stderr, "!FAILED:Test[%d] - %s\n\t%s\n",
				i, files[i+1],
				ret[i]->data.return_val->data.var.string);
		} else if (ret[i]->data.return_val->data.var.type ==
							STRING_LITERAL &&
			   strncmp(ret[i]->data.return_val->data.var.string,
				   "ERROR", 5) == 0) {
			fprintf(stderr, "!FAILED:Test[%d] - %s\n\t%s\n",
				i, files[i+1],
				ret[i]->data.return_val->data.var.string);
		} else {
			printf("*PASSED:Test[%d] - %s\n", i, files[i+1]);
		}

		free(ret[i]);
	}
}

int main(int argc, char *argv[])
{
	struct ast **returnblock;
	struct ast_list **list;
	char test = 0;
	int i;

	if (argc >= 3 && strcmp(argv[1], "--test") == 0) {
		printf("*Running test mode...\n");
		test = 1;
		argv++;
		argc--;
	}

	sym_table_init();

	list = parse_files(argv, argc);

	/* one_test = execute_block(argv[1], 0, list[0]);*/

	returnblock = execute_all(argv, argc, list);


	if (test == 1)
		test_files(argc - 1, argv, returnblock);

	for (i = 0; list[i] != NULL; i++)
		free(list[i]);
	free(list);

	free(returnblock);
	free_sym_table(symtable);
	free_block(program_block);

	return 0;
}
