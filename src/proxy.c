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

#include <errno.h>
#include <pthread.h>

#include "pacrunner.h"

struct pacrunner_proxy {
	gint refcount;

	char *interface;
	enum pacrunner_proxy_method method;
	char *url;
	char *script;
	GList **servers;
	GList **excludes;
};

static GList *proxy_list = NULL;
static pthread_mutex_t proxy_mutex;
static pthread_cond_t proxy_cond;
static int timeout_source = 0;
static gint proxy_updating = -1; /* -1 for 'never set', with timeout */

struct pacrunner_proxy *pacrunner_proxy_create(const char *interface)
{
	struct pacrunner_proxy *proxy;

	DBG("interface %s", interface);

	proxy = g_try_new0(struct pacrunner_proxy, 1);
	if (!proxy)
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

	if (!proxy)
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

	__pacrunner_manual_destroy_servers(proxy->servers);
	proxy->servers = NULL;

	__pacrunner_manual_destroy_excludes(proxy->excludes);
	proxy->excludes = NULL;
}

void pacrunner_proxy_unref(struct pacrunner_proxy *proxy)
{
	DBG("proxy %p", proxy);

	if (!proxy)
		return;

	if (!g_atomic_int_dec_and_test(&proxy->refcount))
		return;

	reset_proxy(proxy);

	g_free(proxy->interface);
	g_free(proxy);
}

const char *pacrunner_proxy_get_interface(struct pacrunner_proxy *proxy)
{
	DBG("proxy %p", proxy);

	if (!proxy)
		return NULL;

	return proxy->interface;
}

const char *pacrunner_proxy_get_script(struct pacrunner_proxy *proxy)
{
	DBG("proxy %p", proxy);

	if (!proxy)
		return NULL;

	return proxy->script;
}

static int set_method(struct pacrunner_proxy *proxy,
					enum pacrunner_proxy_method method)
{
	DBG("proxy %p method %d", proxy, method);

	if (!proxy)
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

	if (!proxy)
		return -EINVAL;

	pthread_mutex_lock(&proxy_mutex);
	if (proxy_updating == -1) {
		proxy_updating = 0;
		pthread_cond_broadcast(&proxy_cond);
	}
	pthread_mutex_unlock(&proxy_mutex);

	return set_method(proxy, PACRUNNER_PROXY_METHOD_DIRECT);
}

int pacrunner_proxy_set_manual(struct pacrunner_proxy *proxy,
					char **servers, char **excludes)
{
	int err;

	DBG("proxy %p servers %p excludes %p", proxy, servers, excludes);

	if (!proxy)
		return -EINVAL;

	if (!servers)
		return -EINVAL;

	err = set_method(proxy, PACRUNNER_PROXY_METHOD_MANUAL);
	if (err < 0)
		return err;

	proxy->servers = __pacrunner_manual_parse_servers(servers);
	if (!proxy->servers)
		return -EINVAL;

	proxy->excludes = __pacrunner_manual_parse_excludes(excludes);

	pacrunner_proxy_enable(proxy);

	return 0;
}

static void download_callback(char *content, void *user_data)
{
	struct pacrunner_proxy *proxy = user_data;

	DBG("url %s content %p", proxy->url, content);

	if (!content) {
		pacrunner_error("Failed to retrieve PAC script");
		goto done;
	}

	g_free(proxy->script);
	proxy->script = content;

	pacrunner_proxy_enable(proxy);

done:
	pthread_mutex_lock(&proxy_mutex);
	proxy_updating--;
	pthread_cond_broadcast(&proxy_cond);
	pthread_mutex_unlock(&proxy_mutex);
	pacrunner_proxy_unref(proxy);
}

int pacrunner_proxy_set_auto(struct pacrunner_proxy *proxy,
					const char *url, const char *script)
{
	int err;

	DBG("proxy %p url %s script %p", proxy, url, script);

	if (!proxy)
		return -EINVAL;

	err = set_method(proxy, PACRUNNER_PROXY_METHOD_AUTO);
	if (err < 0)
		return err;

	g_free(proxy->url);
	proxy->url = g_strdup(url);

	if (!proxy->url) {
		g_free(proxy->script);
		proxy->script = g_strdup(script);
	} else {
		g_free(proxy->script);
		proxy->script = NULL;
	}

	if (proxy->script) {
		pacrunner_proxy_enable(proxy);
		return 0;
	}

	pacrunner_proxy_ref(proxy);

	pthread_mutex_lock(&proxy_mutex);
	err = __pacrunner_download_update(proxy->interface, proxy->url,
						download_callback, proxy);
	if (err < 0) {
		pacrunner_proxy_unref(proxy);
		if (proxy_updating == -1) {
			proxy_updating = 0;
			pthread_cond_broadcast(&proxy_cond);
		}
		pthread_mutex_unlock(&proxy_mutex);
		return err;
	}
	if (proxy_updating == -1)
		proxy_updating = 1;
	else
		proxy_updating++;
	pthread_mutex_unlock(&proxy_mutex);

	return 0;
}

