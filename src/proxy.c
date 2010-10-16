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

#include <errno.h>

#include "pacrunner.h"

struct pacrunner_proxy {
	gint refcount;

	char *interface;
	enum pacrunner_proxy_method method;
	char *url;
	char *script;
	char *server;
};

struct pacrunner_proxy *pacrunner_proxy_create(const char *interface)
{
	struct pacrunner_proxy *proxy;

	DBG("interface %s", interface);

	proxy = g_try_new0(struct pacrunner_proxy, 1);
	if (proxy == NULL)
		return NULL;

	proxy->refcount = 1;

	proxy->interface = g_strdup(interface);
	proxy->method = PACRUNNER_PROXY_METHOD_UNKNOWN;

	DBG("proxy %p", proxy);

	return proxy;
}

struct pacrunner_proxy *pacrunner_proxy_ref(struct pacrunner_proxy *proxy)
{
	DBG("proxy %p", proxy);

	if (proxy == NULL)
		return NULL;

	g_atomic_int_inc(&proxy->refcount);

	return proxy;
}

static void reset_proxy(struct pacrunner_proxy *proxy)
{
	DBG("proxy %p", proxy);

	g_free(proxy->url);
	proxy->url = NULL;

	g_free(proxy->script);
	proxy->script = NULL;

	g_free(proxy->server);
	proxy->server = NULL;
}

void pacrunner_proxy_unref(struct pacrunner_proxy *proxy)
{
	DBG("proxy %p", proxy);

	if (proxy == NULL)
		return;

	if (g_atomic_int_dec_and_test(&proxy->refcount) == FALSE)
		return;

	reset_proxy(proxy);

	g_free(proxy->interface);
	g_free(proxy);
}

const char *pacrunner_proxy_get_interface(struct pacrunner_proxy *proxy)
{
	DBG("proxy %p", proxy);

	if (proxy == NULL)
		return NULL;

	return proxy->interface;
}

const char *pacrunner_proxy_get_script(struct pacrunner_proxy *proxy)
{
	DBG("proxy %p", proxy);

	if (proxy == NULL)
		return NULL;

	return proxy->script;
}

int pacrunner_proxy_set_method(struct pacrunner_proxy *proxy,
					enum pacrunner_proxy_method method)
{
	DBG("proxy %p method %d", proxy, method);

	if (proxy == NULL)
		return -EINVAL;

	if (proxy->method == method)
		return 0;

	proxy->method = method;

	reset_proxy(proxy);

	return 0;
}

int pacrunner_proxy_set_direct(struct pacrunner_proxy *proxy)
{
	DBG("proxy %p", proxy);

	if (proxy == NULL)
		return -EINVAL;

	return pacrunner_proxy_set_method(proxy, PACRUNNER_PROXY_METHOD_DIRECT);
}

static void download_callback(char *content, void *user_data)
{
	struct pacrunner_proxy *proxy = user_data;

	DBG("url %s content %p", proxy->url, content);

	if (content == NULL) {
		pacrunner_error("Failed to retrieve PAC script");
		goto done;
	}

	g_free(proxy->script);
	proxy->script = content;

	pacrunner_proxy_enable(proxy);

done:
	pacrunner_proxy_unref(proxy);
}

int pacrunner_proxy_set_auto(struct pacrunner_proxy *proxy, const char *url)
{
	int err;

	DBG("proxy %p url %s", proxy, url);

	if (proxy == NULL)
		return -EINVAL;

	err = pacrunner_proxy_set_method(proxy, PACRUNNER_PROXY_METHOD_AUTO);
	if (err < 0)
		return err;

	g_free(proxy->url);
	proxy->url = g_strdup(url);

	g_free(proxy->script);
	proxy->script = NULL;

	pacrunner_proxy_ref(proxy);

	err = __pacrunner_download_update(proxy->interface, proxy->url,
						download_callback, proxy);
	if (err < 0) {
		pacrunner_proxy_unref(proxy);
		return err;
	}

	return 0;
}

