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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <time.h>
#include "symbols.h"
#include "tree.h"
#include "spider.h"
#include "parser.h"
#include "string_methods.h"
#include "reg_expr_methods.h"
#include "javascript_functions.h"

struct ast *dns_domain_is(struct ast_list *args, char *object_name)
{
	struct ast *host = eval_expr(args->top, object_name);
	struct ast *domain = eval_expr(args->top->next, object_name);
	char *host_str = host->data.var.string;
	char *domain_str = domain->data.var.string;
	char *compare_str;
	int com_index;
	struct ast *retval = jmalloc(sizeof(*retval));

	retval->type = VAR;
	retval->data.var.type = BOOL;

	if (strlen(host_str) < strlen(domain_str)) {
		retval->data.var.val = 0;
		free_expr(host);
		free_expr(domain);
		return retval;
	}

	com_index = strlen(host_str) - strlen(domain_str);
	compare_str = (char *) &host_str[com_index];

	if (strcmp(compare_str, domain_str) == 0) {
		retval->data.var.val = 1;
		free_expr(host);
		free_expr(domain);
		return retval;
	}

	retval->data.var.val = 0;
	free_expr(host);
	free_expr(domain);

	return retval;
}

char *get_substring(char *string, char *sub_str)
{
	int i = 0;
	while (sub_str != (char *) &string[i])
		i++;

	return (char *) &string[i];
}

int get_index(char *string, char *sub)
{
	int i;
	for (i = 0; (char *) &string[i] != sub; i++)
		/* Keep iterating to get index */

	i++;

	return i;
}
struct ast *dns_domain_levels(struct ast_list *args, char *object_name)
{
	struct ast *host = eval_expr(args->top, object_name);
	char *split = strdup(".");
	char *string = host->data.var.string;
	char *sub_str;
	struct ast *retval = jmalloc(sizeof(*retval));

	retval->type = VAR;
	retval->data.var.type = NUM;
	retval->data.var.val = 0;

	sub_str = strstr(string, split);

	while (sub_str != NULL) {
		retval->data.var.val++;
		string = (char *) &string[get_index(string, sub_str)];
		sub_str = strstr(string, split);
	}

	free_expr(host);
	free(split);

	return retval;
}

int valid_ip_add(regmatch_t *match_array, char *string, int size)
{
	int i, j, k, test;
	int match_size;

	for (i = 1; i < size; i++) {
		match_size = match_array[i].rm_eo - match_array[i].rm_so + 1;
		char match[match_size];
		j = 0;
		for (k = match_array[i].rm_so; k < match_array[i].rm_eo; k++) {
			match[j] = string[k];
			j++;
		}
		match[j] = '\0';
		test = atoi(match);
		if (test > 255)
			return 0;
	}
	return 1;
}

int get_dot(char *string)
{
	int i = 0;
	while (string[i] != '.') {
		if (string[i] == '\0')
			return -1;
		i++;
	}
	return i;
}

int convert_ipaddr(char *string)
{
	int i, start, j, k, retval;
	int bytes[4] = {0};

	i = get_dot(string);
	start = 0;
	k = 0;
	while (i != -1) {
		i += start;
		char match[i - start + 1];
		j = 0;
		while (start < i) {
			match[j] = string[start];
			j++;
			start++;
		}
		match[j] = '\0';
		bytes[k] = atoi(match);
		start++;
		i = get_dot((char *) &string[start]);
	}
	retval = ((bytes[0] & 0xff) << 24) | ((bytes[1] & 0xff) << 16) |
			((bytes[2] & 0xff) << 8) | (bytes[3] & 0xff);
	return retval;
}

struct ast *is_in_net(struct ast_list *args, char *object_name)
{
	struct ast *ipaddr = eval_expr(args->top, object_name);
	struct ast *pattern = eval_expr(args->top->next, object_name);
	struct ast *mask_str = eval_expr(args->top->next->next, object_name);
	struct ast *retval = jmalloc(sizeof(*retval));
	regex_t reg;
	int match_size, k;
	int host, pat, mask;
	char *posix;
	char *addr;

