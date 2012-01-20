/*
 *
 *  PACrunner - Proxy configuration daemon
 *
 *  Copyright (C) 2010-2011  Intel Corporation. All rights reserved.
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

#include "pacrunner.h"

GList **__pacrunner_manual_parse_servers(char **servers)
{
	return NULL;
}

void __pacrunner_manual_destroy_servers(GList **servers)
{
	return;
}

GList **__pacrunner_manual_parse_excludes(char **excludes)
{
	return NULL;
}

void __pacrunner_manual_destroy_excludes(GList **excludes)
{
	return;
}

char *__pacrunner_manual_execute(const char *url, const char *host,
				 GList **servers, GList **excludes)
{
	DBG("url %s host %s", url, host);

	if (servers == NULL || servers[0] == NULL)
		return NULL;

	return g_strdup_printf("PROXY %s", (char *)servers[0]->data);
}

int __pacrunner_manual_init(void)
{
	DBG("");

	return 0;
}

void __pacrunner_manual_cleanup(void)
{
	DBG("");
}
