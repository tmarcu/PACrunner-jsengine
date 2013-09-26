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


%{
	#include <ctype.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include "tree.h"
	#include "spider.h"
	#include "symbols.h"
	#define YYPARSE_PARAM scanner
	#define YYLEX_PARAM scanner
	#define YYERROR_VERBOSE 1
	void yyerror (char const *);
%}

%define api.pure full
%error-verbose

%union {
	double num;
	char *var_name;
	struct ast *node;
	struct ast_list* list;
}

%token QUOTE
%token SINGLE_QUOTE
%token PLUS
%token MINUS
%token MULT
%token DIV
%token BITAND
%token BITOR
%token BITXOR
%token BITNOT
%token MOD
%token SEMICOLONCOLON
%token LPAREN
%token RPAREN
%token LBRACE
%token RBRACE
%token LBRACKET
%token RBRACKET
%token LS
%token GTR
%token COMMA
%token ASSIGN
%token NOT
%token DOT
%token TYPEOF
%token RETURN
%token ENDL
%token IF
%token EQL
%token WHILE
%token VAR_INIT
%token GEQL
%token LEQL
%token NEQL
%token AND
%token OR
%token ELSE
%token ELSE_IF
%token DO
%token FOR
%token LOOP
%token PLUS_ASSIGN
%token MINUS_ASSIGN
%token MULT_ASSIGN
%token DIV_ASSIGN
%token MOD_ASSIGN
%token AND_ASSIGN
%token OR_ASSIGN
%token XOR_ASSIGN
%token RSHIFT
%token LSHIFT
%token LEFTSHIFT_ASSIGN
%token RIGHTSHIFT_ASSIGN
%token INC
%token DEC
%token NEG
%token FUNCTION_DEC
%token CALL_FUNC
%token CALL_METHOD
%token ARRAY
%token ARRAY_ITEM
%token THIS
%token NEW
%token PROTO
%token METHOD_DEC
%token OBJ_PROP
%token SWITCH
%token BREAK
%token CASE
%token COLON
%token DEFAULT
%token QUESTION
%token NULL_TYPE
%token NaN
%token CALL_REG
%token BINARY
%token UNARY
%token EXPR
%token <var_name> BOOL
%token <list> ARGS
%token <num> NUM
%token <var_name> VAR
%token <var_name> STRING_LITERAL
%token <var_name> FUNCTION
%token <var_name> REG_EXP
%type <node> loop_state
%type <node> cond
%type <node> expr
%type <node> stmt 
%type <node> var_dec
%type <node> return_stmt
%type <node> typeof_stmt
%type <list> stmts
%type <list> args
%type <node> assign
%type <node> unary_operation
%type <node> array_dec
%type <list> else_if_stmts
%type <node> else_if
%type <list> arg_statement
%token <list> array_elems
%type <node> switch_stmt
%type <list> switch_stmts
%type <list> cond_stmts
%type <list> stmts_list
%type <list> cond_expr
%type <node> stmt_list_member
%type <node> open_stmt
%type <node> closed_stmt
%type <list> loop_stmt
%type <node> initialization


%left PLUS MULT DIV MINUS BITAND BITOR BITXOR NOT
%right LBRACKET RBRACKET
%right ASSIGN BITNOT
%left VAR_INIT
%left INC DEC
%left MOD RSHIFT LSHIFT
%left COMMA
%left VAR
%left QUESTION
%left COLON
%left CASE
%right LPAREN
%left RPAREN
%left SEMICOLON
%left AND
%left OR
%left EQL GTR LS GEQL LEQL NEQL

%%

program:
	stmts {program_block = $1;}
;

stmts:
	stmt {$$ = create_block($1);}
	| stmts stmt {$$ = add_back($1, $2);}
;

stmt:
	open_stmt SEMICOLON {$$ = $1;}
	| closed_stmt {$$ = $1;}
;