	retval->type = VAR;
	retval->data.var.type = BOOL;

	posix = convert_to_posix(
			 "^(\\d{1,4})\\.(\\d{1,4})\\.(\\d{1,4})\\.(\\d{1,4})$");
	if (regcomp(&reg, posix, 0) != 0)
		printf("ERROR IN IS IN NET\n");
	match_size = (int) reg.re_nsub + 1;
	regmatch_t match_array[match_size];
	k = regexec(&reg, ipaddr->data.var.string, match_size,
							match_array, 0);

	if (k != 0) {
		addr = resolve(ipaddr->data.var.string);
		if (addr == NULL)
			goto END;

		free(ipaddr->data.var.string);
		ipaddr->data.var.string = addr;
	} else {
		if (!valid_ip_add(match_array, ipaddr->data.var.string,
							match_size)) {
			retval->data.var.val = 0;

			free_expr(ipaddr);
			free_expr(pattern);
			free_expr(mask_str);
			free(posix);
			return retval;
		}
	}

	host = convert_ipaddr(ipaddr->data.var.string);
	pat = convert_ipaddr(pattern->data.var.string);
	mask = convert_ipaddr(mask_str->data.var.string);

	retval->data.var.val = (double) ((host & mask) == (pat & mask));

END:
	free_expr(ipaddr);
	free_expr(pattern);
	free_expr(mask_str);
	free(posix);
	regfree(&reg);

	return retval;
}


struct ast *is_plain_hostname(struct ast_list *args,  char *object_name)
{
	struct ast *host = eval_expr(args->top, object_name);
	struct ast *retval = jmalloc(sizeof(*retval));

	retval->type = VAR;
	retval->data.var.type = BOOL;

	if (get_dot(host->data.var.string) != -1) {
		retval->data.var.val = 0;
		free_expr(host);
		return retval;
	}

	retval->data.var.val = 1;

	free_expr(host);

	return retval;
}

struct ast *local_host_or_domain_is(struct ast_list *args,  char *object_name)
{
	struct ast *host = eval_expr(args->top, object_name);
	struct ast *hostdom = eval_expr(args->top->next, object_name);
	char *host_str = host->data.var.string;
	char *hostdom_str = host->data.var.string;
	struct ast *retval = jmalloc(sizeof(*retval));
	char *temp;

	retval->type = VAR;
	retval->data.var.type = BOOL;

	if (get_dot(host_str) == -1) {
		temp = strstr(hostdom_str, host_str);
		if (temp == hostdom_str)
			retval->data.var.val = 1;
		else
			retval->data.var.val = 0;
	} else {
		if (strcmp(host_str, hostdom_str) == 0)
			retval->data.var.val = 1;
		else
			retval->data.var.val = 0;
	}

	free_expr(host);
	free_expr(hostdom);

	return retval;
}

char *convert_to_reg(char *pat)
{
	int size = 0;
	unsigned int i;
	char *new;

	for (i = 0; i < strlen(pat); i++) {
		switch (pat[i]) {
		case '.':
		case '*':
			size++;
		default:
			size++;
		}
	}
	size++;

	new = jmalloc(sizeof(char) * size);
	size = 0;

	for (i = 0; i < strlen(pat); i++) {
		switch (pat[i]) {
		case '.':
			new[size] = '\\';
			size++;
			new[size] = '.';
		break;
		case '*':
			new[size] = '.';
			size++;
			new[size] = '*';
		break;
		case '?':
			new[size] = '.';
		break;
		default:
			new[size] = pat[i];
		break;
		}
		size++;
	}
	return new;
}

struct ast *sh_exp_match(struct ast_list *args,  char *object_name)
{
	struct ast *url = eval_expr(args->top, object_name);
	struct ast *pat = eval_expr(args->top->next, object_name);
	char *url_str = url->data.var.string;
	char *pat_str = pat->data.var.string;
	struct ast *retval = jmalloc(sizeof(*retval));
	char *reg_pat;
	char *posix_pat;
	regex_t reg;

	retval->type = VAR;
	retval->data.var.type = BOOL;

