/*
 *
 *  libproxy - A library for proxy configuration
 *
 *  Copyright (C) 2010-2011  Intel Corporation. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dbus/dbus.h>

#include "proxy.h"

struct _pxProxyFactory {
	DBusConnection *conn;
};

pxProxyFactory *px_proxy_factory_new(void)
{
	pxProxyFactory *factory;

	factory = malloc(sizeof(*factory));
	if (factory == NULL)
		return NULL;

	memset(factory, 0, sizeof(*factory));

	factory->conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, NULL);
	if (factory->conn == NULL) {
		free(factory);
		return NULL;
	}

	dbus_connection_set_exit_on_disconnect(factory->conn, FALSE);

	return factory;
}

void px_proxy_factory_free(pxProxyFactory *factory)
{
	if (factory == NULL)
		return;

	dbus_connection_close(factory->conn);

	free(factory);
}

static char **extract_result(const char *str)
{
	char **result;

	result = malloc(sizeof(char *) * 2);
	if (result == NULL)
		return NULL;

	result[0] = NULL;
	result[1] = NULL;

	if (strcasecmp(str, "DIRECT") == 0) {
		result[0] = strdup("direct://");
		return result;
	}

	if (strncasecmp(str, "PROXY ", 6) == 0) {
		int len = strlen(str + 6) + 8;
		result[0] = malloc(len);
		if (result[0] != NULL)
			sprintf(result[0], "http://%s", str + 6);
		return result;
	}

	if (strncasecmp(str, "SOCKS ", 6) == 0) {
		int len = strlen(str + 6) + 9;
		result[0] = malloc(len);
		if (result[0] != NULL)
			sprintf(result[0], "socks://%s", str + 6);
		return result;
	}

	if (strncasecmp(str, "SOCKS4 ", 7) == 0) {
		int len = strlen(str + 7) + 10;
		result[0] = malloc(len);
		if (result[0] != NULL)
			sprintf(result[0], "socks4://%s", str + 7);
		return result;
	}

	if (strncasecmp(str, "SOCKS5 ", 7) == 0) {
		int len = strlen(str + 7) + 10;
		result[0] = malloc(len);
		if (result[0] != NULL)
			sprintf(result[0], "socks5://%s", str + 7);
		return result;
	}

	return result;
}

char **px_proxy_factory_get_proxies(pxProxyFactory *factory, const char *url)
{
	DBusMessage *msg, *reply;
	const char *str = NULL;
	char *scheme, *host, *port, *path, **result;

	if (factory == NULL)
		return NULL;

	if (url == NULL)
		return NULL;

	msg = dbus_message_new_method_call("org.pacrunner",
			"/org/pacrunner/client", "org.pacrunner.Client",
							"FindProxyForURL");
	if (msg == NULL)
		goto direct;

	scheme = strdup(url);
	if (scheme == NULL) {
		dbus_message_unref(msg);
		goto direct;
	}

	host = strstr(scheme, "://");
	if (host != NULL) {
		*host = '\0';
		host += 3;
	} else {
		dbus_message_unref(msg);
		goto direct;
	}

	path = strchr(host, '/');
	if (path != NULL)
		*(path++) = '\0';

	port = strrchr(host, ':');
	if (port != NULL) {
		char *end;
		int tmp __attribute__ ((unused));

		tmp = strtol(port + 1, &end, 10);
		if (*end == '\0')
			*port = '\0';
	}

	dbus_message_append_args(msg, DBUS_TYPE_STRING, &url,
				DBUS_TYPE_STRING, &host, DBUS_TYPE_INVALID);
	free(scheme);

	reply = dbus_connection_send_with_reply_and_block(factory->conn,
							msg, -1, NULL);

	dbus_message_unref(msg);

	if (reply == NULL)
		goto direct;

	dbus_message_get_args(reply, NULL, DBUS_TYPE_STRING, &str,
							DBUS_TYPE_INVALID);

	if (str == NULL || strlen(str) == 0)
		str = "DIRECT";

	result = extract_result(str);

	dbus_message_unref(reply);

	return result;

direct:
	result = malloc(sizeof(char *) * 2);
	if (result == NULL)
		return NULL;

	result[0] = strdup("direct://");
	result[1] = NULL;

	return result;
}