open_stmt:
	var_dec {$$ = $1;}
	| array_dec {$$ = $1;}
	| assign {$<node>$ = $<node>1;}
	| return_stmt {$<node>$ = $<node>1;}
	| unary_operation {$<node>$ = $<node>1;}
	| typeof_stmt {$<node>$ = $<node>1;}
	| VAR LPAREN arg_statement {$$ = call_function($1, $3);}
	| FUNCTION LPAREN arg_statement {$$ = call_defined_func($1, $3);}
	| VAR DOT VAR LPAREN arg_statement {$$ = call_obj_method($1, $3, $5);}
	| BREAK {$$ = create_break();}
	| expr QUESTION cond_stmts COLON cond_stmts {$$ = create_cond_operator($1, $3, $5);}
;

closed_stmt:
	loop_state {$<node>$ = $<node>1;}
	| FUNCTION_DEC VAR LPAREN arg_statement LBRACE stmts RBRACE {$$ = create_new_function($2, $4, $6);}
	| VAR DOT PROTO DOT VAR ASSIGN FUNCTION_DEC LPAREN arg_statement LBRACE stmts RBRACE
	{
		$$ = create_obj_method($1, $5, $9, $11);
	}
	| SWITCH LPAREN expr RPAREN LBRACE switch_stmts RBRACE {$$ = create_switch_statement($3, $6);}
;

unary_operation:
	VAR INC {$$ = new_unary_operator($1, INC);}
	| VAR DEC {$$ = new_unary_operator($1, DEC);}
	| MINUS expr {$$ = new_unary_expr($2, NEG);}
;

var_dec: 
	VAR_INIT VAR {$<node>$ = create_new_var_dec($2); free($2);}
;

array_dec:
	var_dec ASSIGN LBRACKET args RBRACKET
	{
		$$ = create_array($1, $4);
		
	}
	| var_dec ASSIGN ARRAY args RPAREN
	{
		$$ = create_array($1, $4);
	}
	| var_dec ASSIGN ARRAY RPAREN {$$ = create_array($1, 0);}
;

expr:
	NUM {$$ = create_num ($1);}
	| STRING_LITERAL {$$ = create_string_literal($1);}
	| TYPEOF expr {$$ = create_typeof($2);}
	| VAR {$$ = create_new_var($1);}
	| BOOL {$$ = create_boolean($1);}
	| REG_EXP {$$ = create_regular($1);}
	| expr PLUS expr {$$ = new_binary_operator($1, $3, PLUS);}
	| expr MINUS expr {$$ = new_binary_operator($1, $3, MINUS);}
	| expr MULT expr {$$ = new_binary_operator($1, $3, MULT);}
	| expr DIV expr {$$ = new_binary_operator($1, $3, DIV);}
	| expr BITAND expr {$$ = new_binary_operator($1, $3, BITAND);}
	| expr BITOR expr {$$ = new_binary_operator($1, $3, BITOR);}
	| expr BITXOR expr {$$ = new_binary_operator($1, $3, BITXOR);}
	| expr MOD expr {$$ = new_binary_operator($1, $3, MOD);}
	| expr RSHIFT expr {$$ = new_binary_operator($1, $3, RSHIFT);}
	| expr LSHIFT expr {$$ = new_binary_operator($1, $3, LSHIFT);}
	| LPAREN expr RPAREN	{$$ = $2;}
	| FUNCTION LPAREN arg_statement {$$ = call_defined_func($1, $3);}
	| VAR LPAREN arg_statement {$$ = call_function($1, $3);}
	| VAR DOT VAR LPAREN arg_statement {$$ = call_obj_method($1, $3, $5);}
	| REG_EXP DOT VAR LPAREN arg_statement {$$ = call_regular_method($1, $3, $5);}
	| VAR LBRACKET expr RBRACKET {$$ = create_array_item($1, $3);}
	| VAR DOT VAR {$$ = create_get_obj_prop($1, $3);}
	| THIS DOT VAR {$$ = create_obj_prop($3);}
	| expr QUESTION cond_expr COLON cond_expr {$$ = create_cond_operator($1, $3, $5);}
	| NULL_TYPE {$$ = create_null();}
	| cond {$$ = $1;}
	| unary_operation {$$ = $1;}