	reg_pat = convert_to_reg(pat_str);

	posix_pat = convert_to_posix(reg_pat);

	if (regcomp(&reg, posix_pat, REG_NOSUB) != 0)
		printf("ERROR IN IS IN NET\n");

	if (regexec(&reg, url_str, 0, NULL, 0) != 0)
		retval->data.var.val = 0;
	else
		retval->data.var.val = 1;

	free_expr(url);
	free_expr(pat);
	free(reg_pat);
	free(posix_pat);
	regfree(&reg);

	return retval;
}

struct ast *my_ip_address(struct ast_list *args,  char *object_name)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	char buf[64];
	struct ifaddrs *addr, *ifa;
	struct sockaddr_in *s_four;
	struct sockaddr_in6 *s_six;
	void *addr_str;

	if (getifaddrs(&addr) != 0)
		printf("ERROR GETTING IF ADDRS\n");

	for (ifa = addr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;
		if (strcmp(ifa->ifa_name, "lo") == 0)
			continue;

		switch (ifa->ifa_addr->sa_family) {
		case AF_INET:
			s_four = (struct sockaddr_in *) ifa->ifa_addr;
			addr_str = &s_four->sin_addr;
		break;
		case AF_INET6:
			s_six = (struct sockaddr_in6 *) ifa->ifa_addr;
			addr_str = &s_six->sin6_addr;
		break;
		default:
			continue;
		break;
		}

		if (!inet_ntop(ifa->ifa_addr->sa_family, addr_str, buf,
				sizeof(buf)))
			printf("inet_ntop() failed\n");
		else {
			retval->data.var.string = strdup(buf);
			break;
		}
	}

	freeifaddrs(addr);

	retval->type = VAR;
	retval->data.var.type = STRING_LITERAL;


	return retval;
}

struct ast *dns_resolve(struct ast_list *args,  char *object_name)
{
	struct ast *host = eval_expr(args->top, object_name);
	struct ast *retval = jmalloc(sizeof(*retval));
	char *host_str = host->data.var.string;
	char *addr;

	retval->type = VAR;
	retval->data.var.type = STRING_LITERAL;
	retval->data.var.string = NULL;

	addr = resolve(host_str);
	if (addr != NULL)
		retval->data.var.string = addr;
	else
		retval->type = NULL_TYPE;

	free_expr(host);

	return retval;
}

char *resolve(char *str)
{
	struct addrinfo *info;
	char addr[65];
	int err;
	if (getaddrinfo(str, NULL, NULL, &info) != 0)
		return NULL;

	err = getnameinfo(info->ai_addr, info->ai_addrlen,
				addr, 65, NULL, 0, NI_NUMERICHOST);
	if (err < 0)
		return strdup("error");

	freeaddrinfo(info);

	return strdup(addr);
}

struct ast *is_resolvable(struct ast_list *args,  char *object_name)
{
	struct ast *retval = jmalloc(sizeof(*retval));
	struct ast *host = eval_expr(args->top, NULL);
	char *addr;

	retval->type = VAR;
	retval->data.var.type = BOOL;

	addr = resolve(host->data.var.string);

	if (addr != NULL) {
		free(addr);
		retval->data.var.val = 1;
	} else
		retval->data.var.val = 0;

	free_expr(host);

	return retval;
}

int day_num(struct ast *day)
{
	if (day == NULL)
		return -1;
	char *day_str = day->data.var.string;

	switch (day_str[0]) {
	case 'S':
		if (strcmp(day_str, "SUN") == 0)
			return 0;
		if (strcmp(day_str, "SAT") == 0)
			return 6;
	break;
	case 'M':
		if (strcmp(day_str, "MON") == 0)
			return 1;
	break;
	case 'T':
		if (strcmp(day_str, "TUE") == 0)
			return 2;
		if (strcmp(day_str, "THU") == 0)
			return 4;
	break;
	case 'W':
		if (strcmp(day_str, "WED") == 0)
			return 3;
	break;
	case 'F':
		if (strcmp(day_str, "FRI") == 0)
			return 5;
	break;
	case 'G':
		if (strcmp(day_str, "GMT") == 0)
			return 8;
	break;
	default:
		printf("day not recognized\n");
	break;
	}

	return -1;
}

