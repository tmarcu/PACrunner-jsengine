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

#include <gdbus.h>

#include "pacrunner.h"

struct jsrun_data {
	DBusConnection *conn;
	DBusMessage *msg;
	GThread *thread;
};

static void jsrun_free(gpointer data)
{
	struct jsrun_data *jsrun = data;

	dbus_message_unref(jsrun->msg);
	dbus_connection_unref(jsrun->conn);
	g_free(jsrun);
}

static gpointer jsrun_thread(gpointer data)
{
	struct jsrun_data *jsrun = data;
	const char *sender, *url, *host;
	char *result;
	static char direct[] = "DIRECT";

	sender = dbus_message_get_sender(jsrun->msg);

	DBG("sender %s", sender);

	dbus_message_get_args(jsrun->msg, NULL, DBUS_TYPE_STRING, &url,
						DBUS_TYPE_STRING, &host,
							DBUS_TYPE_INVALID);

	DBG("url %s host %s", url, host);

	result = pacrunner_proxy_lookup(url, host);

	DBG("result %s", result);

	if (result == NULL)
		result = direct;

	g_dbus_send_reply(jsrun->conn, jsrun->msg, DBUS_TYPE_STRING, &result,
							DBUS_TYPE_INVALID);

	if (result != direct)
		g_free(result);

	jsrun_free(jsrun);

	g_thread_exit(NULL);

	return NULL;
}

static DBusMessage *find_proxy_for_url(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct jsrun_data *jsrun;

	jsrun = g_try_new0(struct jsrun_data, 1);
	if (jsrun == NULL)
		return g_dbus_create_error(msg,
					PACRUNNER_ERROR_INTERFACE ".Failed",
							"Out of memory");

	jsrun->conn = dbus_connection_ref(conn);
	jsrun->msg = dbus_message_ref(msg);

	jsrun->thread = g_thread_create(jsrun_thread, jsrun, FALSE, NULL);
	if (jsrun->thread == NULL) {
		jsrun_free(jsrun);
		return g_dbus_create_error(msg,
					PACRUNNER_ERROR_INTERFACE ".Failed",
						"Thread execution failed");
	}

	return NULL;
}

static GDBusMethodTable client_methods[] = {
	{ "FindProxyForURL", "ss", "s", find_proxy_for_url,
						G_DBUS_METHOD_FLAG_ASYNC  },
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