int pacrunner_proxy_enable(struct pacrunner_proxy *proxy)
{
	GList *list;

	DBG("proxy %p", proxy);

	if (!proxy)
		return -EINVAL;

	list = g_list_find(proxy_list, proxy);
	if (list)
		return -EEXIST;

	proxy = pacrunner_proxy_ref(proxy);
	if (!proxy)
		return -EIO;

	__pacrunner_js_set_proxy(proxy);

	pthread_mutex_lock(&proxy_mutex);
	if (proxy_updating == -1) {
		proxy_updating = 0;
		pthread_cond_broadcast(&proxy_cond);
	}
	proxy_list = g_list_append(proxy_list, proxy);
	pthread_mutex_unlock(&proxy_mutex);

	return 0;
}

int pacrunner_proxy_disable(struct pacrunner_proxy *proxy)
{
	GList *list;

	DBG("proxy %p", proxy);

	if (!proxy)
		return -EINVAL;

	list = g_list_find(proxy_list, proxy);
	if (!list)
		return -ENXIO;

	pthread_mutex_lock(&proxy_mutex);
	proxy_list = g_list_remove_link(proxy_list, list);
	pthread_mutex_unlock(&proxy_mutex);

	__pacrunner_js_set_proxy(NULL);

	pacrunner_proxy_unref(proxy);

	return 0;
}

char *pacrunner_proxy_lookup(const char *url, const char *host)
{
	GList *list;
	struct pacrunner_proxy *selected_proxy = NULL;

	DBG("url %s host %s", url, host);

	pthread_mutex_lock(&proxy_mutex);
	while (proxy_updating)
		pthread_cond_wait(&proxy_cond, &proxy_mutex);

	if (!proxy_list) {
		pthread_mutex_unlock(&proxy_mutex);
		return NULL;
	}

	for (list = g_list_first(proxy_list); list; list = g_list_next(list)) {
		struct pacrunner_proxy *proxy = list->data;

		if (proxy->method == PACRUNNER_PROXY_METHOD_MANUAL ||
				proxy->method == PACRUNNER_PROXY_METHOD_AUTO) {
			selected_proxy = proxy;
			break;
		} else if (proxy->method == PACRUNNER_PROXY_METHOD_DIRECT)
			selected_proxy = proxy;
	}

	pthread_mutex_unlock(&proxy_mutex);

	if (!selected_proxy)
		return NULL;

	switch (selected_proxy->method) {
	case PACRUNNER_PROXY_METHOD_UNKNOWN:
	case PACRUNNER_PROXY_METHOD_DIRECT:
		break;
	case PACRUNNER_PROXY_METHOD_MANUAL:
		return __pacrunner_manual_execute(url, host,
						selected_proxy->servers,
						selected_proxy->excludes);
	case PACRUNNER_PROXY_METHOD_AUTO:
		return __pacrunner_js_execute(url, host);
	}

	return NULL;
}

static gboolean proxy_config_timeout(gpointer user_data)
{
	DBG("");

	/* If ConnMan/NetworkManager/whatever hasn't given us a config within
	   a reasonable length of time, start responding 'DIRECT'. */
	if (proxy_updating == -1) {
		proxy_updating = 0;
		pthread_cond_broadcast(&proxy_cond);
	}
	return FALSE;
}

int __pacrunner_proxy_init(void)
{
	DBG("");

	pthread_mutex_init(&proxy_mutex, NULL);
	pthread_cond_init(&proxy_cond, NULL);

	timeout_source = g_timeout_add_seconds(5, proxy_config_timeout, NULL);

	return 0;
}

void __pacrunner_proxy_cleanup(void)
{
	GList *list;

	DBG("");

	for (list = g_list_first(proxy_list); list; list = g_list_next(list)) {
		struct pacrunner_proxy *proxy = list->data;

		DBG("proxy %p", proxy);

		if (proxy)
			pacrunner_proxy_unref(proxy);
	}

	pthread_mutex_destroy(&proxy_mutex);
	pthread_cond_destroy(&proxy_cond);

	g_list_free(proxy_list);
	proxy_list = NULL;

	if (timeout_source) {
		g_source_remove(timeout_source);
		timeout_source = 0;
	}
}