int time_num(struct ast *t)
{
	if (t == NULL)
		return 0;
	if (strcmp(t->data.var.string, "GMT") == 0)
		return 1;
	return 0;
}

int get_cur_num(int gmt)
{
	time_t raw;
	struct tm *real_time;
	time(&raw);

	if (gmt)
		real_time = gmtime(&raw);
	else
		real_time = localtime(&raw);

	return real_time->tm_wday;
}

double check_time(int start, int stop, int cur)
{
	if (stop == 8 || stop == -1) {
		if (cur == start)
			return 1;
	}

	if (start < stop) {
		if (cur >= start && cur <= stop)
			return 1;
		return 0;
	}
	stop += 7;
	if (cur < start)
		cur += 7;

	if (cur >= start && cur <= stop)
		return 1;
	return 0;
}

struct ast *weekday_range(struct ast_list *args,  char *object_name)
{
	struct ast *start, *stop, *retval, *gmt;
	int start_num, stop_num, gmt_num, cur_num;
	retval = jmalloc(sizeof(*retval));
	start = eval_expr(args->top, object_name);
	stop = NULL;
	gmt = NULL;
	if (args->top->next != NULL)
		stop = eval_expr(args->top->next, object_name);
	if (stop != NULL && args->top->next->next != NULL)
		gmt = eval_expr(args->top->next->next, object_name);

	retval->type = VAR;
	retval->data.var.type = BOOL;

	start_num = day_num(start);
	stop_num = day_num(stop);
	gmt_num = time_num(gmt);

	if (stop_num == 8 || gmt_num == 1)
		cur_num = get_cur_num(1);
	else
		cur_num = get_cur_num(0);

	retval->data.var.val = check_time(start_num, stop_num, cur_num);

	free_expr(start);
	if (stop != NULL)
		free_expr(stop);
	if (gmt != NULL)
		free_expr(gmt);

	return retval;
}

int get_arg_num(struct ast_list *args)
{
	struct ast *temp;
	int i = 0;
	temp = args->top;
	while (temp != NULL) {
		i++;
		temp = temp->next;
	}
	return i;
}

int is_last_gmt(struct ast_list *args, char *object_name)
{
	struct ast *temp, *test;
	temp = args->top;
	while (temp->next != NULL)
		temp = temp->next;

	test = eval_expr(temp, object_name);
	if (test->type != VAR)
		return 0;
	if (test->data.var.type != STRING_LITERAL)
		return 0;
	if (strcmp(test->data.var.string, "GMT") != 0)
		return 0;
	return 1;
}

struct tm *assign_time(time_t *utc, int gmt)
{
	if (gmt)
		return gmtime(utc);
	return localtime(utc);
}

int get_arg_hour(struct ast *arg, char *object_name)
{
	struct ast *temp = eval_expr(arg, object_name);
	int ret = (int) temp->data.var.val;
	free_expr(temp);

	return ret;
}

double time_range_one(struct ast_list *args, char *object_name, int gmt)
{
	int hour = get_arg_hour(args->top, object_name);

	time_t utc;
	struct tm *cur;
	time(&utc);
	cur = assign_time(&utc, gmt);

	if (hour != cur->tm_hour)
		return 0;
	return 1;
}

int between(int start, int end, int test, int base)
{
	if (start == 0 && end == 0)
		return 1;

	if (start >= end) {
		end += base;
		if (test < start)
			start += base;
	}

	if (test >= start && test <= end)
		return 1;
	return 0;
}

double time_range_two(struct ast_list *args, char *object_name, int gmt)
{
	int start = get_arg_hour(args->top, object_name);
	int end = get_arg_hour(args->top->next, object_name);

	time_t utc;
	struct tm *cur;
	time(&utc);
	cur = assign_time(&utc, gmt);

	if (between(start, end, cur->tm_hour, 24))
		return 1;
	return 0;
}