;

assign: 
	expr ASSIGN expr {$<node>$ = new_binary_operator($<node>1, $<node>3, ASSIGN);}
	| var_dec ASSIGN expr
	{
		$<node>$ = new_binary_operator($1, $3, ASSIGN);
	}
	| expr PLUS_ASSIGN expr {$$ = new_binary_operator($1, $3, PLUS_ASSIGN);}
	| expr MINUS_ASSIGN expr {$$ = new_binary_operator($1, $3, MINUS_ASSIGN);}
	| expr MULT_ASSIGN expr {$$ = new_binary_operator($1, $3, MULT_ASSIGN);}
	| expr DIV_ASSIGN expr {$$ = new_binary_operator($1, $3, DIV_ASSIGN);}
	| expr MOD_ASSIGN expr {$$ = new_binary_operator($1, $3, MOD_ASSIGN);}
	| expr AND_ASSIGN expr {$$ = new_binary_operator($1, $3, AND_ASSIGN);}
	| expr OR_ASSIGN expr {$$ = new_binary_operator($1, $3, OR_ASSIGN);}
	| expr XOR_ASSIGN expr {$$ = new_binary_operator($1, $3, XOR_ASSIGN);}
	| expr LEFTSHIFT_ASSIGN expr {$$ = new_binary_operator($1, $3, LEFTSHIFT_ASSIGN);}
	| expr RIGHTSHIFT_ASSIGN expr {$$ = new_binary_operator($1, $3, RIGHTSHIFT_ASSIGN);}
	| var_dec ASSIGN NEW expr {$$ = init_obj($1, $4);}
;

cond:
	NOT expr {$$ = new_unary_expr($2, NOT);}
	| expr EQL expr {$$ = new_binary_operator($1, $3, EQL);}
	| expr GTR expr {$<node>$ = new_binary_operator($<node>1, $<node>3, GTR);}
	| expr LS expr {$$ = new_binary_operator($1, $3, LS);}
	| expr GEQL expr {$$ = new_binary_operator($1, $3, GEQL);}
	| expr LEQL expr {$$ = new_binary_operator($1, $3, LEQL);}
	| expr NEQL expr {$$ = new_binary_operator($1, $3, NEQL);}
	| expr OR expr {$$ = new_binary_operator($1 , $3, OR);}
	| expr AND expr {$$ = new_binary_operator($1, $3, AND);}
;

loop_state:
	IF LPAREN expr RPAREN loop_stmt
	{
		$<node>$ = create_loop_statement($3, $5, IF);
	}
	|IF LPAREN expr RPAREN loop_stmt ELSE loop_stmt
	{
		$$ = create_else_loop($3, $5, $7, ELSE);
	}
	| IF LPAREN expr RPAREN loop_stmt else_if_stmts
	{
		$$ = create_else_if_statement($3, $5, $6);
	}
	| IF LPAREN expr RPAREN loop_stmt else_if_stmts ELSE loop_stmt
	{
		$$ = create_elseif_else_loop($3, $5, $6, $8);
	}	
	| WHILE LPAREN expr RPAREN loop_stmt
	{
		$$ = create_loop_statement($3, $5, WHILE);
	}
	| DO loop_stmt WHILE LPAREN expr RPAREN SEMICOLON 
	{
		$$ = create_loop_statement($5, $2, DO);
	}
	| FOR LPAREN initialization expr SEMICOLON expr RPAREN loop_stmt
	{
		$$ = create_for_loop($3, $6, $4, $8, FOR);
	}
;


initialization:
	SEMICOLON{$$ = create_semi();}
	| assign SEMICOLON{$$ = $1;}
	| expr SEMICOLON{$$ = $1;}
;

stmt_list_member:
	open_stmt {$$ = $1;}
	| closed_stmt {$$ = $1;}
