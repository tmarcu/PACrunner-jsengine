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
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdbool.h>
	#include "parser.h"
	char *get_string(char *str);
%}

%option bison-bridge
%option yylineno
%x MULTI_LINE_COMMENT

HEX_PREFIX 0[xX]

%%

[ \t\n]+

"/*"				{BEGIN(MULTI_LINE_COMMENT);}
<MULTI_LINE_COMMENT>"*/"	{BEGIN(INITIAL);}
<MULTI_LINE_COMMENT>.		{}



"//"[^\n]*	/* Single line comments ignore */

"var"		{return VAR_INIT;}

L?\"(\\.|[^\\"])*\"	{
			yylval->var_name = strndup(yytext+1, strlen(yytext - 1));
			yylval->var_name[strlen(yylval->var_name) - 1] = '\0';
			return STRING_LITERAL;
			}


-{0,1}[0-9]"."*[0-9]*	{yylval->num = atof(yytext); return NUM;}
"."*[0-9]+		{yylval->num = atof(yytext); return NUM;}
{HEX_PREFIX}[0-9|a-fA-F]*	{yylval->num = atof(yytext); return NUM;}

"."		{return DOT;}
"==" 		{return EQL;}
"!="		{return NEQL;}
">="		{return GEQL;}
"<="		{return LEQL;}
"&&"		{return AND;}
"||"		{return OR;}
"+="		{return PLUS_ASSIGN;}
"-="		{return MINUS_ASSIGN;}
"*="		{return MULT_ASSIGN;}
"/="		{return DIV_ASSIGN;}
"%="		{return MOD_ASSIGN;}
"&="		{return AND_ASSIGN;}
"|="		{return OR_ASSIGN;}
"^="		{return XOR_ASSIGN;}
"<<="		{return LEFTSHIFT_ASSIGN;}
">>="		{return RIGHTSHIFT_ASSIGN;}
"++"		{return INC;}
"--"		{return DEC;}

"null"		{return NULL_TYPE;}
"switch"	{return SWITCH;}
"case"		{return CASE;}
"break"		{return BREAK;}
"default"	{return DEFAULT;}
"Math.addtwo"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.cos"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.abs"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.acos"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.asin"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.atan"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.atan2"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.ceil"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.floor"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.log"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.pow"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.round"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.sqrt"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.tan"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.exp"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.max"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"Math.min"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"parseInt"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"toNumber"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"isNaN"		{yylval->var_name = strdup(yytext); return FUNCTION;}
"alert"		{yylval->var_name = strdup(yytext); return FUNCTION;}
"this"		{return THIS;}
"new"		{return NEW;}
"prototype"	{return PROTO;}
"dnsDomainIs"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"dnsDomainLevels"  {yylval->var_name = strdup(yytext); return FUNCTION;}
"isInNet"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"isPlainHostName" {yylval->var_name = strdup(yytext); return FUNCTION;}
"shExpMatch"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"localHostOrDomainIs"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"myIpAddress"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"dnsResolve"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"isResolvable"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"weekdayRange"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"timeRange"	{yylval->var_name = strdup(yytext); return FUNCTION;}
"dateRange"	{yylval->var_name = strdup(yytext); return FUNCTION;}


"if" 		{return IF;}
"else if"	{return ELSE_IF;}
"else"		{return ELSE;}
"do"		{return DO;}
"for"		{return FOR;}

"while"		{return WHILE;}
"return"	{return RETURN;}
"function"	{return FUNCTION_DEC;}
"typeof"	{return TYPEOF;}
"new Array("	{return ARRAY;}
"true"		{yylval->var_name = "true"; return BOOL;}
"false"		{yylval->var_name = "false"; return BOOL;}

[a-zA-Z_][a-zA-Z0-9_]* {yylval->var_name = strdup(yytext); return VAR;}

[/][a-zA-Z\\\^\$\*\+\?\(\)\.\|\{\}\,0-9\[\]\- ]+[/][igm]* {yylval->var_name =
						strdup(yytext); return REG_EXP;}

"?"		{return QUESTION;}
":"		{return COLON;}
"!"		{return NOT;}
"=" 		{return ASSIGN;}
","		{return COMMA;}
">"		{return GTR;}
"<"		{return LS;}
">>"		{return RSHIFT;}
"<<"		{return LSHIFT;}
"+"		{return PLUS;}
"-"		{return MINUS;}
"*"		{return MULT;}
[/]		{return DIV;}
"&"		{return BITAND;}
"|"		{return BITOR;}
"^"		{return BITXOR;}
"~"		{return BITNOT;}
"%"		{return MOD;}
";"		{return SEMICOLON;}
"("		{return LPAREN;}
")"		{return RPAREN;}
"{"		{return LBRACE;}
"}"		{return RBRACE;}
"["		{return LBRACKET;}
"]"		{return RBRACKET;}

.  {printf("unrecognized character: %s\n", yytext);}

%%