int pacrunner_proxy_set_script(struct pacrunner_proxy *proxy,
						const char *script)
{
	int err;

	DBG("proxy %p script %p", proxy, script);

	if (proxy == NULL)
		return -EINVAL;

	if (script == NULL)
		return -EINVAL;

	err = pacrunner_proxy_set_method(proxy, PACRUNNER_PROXY_METHOD_AUTO);
	if (err < 0)
		return err;

	g_free(proxy->url);
	proxy->url = NULL;

	g_free(proxy->script);
	proxy->script = g_strdup(script);

	pacrunner_proxy_enable(proxy);

	return 0;
}

int pacrunner_proxy_set_server(struct pacrunner_proxy *proxy,
						const char *server)
{
	int err;

	DBG("proxy %p server %s", proxy, server);

	if (proxy == NULL)
		return -EINVAL;

	if (server == NULL)
		return -EINVAL;

	err = pacrunner_proxy_set_method(proxy, PACRUNNER_PROXY_METHOD_MANUAL);
	if (err < 0)
		return err;

	g_free(proxy->server);
	proxy->server = g_strdup(server);

	pacrunner_proxy_enable(proxy);

	return 0;
}

static GList *proxy_list = NULL;
static GStaticMutex proxy_mutex = G_STATIC_MUTEX_INIT;

int pacrunner_proxy_enable(struct pacrunner_proxy *proxy)
{
	GList *list;

	DBG("proxy %p", proxy);

	if (proxy == NULL)
		return -EINVAL;

	list = g_list_find(proxy_list, proxy);
	if (list != NULL)
		return -EEXIST;

	proxy = pacrunner_proxy_ref(proxy);
	if (proxy == NULL)
		return -EIO;

	__pacrunner_mozjs_set_proxy(proxy);

	g_static_mutex_lock(&proxy_mutex);
	proxy_list = g_list_append(proxy_list, proxy);
	g_static_mutex_unlock(&proxy_mutex);

	return 0;
}

int pacrunner_proxy_disable(struct pacrunner_proxy *proxy)
{
	GList *list;

	DBG("proxy %p", proxy);

	if (proxy == NULL)
		return -EINVAL;

	list = g_list_find(proxy_list, proxy);
	if (list == NULL)
		return -ENXIO;

	g_static_mutex_lock(&proxy_mutex);
	proxy_list = g_list_remove_link(proxy_list, list);
	g_static_mutex_unlock(&proxy_mutex);

	__pacrunner_mozjs_set_proxy(NULL);

	pacrunner_proxy_unref(proxy);

	return 0;
}

const char *pacrunner_proxy_lookup(const char *url, const char *host)
{
	GList *list;
	struct pacrunner_proxy *selected_proxy = NULL;

	DBG("url %s host %s", url, host);

	if (proxy_list == NULL)
		return "DIRECT";

	g_static_mutex_lock(&proxy_mutex);

	for (list = g_list_first(proxy_list); list; list = g_list_next(list)) {
		struct pacrunner_proxy *proxy = list->data;

		if (proxy->method == PACRUNNER_PROXY_METHOD_MANUAL ||
				proxy->method == PACRUNNER_PROXY_METHOD_AUTO) {
			selected_proxy = proxy;
			break;
		} else if (proxy->method == PACRUNNER_PROXY_METHOD_DIRECT)
			selected_proxy = proxy;
	}

	g_static_mutex_unlock(&proxy_mutex);

	if (selected_proxy == NULL)
		return "DIRECT";

	switch (selected_proxy->method) {
	case PACRUNNER_PROXY_METHOD_UNKNOWN:
	case PACRUNNER_PROXY_METHOD_DIRECT:
		return "DIRECT";
	case PACRUNNER_PROXY_METHOD_MANUAL:
	case PACRUNNER_PROXY_METHOD_AUTO:
		break;
	}

	return __pacrunner_mozjs_execute(url, host);
}

int __pacrunner_proxy_init(void)
{
	DBG("");

	return 0;
}

void __pacrunner_proxy_cleanup(void)
{
	GList *list;

	DBG("");

	for (list = g_list_first(proxy_list); list; list = g_list_next(list)) {
		struct pacrunner_proxy *proxy = list->data;

		DBG("proxy %p", proxy);

		if (proxy != NULL)
			pacrunner_proxy_unref(proxy);
	}

	g_list_free(proxy_list);
	proxy_list = NULL;
}
