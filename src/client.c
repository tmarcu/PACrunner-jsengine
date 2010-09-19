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

#include <gdbus.h>

#include "pacrunner.h"

static DBusMessage *find_proxy_for_url(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	const char *sender, *url, *host, *result;

	sender = dbus_message_get_sender(msg);

	DBG("sender %s", sender);

	dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &url,
						DBUS_TYPE_STRING, &host,
							DBUS_TYPE_INVALID);

	DBG("url %s host %s", url, host);

	result = __pacrunner_mozjs_execute(url, host);
	if (result == NULL)
		return g_dbus_create_error(msg,
					PACRUNNER_ERROR_INTERFACE ".Failed",
							"PAC execution failed");

	DBG("result %s", result);

	return g_dbus_create_reply(msg, DBUS_TYPE_STRING, &result,
							DBUS_TYPE_INVALID);
}

static GDBusMethodTable client_methods[] = {
	{ "FindProxyForURL", "ss", "s", find_proxy_for_url },
	{ },
};

static DBusConnection *connection;

int __pacrunner_client_init(DBusConnection *conn)
{
	DBG("");

	connection = dbus_connection_ref(conn);

	g_dbus_register_interface(connection, PACRUNNER_CLIENT_PATH,
						PACRUNNER_CLIENT_INTERFACE,
						client_methods,
						NULL, NULL, NULL, NULL);

	return 0;
}

void __pacrunner_client_cleanup(void)
{
	DBG("");

	g_dbus_unregister_interface(connection, PACRUNNER_CLIENT_PATH,
						PACRUNNER_CLIENT_INTERFACE);

	dbus_connection_unref(connection);
}
