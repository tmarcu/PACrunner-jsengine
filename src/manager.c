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
	char *script;
	char *server;
	char *interface;
	char *domainname;
	char *nameserver;
};

static unsigned int next_config_number = 0;

static GHashTable *config_list;

static void destroy_config(gpointer data)
{
	struct proxy_config *config = data;

	DBG("path %s", config->path);

	__pacrunner_mozjs_clear();

	if (config->watch > 0)
		g_dbus_remove_watch(config->conn, config->watch);

	g_free(config->nameserver);
	g_free(config->domainname);
	g_free(config->interface);
	g_free(config->server);
	g_free(config->script);
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

static void download_callback(char *content, void *user_data)
{
	struct proxy_config *config = user_data;

	DBG("url %s content %p", config->url, content);

	if (content == NULL) {
		pacrunner_error("Failed to retrieve PAC script");
		goto done;
	}

	if (__pacrunner_mozjs_set_script(config->interface, content) < 0)
		pacrunner_error("Failed to set retrieved PAC script");

done:
	g_free(content);
}

static struct proxy_config *create_config(DBusConnection *conn,
						const char *sender,
						const char *method,
						const char *url,
						const char *script,
						const char *server,
						const char *interface,
						const char *domainname,
						const char *nameserver)
{
	struct proxy_config *config;

	config = g_try_new0(struct proxy_config, 1);
	if (config == NULL)
		return NULL;

	config->path = g_strdup_printf("%s/configuration%d", PACRUNNER_PATH,
							next_config_number++);
	config->sender = g_strdup(sender);

	config->url = g_strdup(url);
	config->script = g_strdup(script);
	config->server = g_strdup(server);
	config->interface = g_strdup(interface);
	config->domainname = g_strdup(domainname);
	config->nameserver = g_strdup(nameserver);

	DBG("path %s", config->path);

	config->conn = conn;
	config->watch = g_dbus_add_disconnect_watch(conn, sender,
					disconnect_callback, config, NULL);

	if (g_strcmp0(method, "manual") == 0) {
		if (__pacrunner_mozjs_set_server(config->interface,
							config->server) < 0)
			pacrunner_error("Failed to set proxy server");
		return config;
	}

	if (g_strcmp0(method, "auto") != 0)
		return config;

	if (config->script != NULL) {
		if (__pacrunner_mozjs_set_script(config->interface,
							config->script) < 0)
			pacrunner_error("Failed to set provided PAC script");
	} else if (config->url != NULL) {
		if (__pacrunner_download_update(config->interface,
				config->url, download_callback, config) < 0)
			pacrunner_error("Failed to start PAC script download");
	}

	return config;
}

static DBusMessage *create_proxy_config(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	DBusMessageIter iter, array;
	struct proxy_config *config;
	const char *sender, *method = NULL;
	const char *url = NULL, *script = NULL;
	const char *server = NULL, *exclude = NULL;
	const char *interface = NULL, *domainname = NULL, *nameserver = NULL;

	sender = dbus_message_get_sender(msg);

	dbus_message_iter_init(msg, &iter);
	dbus_message_iter_recurse(&iter, &array);

	while (dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter entry, value, list;
		const char *key;

		dbus_message_iter_recurse(&array, &entry);
		dbus_message_iter_get_basic(&entry, &key);

		dbus_message_iter_next(&entry);
		dbus_message_iter_recurse(&entry, &value);

		switch (dbus_message_iter_get_arg_type(&value)) {
		case DBUS_TYPE_STRING:
			if (g_str_equal(key, "Method") == TRUE) {
				dbus_message_iter_get_basic(&value, &method);
				if (strlen(method) == 0)
					method = NULL;
			} else if (g_str_equal(key, "URL") == TRUE) {
				dbus_message_iter_get_basic(&value, &url);
				if (strlen(url) == 0)
					url = NULL;
			} else if (g_str_equal(key, "Script") == TRUE) {
				dbus_message_iter_get_basic(&value, &script);
				if (strlen(script) == 0)
					script = NULL;
			} else if (g_str_equal(key, "Interface") == TRUE) {
				dbus_message_iter_get_basic(&value, &interface);
				if (strlen(interface) == 0)
					interface = NULL;
			}
			break;
		case DBUS_TYPE_ARRAY:
			dbus_message_iter_recurse(&value, &list);

			if (dbus_message_iter_get_arg_type(&list) ==
							DBUS_TYPE_INVALID)
				break;

			if (g_str_equal(key, "Servers") == TRUE) {
				dbus_message_iter_get_basic(&list, &server);
				if (strlen(server) == 0)
					server = NULL;
			} else if (g_str_equal(key, "Excludes") == TRUE) {
				dbus_message_iter_get_basic(&list, &exclude);
				if (strlen(exclude) == 0)
					exclude = NULL;
			} else if (g_str_equal(key, "Domains") == TRUE) {
				dbus_message_iter_get_basic(&list, &domainname);
				if (strlen(domainname) == 0)
					domainname = NULL;
			} else if (g_str_equal(key, "Nameservers") == TRUE) {
				dbus_message_iter_get_basic(&list, &nameserver);
				if (strlen(nameserver) == 0)
					nameserver = NULL;
			}
			break;
		}

		dbus_message_iter_next(&array);
	}

	DBG("sender %s method %s", sender, method);
	DBG("url %s script %p server %s exclude %s",
				url, script, server, exclude);
	DBG("interface %s domainname %s nameserver %s",
				interface, domainname, nameserver);

	if (method == NULL)
		return g_dbus_create_error(msg,
					PACRUNNER_ERROR_INTERFACE ".Failed",
					"No proxy method specified");

	config = create_config(conn, sender, method, url, script, server,
					interface, domainname, nameserver);
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