double time_range_four(struct ast_list *arg, char *obj, int gmt)
{
	int start_hour = get_arg_hour(arg->top, obj);
	int start_min = get_arg_hour(arg->top->next, obj);
	int end_hour = get_arg_hour(arg->top->next->next, obj);
	int end_min = get_arg_hour(arg->top->next->next->next, obj);

	time_t utc;
	struct tm *cur;
	time(&utc);
	cur = assign_time(&utc, gmt);

	if (!between(start_hour, end_hour, cur->tm_hour, 24))
		return 0;

	if (!between(start_min, end_min, cur->tm_min, 60))
		return 0;

	return 1;
}

double time_range_six(struct ast_list *arg, char *obj, int gmt)
{
	int start_hour = get_arg_hour(arg->top, obj);
	int start_min = get_arg_hour(arg->top->next, obj);
	int start_sec = get_arg_hour(arg->top->next->next, obj);
	struct ast *end = arg->top->next->next->next;
	int end_hour = get_arg_hour(end, obj);
	int end_min = get_arg_hour(end->next, obj);
	int end_sec = get_arg_hour(end->next->next, obj);

	time_t utc;
	struct tm *cur;
	time(&utc);
	cur = assign_time(&utc, gmt);

	if (!between(start_hour, end_hour, cur->tm_hour, 24))
		return 0;

	if (!between(start_min, end_min, cur->tm_min, 60))
		return 0;

	if (!between(start_sec, end_sec, cur->tm_sec, 60))
		return 0;

	return 1;
}

struct ast *time_range(struct ast_list *args,  char *object_name)
{
	int arg_num = get_arg_num(args);
	int last_gmt = is_last_gmt(args, object_name);
	struct ast *retval = jmalloc(sizeof(*retval));

	retval->type = VAR;
	retval->data.var.type = BOOL;
	retval->data.var.val = 0;

	if (last_gmt == 1)
		arg_num--;

	switch (arg_num) {
	case 1:
		retval->data.var.val = time_range_one(args,
					object_name, last_gmt);
	break;
	case 2:
		retval->data.var.val = time_range_two(args,
					object_name, last_gmt);
	break;
	case 4:
		retval->data.var.val = time_range_four(args,
					object_name, last_gmt);
	break;
	case 6:
		retval->data.var.val = time_range_six(args,
					object_name, last_gmt);
	break;
	default:
		printf("Incorrect arguments for time range\n");
	break;
	}
	return retval;
}

int get_date_type(struct ast *test, char *obj)
{
	struct ast *temp = eval_expr(test, obj);
	if (temp->data.var.type == STRING_LITERAL)
		return 2;
	if (temp->data.var.val > 999)
		return 3;
	return 1;
}

int get_arg_month(struct ast *arg, char *obj)
{
	struct ast *temp = eval_expr(arg, obj);
	char *mon = temp->data.var.string;

	if (strcmp(mon, "JAN") == 0)
		return 0;
	else if (strcmp(mon, "FEB") == 0)
		return 1;
	else if (strcmp(mon, "MAR") == 0)
		return 2;
	else if (strcmp(mon, "APR") == 0)
		return 3;
	else if (strcmp(mon, "MAY") == 0)
		return 4;
	else if (strcmp(mon, "JUN") == 0)
		return 5;
	else if (strcmp(mon, "JUL") == 0)
		return 6;
	else if (strcmp(mon, "AUG") == 0)
		return 7;
	else if (strcmp(mon, "SEP") == 0)
		return 8;
	else if (strcmp(mon, "OCT") == 0)
		return 9;
	else if (strcmp(mon, "NOV") == 0)
		return 10;
	else if (strcmp(mon, "DEC") == 0)
		return 11;

	return -1;
}