;

cond_stmts:
	stmt_list_member {$$ = create_block($1);}
	| LPAREN stmts_list RPAREN {$$ = $2;}
;

stmts_list:
	stmt_list_member {$$ = create_block($1);}
	| stmts_list COMMA stmt_list_member {$$ = add_back($1, $3);}
;

cond_expr:
	expr {$$ = create_block($1);}
	| LPAREN stmts_list COMMA expr RPAREN {$$ = add_back ($2, $4);}
;

switch_stmts:
	switch_stmt {$$ = create_block($1);}
	| switch_stmts switch_stmt {$$ = add_back($1, $2);}
;

switch_stmt:
	CASE expr COLON {$$ = create_null_case($2);}
	|CASE expr COLON stmts {$$ = create_case($2, $4);}
	| DEFAULT COLON {$$ = create_null_default();}
	| DEFAULT COLON stmts {$$ = create_default($3);}
	
;
	
else_if:
	ELSE_IF LPAREN expr RPAREN loop_stmt { $$ = create_loop_statement($3, $5, ELSE_IF);}
;

else_if_stmts:
	else_if	{$$ = create_block($1);}
	| else_if_stmts else_if {$$ = add_back($1, $2);}
;

loop_stmt:
	stmt {$$ = create_single_stmt($1);}
	| LBRACE stmts RBRACE {$$ = $2;}
;

return_stmt:
	RETURN expr { $$ = create_return($2);}
;

typeof_stmt:
	TYPEOF expr {$$ = create_typeof($2);}
;

arg_statement:
	RPAREN {$$ = NULL;}
	| args RPAREN {$$ = $1;}
;

args:
	expr {$$ = create_block($1);}
	| args COMMA expr {$$ = add_back($1, $3);}
;


%%

YYSTYPE yylval;

void yyerror(char const *s)
{
	fprintf (stderr, "%s\n", s);
}

struct ast_list **parse_files(char **file, int sz)
{
	FILE *in;
	struct ast_list **pblock = jmalloc(sizeof(*pblock) * sz);
	int i;

	if (file[1] == NULL || strcmp(file[1], "--test") == 0) {
		fprintf(stderr, "*Please supply at least one file!\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < sz-1; i++) {
		pblock[i] = jmalloc(sizeof(*pblock[i]));
		in = fopen(file[i+1], "r");

		if (in == NULL) {
			fprintf(stderr, "Could not open Javascript file\n");
			exit(EXIT_FAILURE);
		}

		yyrestart(in);
		(void) yyparse(&yylval);

		/* Should free programblock to have fresh block for each file */
		memcpy(pblock[i], program_block, sizeof(*pblock[i]));
		free(program_block);

		fclose(in);
	}
	yylex_destroy();

	return pblock;
}

struct ast *execute_block(char *file, int filenum, struct ast_list *pblock)
{
	if (pblock == NULL) {
		fprintf(stderr, "Test does not exist!\n");
		exit(EXIT_FAILURE);
	}

	printf("\n\t~File[%d]: %s~\n", filenum, file);

	return statements_spider(pblock);
}

struct ast **execute_all(char **list, int sz, struct ast_list **blocks)
{
	int i;
	struct ast **return_val = jmalloc(sizeof(*return_val) * (sz-1));

	for (i = 0; i < sz - 1; i++) {
		return_val[i] = execute_block(list[i+1], i, blocks[i]);

		if (return_val[i]->data.return_val->data.var.type == NUM)
			printf("%g\n",
				return_val[i]->data.return_val->data.var.val);
		else if (return_val[i]->data.return_val->data.var.type ==
				STRING_LITERAL)
			printf("%s\n",
			       return_val[i]->data.return_val->data.var.string);
		else if (return_val[i]->data.return_val->data.var.type ==
				BOOL)
			printf("%s\n",
			       return_val[i]->data.return_val->data.var.string);
	}

	return return_val;
}
