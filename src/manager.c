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

#include <string.h>

#include <gdbus.h>

#include "pacrunner.h"

struct proxy_config {
	char *path;
	char *sender;
	DBusConnection *conn;
	guint watch;

	char *url;
	char *domain;
};

static unsigned int next_config_number = 0;

static GHashTable *config_list;

static void destroy_config(gpointer data)
{
	struct proxy_config *config = data;

	DBG("path %s", config->path);

	if (config->watch > 0)
		g_dbus_remove_watch(config->conn, config->watch);

	g_free(config->domain);
	g_free(config->url);

	g_free(config->sender);
	g_free(config->path);
	g_free(config);
}

static void disconnect_callback(DBusConnection *conn, void *user_data)
{
	struct proxy_config *config = user_data;

	DBG("path %s", config->path);

	config->watch = 0;

	g_hash_table_remove(config_list, config->path);
}

static struct proxy_config *create_config(DBusConnection *conn,
						const char *sender,
						const char *url,
						const char *domain)
{
	struct proxy_config *config;

	config = g_try_new0(struct proxy_config, 1);
	if (config == NULL)
		return NULL;

	config->path = g_strdup_printf("%s/configuration%d", PACRUNNER_PATH,
							next_config_number++);
	config->sender = g_strdup(sender);

	config->url = g_strdup(url);
	config->domain = g_strdup(domain);

	DBG("path %s", config->path);

	config->conn = conn;
	config->watch = g_dbus_add_disconnect_watch(conn, sender,
					disconnect_callback, config, NULL);

	return config;
}

static DBusMessage *create_proxy_config(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	DBusMessageIter iter, array;
	struct proxy_config *config;
	const char *sender, *url = NULL, *domain = NULL;

	sender = dbus_message_get_sender(msg);

	dbus_message_iter_init(msg, &iter);
	dbus_message_iter_recurse(&iter, &array);

	while (dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter entry, value;
		const char *key;

		dbus_message_iter_recurse(&array, &entry);
		dbus_message_iter_get_basic(&entry, &key);

		dbus_message_iter_next(&entry);
		dbus_message_iter_recurse(&entry, &value);

		switch (dbus_message_iter_get_arg_type(&value)) {
		case DBUS_TYPE_STRING:
			if (g_str_equal(key, "URL") == TRUE) {
				dbus_message_iter_get_basic(&value, &url);
				if (strlen(url) == 0)
					url = NULL;
			} else if (g_str_equal(key, "Domain") == TRUE) {
				dbus_message_iter_get_basic(&value, &domain);
				if (strlen(domain) == 0)
					domain = NULL;
			}
			break;
		}

		dbus_message_iter_next(&array);
	}

	DBG("sender %s url %s domain %s", sender, url, domain);

	if (url == NULL)
		return g_dbus_create_error(msg,
					PACRUNNER_ERROR_INTERFACE ".Failed",
					"Missing URL in configuration");

	config = create_config(conn, sender, url, domain); 
	if (config == NULL)
		return g_dbus_create_error(msg,
					PACRUNNER_ERROR_INTERFACE ".Failed",
					"Memory allocation failed");

	g_hash_table_insert(config_list, config->path, config);

	return g_dbus_create_reply(msg, DBUS_TYPE_OBJECT_PATH, &config->path,
							DBUS_TYPE_INVALID);
}

static DBusMessage *destroy_proxy_config(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	const char *sender, *path;

	sender = dbus_message_get_sender(msg);

	dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &path,
							DBUS_TYPE_INVALID);

	DBG("sender %s path %s", sender, path);

	g_hash_table_remove(config_list, path);

	return g_dbus_create_reply(msg, DBUS_TYPE_INVALID);
}

static GDBusMethodTable manager_methods[] = {
	{ "CreateProxyConfiguration",  "a{sv}", "o", create_proxy_config  },
	{ "DestroyProxyConfiguration", "o",     "",  destroy_proxy_config },
	{ },
};

static DBusConnection *connection;

int __pacrunner_manager_init(DBusConnection *conn)
{
	DBG("");

	connection = dbus_connection_ref(conn);

	config_list = g_hash_table_new_full(g_str_hash, g_str_equal,
						NULL, destroy_config);

	g_dbus_register_interface(connection, PACRUNNER_MANAGER_PATH,
						PACRUNNER_MANAGER_INTERFACE,
						manager_methods,
						NULL, NULL, NULL, NULL);

	return 0;
}

void __pacrunner_manager_cleanup(void)
{
	DBG("");

	g_hash_table_destroy(config_list);

	g_dbus_unregister_interface(connection, PACRUNNER_MANAGER_PATH,
						PACRUNNER_MANAGER_INTERFACE);

	dbus_connection_unref(connection);
}