double date_range_one(struct ast_list *args, char *obj, int gmt)
{
	int type = get_date_type(args->top, obj);
	int val;

	time_t utc;
	struct tm *cur;

	time(&utc);
	cur = assign_time(&utc, gmt);

	switch (type) {
	case 1:
		val = get_arg_hour(args->top, obj);
		if (cur->tm_mday == val)
			return 1;
		return 0;
	break;
	case 2:
		val = get_arg_month(args->top, obj);
		if (cur->tm_mon == val)
			return 1;
		return 0;
	break;
	case 3:
		val = get_arg_hour(args->top, obj);
		if ((cur->tm_year + 1900) == val)
			return 1;
		return 0;
	break;
	default:
		printf("type not recognized\n");
	break;
	}

	return 0;
}

double date_compare(int s_day, int s_month, int s_year, int e_day,
				int e_month, int e_year, int gmt)
{
	time_t utc;
	struct tm *cur;
	time(&utc);
	cur = assign_time(&utc, gmt);

	if (!between(s_day, e_day, cur->tm_mday, 0))
		return 0;

	if (!between(s_month, e_month, cur->tm_mon, 31))
		return 0;

	if (!between(s_year, e_year, cur->tm_year, 3000))
		return 0;

	return 1;
}

double date_range_two(struct ast_list *arg, char *obj, int gmt)
{
	int type = get_date_type(arg->top, obj);
	int left, right;

	switch (type) {
	case 1:
		left = get_arg_hour(arg->top, obj);
		right = get_arg_hour(arg->top->next, obj);
		return date_compare(left, 0, 0, right, 0, 0,
							gmt);
	break;
	case 2:
		left = get_arg_month(arg->top, obj);
		right = get_arg_month(arg->top->next, obj);
		return date_compare(0, left, 0, 0, right, 0,
							gmt);
	break;
	case 3:
		left = get_arg_hour(arg->top, obj);
		right = get_arg_hour(arg->top->next, obj);
		return date_compare(0, 0, left, 0, 0, right,
							gmt);
	break;
	}

	return 0;
}

double date_range_four(struct ast_list *args, char *obj, int gmt)
{
	int type = get_date_type(args->top, obj);
	int one, two, three, four;
	struct ast *end;

	switch (type) {
	case 1:
		one = get_arg_hour(args->top, obj);
		two = get_arg_month(args->top->next, obj);
		end = args->top->next->next;

		three = get_arg_hour(end, obj);
		four = get_arg_month(end->next, obj);

		return date_compare(one, two, 0, three, four, 0,
							gmt);
	break;
	case 2:
		one = get_arg_month(args->top, obj);
		two = get_arg_hour(args->top->next, obj);
		end = args->top->next->next;

		three = get_arg_month(end, obj);
		four = get_arg_hour(end->next, obj);

		return date_compare(0, one, two, 0, three, four,
							gmt);
	break;
	}

	return 0;
}

double date_range_six(struct ast_list *args, char *obj, int gmt)
{
	int sd, sm, sy, ed, em, ey;
	struct ast *end;
	sd = get_arg_hour(args->top, obj);
	sm = get_arg_month(args->top->next, obj);
	sy = get_arg_hour(args->top->next->next, obj);
	end = args->top->next->next;
	ed = get_arg_hour(end, obj);
	em = get_arg_month(end->next, obj);
	ey = get_arg_hour(end->next->next, obj);

	return date_compare(sd, sm, sy, ed, em, ey, gmt);
}

struct ast *date_range(struct ast_list *args,  char *object_name)
{
	int arg_num = get_arg_num(args);
	int last_gmt = is_last_gmt(args, object_name);
	struct ast *retval = jmalloc(sizeof(*retval));

	retval->type = VAR;
	retval->data.var.type = BOOL;
	retval->data.var.val = 0;

	if (last_gmt == 1)
		arg_num--;

	switch (arg_num) {
	case 1:
		retval->data.var.val = date_range_one(args,
					object_name, last_gmt);
	break;
	case 2:
		retval->data.var.val = date_range_two(args,
					object_name, last_gmt);
	break;
	case 4:
		retval->data.var.val = date_range_four(args,
					object_name, last_gmt);
	break;
	case 6:
		retval->data.var.val = date_range_six(args,
					object_name, last_gmt);
	break;
	default:
		printf("Incorrect args dateRange\n");
	break;
	}

	return retval;
}
