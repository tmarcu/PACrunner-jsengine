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

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "pacrunner.h"

enum pacrunner_manual_exclude_appliance {
	PACRUNNER_MANUAL_EXCLUDE_POST = 0,
	PACRUNNER_MANUAL_EXCLUDE_PRE  = 1,
	PACRUNNER_MANUAL_EXCLUDE_ANY  = 2,
};

enum pacrunner_manual_protocol {
	PACRUNNER_PROTOCOL_ALL            = 0,
	PACRUNNER_PROTOCOL_HTTP           = 1,
	PACRUNNER_PROTOCOL_HTTPS          = 2,
	PACRUNNER_PROTOCOL_FTP            = 3,
	PACRUNNER_PROTOCOL_SOCKS          = 4,
	PACRUNNER_PROTOCOL_SOCKS4         = 5,
	PACRUNNER_PROTOCOL_SOCKS5         = 6,
	PACRUNNER_PROTOCOL_MAXIMUM_NUMBER = 7,
	PACRUNNER_PROTOCOL_UNKNOWN        = 8,
};

struct pacrunner_manual_exclude {
	enum pacrunner_manual_exclude_appliance appliance;
	int host_length;
	char *host;
};

static int parse_uri(char *uri,
			char **host,
			char **protocol,
			gboolean no_path,
			gboolean exclusion)
{
	int ret = PACRUNNER_MANUAL_EXCLUDE_POST;
	gboolean proto, post_confirmed, ipv6;
	char *scheme, *sep, *cur;
	long int port;
	int length;

	proto = post_confirmed = ipv6 = FALSE;
	port = -1;

	/**
	 * Make sure host and protocol, if given, are properly set.
	 */
	if (host != NULL)
		*host = NULL;

	if (protocol != NULL)
		*protocol = NULL;

	/**
	 * The parsing will actually process on a copy of given uri
	 */
	scheme = g_strdup(uri);
	if (scheme == NULL)
		goto error;

	cur = scheme;

	/**
	 * 1 - parsing protocol first
	 * Note: protocol scheme is here totally ignored
	 */
	sep = strstr(cur, "://");
	if (sep != NULL) {
		if (sep == cur)
			goto error;

		if (protocol != NULL) {
			*sep = '\0';

			*protocol = g_strdup(cur);
			if (*protocol == NULL)
				goto error;
		}

		cur = sep + 3;
		proto = TRUE;
	}

	/**
	 * 2 - detecting end of uri
	 * Note: in case of server/exclusion configuration,
	 * no path should be present
	 */
	sep = strchr(cur, '/');
	if (sep != NULL) {
		if (exclusion == TRUE || (*(sep + 1) != '\0' &&
							no_path == TRUE))
			goto error;

		*sep = '\0';
	}

	/**
	 * 3 - We skip <login:password> if present
	 * Note: exclusion rule cannot contain such authentication information
	 */
	sep = strchr(cur, '@');
	if (sep != NULL) {
		if (exclusion == TRUE)
			goto error;

		*sep = '\0';
		cur = sep + 1;
	}

	/**
	 * 4 - Are we in front of a possible IPv6 address?
	 * Note: ipv6 format is not checked!
	 */
	sep = strchr(cur, '[');
	if (sep != NULL) {
		char *bracket;

		bracket = strchr(cur, ']');
		if (bracket == NULL)
			goto error;

		cur = sep;
		sep = strchr(bracket, ':');

		ipv6 = TRUE;
	} else
		sep = strchr(cur, ':');

	/**
	 * 5 - Checking port validity if present
	 * Note: exclusion rule cannot embed port
	 */
	if (sep != NULL) {
		char *err = NULL;

		if (exclusion == TRUE)
			goto error;

		errno = 0;
		port = strtol(sep+1, &err, 10);
		if (*err != '\0' || port <= 0 || port > USHRT_MAX ||
					errno == ERANGE || errno == EINVAL)
			goto error;

		*sep = '\0';
	}

	/**
	 * 6 - We detect/trim '.'/'*' from start
	 * Note: This is valid only for exclusion URI since it defines
	 * its rule's appliance */
	for (sep = cur; *sep != '\0' && (*sep == '*' || *sep == '.'); sep++)
		*sep = '\0';

	if (sep != cur) {
		if (exclusion == FALSE)
			goto error;

		cur = sep;
		post_confirmed = TRUE;
	}

	/**
	 * 7 - Parsing host if present
	 */
	length = strlen(cur);
	if (length > 0) {
		const char *forbidden_chars;
		char **forbidden = NULL;

		/**
		 * We first detect/trim '.'/'*' from end
		 * Note: valid only for exclusion
		 */
		for (sep = cur + length - 1;
			*sep != '\0' && (*sep == '*' || *sep == '.'); sep--)
			*sep = '\0';

		if (sep - cur + 1 != length) {
			if (exclusion == FALSE)
				goto error;

			length = sep - cur + 1;

			ret = PACRUNNER_MANUAL_EXCLUDE_PRE;
			if (post_confirmed == TRUE)
				ret = PACRUNNER_MANUAL_EXCLUDE_ANY;
		}

		if ((length > 255) || (*cur == '-' || *sep == '-') ||
				((*cur == '\0') && (exclusion == FALSE ||
				(exclusion == TRUE && proto == FALSE))))
			goto error;

		/**
		 * We do not allow some characters. However we do not run
		 * a strict check if it's an IP address which is given
		 */
		if (ipv6 == TRUE)
			forbidden_chars = "%?!,;@\\'*|<>{}()+=$&~# \"";
		else
			forbidden_chars = "%?!,;@\\'*|<>{}[]()+=$&~# \"";

		forbidden = g_strsplit_set(cur, forbidden_chars, -1);
		if (forbidden != NULL) {
			length = g_strv_length(forbidden);
			g_strfreev(forbidden);

			if (length > 1)
				goto error;
		}

		if (host != NULL && *cur != '\0') {
			if (port > 0) {
				/**
				 * Instead of transcoding the port back
				 * to string we just get the host:port line
				 * from the original uri.
				 * */
				cur = uri + (cur - scheme);

				sep = strchr(cur, '/');
				if (sep != NULL)
					length = sep - cur;
				else
					length = strlen(cur);

				*host = g_strndup(cur, length);
			} else
				*host = g_strdup(cur);

			if (*host == NULL)
				goto error;
		}
	} else {
		if (exclusion == FALSE ||
				(exclusion == TRUE && proto == FALSE))
			goto error;
		else
			ret = PACRUNNER_MANUAL_EXCLUDE_ANY;
	}

	g_free(scheme);

	return ret;

error:
	if (protocol != NULL) {
		g_free(*protocol);
		*protocol = NULL;
	}

	g_free(scheme);

	return -EINVAL;
}

static enum pacrunner_manual_protocol get_protocol_from_string(const char *protocol)
{
	if (protocol == NULL)
		return PACRUNNER_PROTOCOL_ALL;

	if (g_strcmp0(protocol, "http") == 0)
		return PACRUNNER_PROTOCOL_HTTP;
	if (g_strcmp0(protocol, "https") == 0)
		return PACRUNNER_PROTOCOL_HTTPS;
	if (g_strcmp0(protocol, "ftp") == 0)
		return PACRUNNER_PROTOCOL_FTP;
	if (g_strcmp0(protocol, "socks") == 0)
		return PACRUNNER_PROTOCOL_SOCKS;
	if (g_strcmp0(protocol, "socks4") == 0)
		return PACRUNNER_PROTOCOL_SOCKS4;
	if (g_strcmp0(protocol, "socks5") == 0)
		return PACRUNNER_PROTOCOL_SOCKS5;

	return PACRUNNER_PROTOCOL_UNKNOWN;
}

static const char *protocol_to_prefix_string(enum pacrunner_manual_protocol proto)
{
	switch (proto) {
	case PACRUNNER_PROTOCOL_ALL:
	case PACRUNNER_PROTOCOL_HTTP:
	case PACRUNNER_PROTOCOL_HTTPS:
	case PACRUNNER_PROTOCOL_FTP:
		return "PROXY";
	case PACRUNNER_PROTOCOL_SOCKS4:
		return "SOCKS4";
	case PACRUNNER_PROTOCOL_SOCKS5:
		return "SOCKS5";
	case PACRUNNER_PROTOCOL_SOCKS:
		return "SOCKS";
	case PACRUNNER_PROTOCOL_MAXIMUM_NUMBER:
	case PACRUNNER_PROTOCOL_UNKNOWN:
		break;
	};

	return "";
}

static GList *append_proxy(GList *list,
			enum pacrunner_manual_protocol proto, char *host)
{
	char *proxy;

	proxy = g_strdup_printf("%s %s",
				protocol_to_prefix_string(proto), host);
	if (proxy == NULL)
		return list;

	return g_list_append(list, proxy);
}

GList **__pacrunner_manual_parse_servers(char **servers)
{
	char *host, *protocol;
	GList **result;
	char **uri;
	int proto;
	int ret;

	if (servers == NULL)
		return NULL;

	result = g_try_malloc0(PACRUNNER_PROTOCOL_MAXIMUM_NUMBER *
							sizeof(GList *));
	if (result == NULL)
		return NULL;

	for (uri = (char **)servers; *uri != NULL; uri++) {
		ret = parse_uri(*uri, &host, &protocol, TRUE, FALSE);

		if (ret < 0)
			continue;

		proto = get_protocol_from_string(protocol);
		if (proto == PACRUNNER_PROTOCOL_UNKNOWN)
			goto error;

		result[proto] = append_proxy(result[proto], proto, host);

		if (proto == PACRUNNER_PROTOCOL_SOCKS) {
			result[PACRUNNER_PROTOCOL_SOCKS4] = append_proxy(
					result[PACRUNNER_PROTOCOL_SOCKS4],
					PACRUNNER_PROTOCOL_SOCKS4, host);
			result[PACRUNNER_PROTOCOL_SOCKS5] = append_proxy(
					result[PACRUNNER_PROTOCOL_SOCKS5],
					PACRUNNER_PROTOCOL_SOCKS5, host);
		}

		g_free(protocol);
		g_free(host);
	}

	return result;

error:
	g_free(host);
	g_free(protocol);

	__pacrunner_manual_destroy_servers(result);

	return NULL;
}

void __pacrunner_manual_destroy_servers(GList **servers)
{
	int i;

	if (servers == NULL)
		return;

	for (i = 0; i < PACRUNNER_PROTOCOL_MAXIMUM_NUMBER; i++)
		g_list_free_full(servers[i], g_free);

	g_free(servers);
}

GList **__pacrunner_manual_parse_excludes(char **excludes)
{
	struct pacrunner_manual_exclude *exclude;
	char *host, *protocol;
	GList **result = NULL;
	char **uri;
	int proto;
	int ret;

	if (excludes == NULL)
		return NULL;

	result = g_try_malloc0(PACRUNNER_PROTOCOL_MAXIMUM_NUMBER *
							sizeof(GList *));
	if (result == NULL)
		return NULL;

	for (uri = (char **)excludes; *uri != NULL; uri++) {
		ret = parse_uri(*uri, &host, &protocol, TRUE, TRUE);

		if (ret < 0)
			continue;

		proto = get_protocol_from_string(protocol);
		if (proto == PACRUNNER_PROTOCOL_UNKNOWN)
			goto error;

		exclude = g_try_malloc0(sizeof(
					struct pacrunner_manual_exclude));
		if (exclude == NULL)
			goto error;

		exclude->appliance = ret;
		exclude->host = host;

		if (host != NULL)
			exclude->host_length = strlen(host);

		result[proto] = g_list_append(result[proto], exclude);

		g_free(protocol);
		protocol = NULL;
		host = NULL;
	}

	return result;

error:
	g_free(host);
	g_free(protocol);

	__pacrunner_manual_destroy_excludes(result);

	return NULL;
}

static void free_exclude(gpointer data)
{
	struct pacrunner_manual_exclude *exclude;

	exclude = (struct pacrunner_manual_exclude *) data;
	if (exclude == NULL)
		return;

	g_free(exclude->host);
	g_free(exclude);
}

void __pacrunner_manual_destroy_excludes(GList **excludes)
{
	int i;

	if (excludes == NULL)
		return;

	for (i = 0; i < PACRUNNER_PROTOCOL_MAXIMUM_NUMBER; i++)
		g_list_free_full(excludes[i], free_exclude);

	g_free(excludes);
}

static gboolean is_exclusion_matching(GList *excludes_list,
					const char *host)
{
	struct pacrunner_manual_exclude *exclusion;
	GList *excludes = NULL;
	char *cursor;

	for (excludes = excludes_list; excludes != NULL;
					excludes = excludes->next) {
		exclusion = (struct pacrunner_manual_exclude *) excludes->data;
		if (exclusion == NULL)
			continue;

		cursor = NULL;

		if (exclusion->host != NULL)
			cursor = strstr(host, exclusion->host);

		switch (exclusion->appliance) {
		case PACRUNNER_MANUAL_EXCLUDE_POST:
			if (cursor == NULL)
				break;

			if ((int)strlen(cursor) < exclusion->host_length)
				break;

			if (*(cursor + exclusion->host_length) == '\0')
				return TRUE;

			break;
		case PACRUNNER_MANUAL_EXCLUDE_PRE:
			if (cursor == host)
				return TRUE;

			break;
		case PACRUNNER_MANUAL_EXCLUDE_ANY:
			if (exclusion->host != NULL) {
				if (cursor != NULL)
					return TRUE;
				else
					break;
			}

			return TRUE;
		default:
			break;
		}
	}

	return FALSE;
}

static gboolean is_url_excluded(GList **excludes,
				const char *host,
				enum pacrunner_manual_protocol proto)
{
	if (excludes == NULL)
		return FALSE;

	if (excludes[PACRUNNER_PROTOCOL_ALL] != NULL)
		if (is_exclusion_matching(excludes[PACRUNNER_PROTOCOL_ALL],
								host) == TRUE)
			return TRUE;

	if (proto == PACRUNNER_PROTOCOL_UNKNOWN)
		return FALSE;

	if (excludes[proto] != NULL)
		if (is_exclusion_matching(excludes[proto], host) == TRUE)
			return TRUE;

	return FALSE;
}

static inline char *append_server(char *prev_result, const char *proxy)
{
	char *result;

	if (prev_result == NULL)
		return g_strdup(proxy);

	result = g_strjoin("; ", prev_result, proxy, NULL);
	if (result == NULL)
		return prev_result;

	g_free(prev_result);

	return result;
}

static inline char *append_servers_to_proxy_string(char *prev_result,
							GList *proxies)
{
	char *result = prev_result;
	GList *list, *prev;

	prev = NULL;
	for (list = proxies; list != NULL && list != prev;
						prev = list, list = list->next)
		result = append_server(result, (const char *) list->data);

	return result;
}

static char *generate_proxy_string(GList **servers,
				enum pacrunner_manual_protocol proto)
{
	enum pacrunner_manual_protocol i;
	char *result = NULL;

	/* if the protocol is known, we will prefer to set same
	 * protocol-based proxies first, if any... */
	if (proto >= PACRUNNER_PROTOCOL_HTTP &&
				proto < PACRUNNER_PROTOCOL_MAXIMUM_NUMBER) {
		if (servers[proto] != NULL)
			result = append_servers_to_proxy_string(result,
							servers[proto]);

		if (proto == PACRUNNER_PROTOCOL_SOCKS) {
			if (servers[PACRUNNER_PROTOCOL_SOCKS4] != NULL)
				result = append_servers_to_proxy_string(result,
					servers[PACRUNNER_PROTOCOL_SOCKS4]);
			if (servers[PACRUNNER_PROTOCOL_SOCKS5] != NULL)
				result = append_servers_to_proxy_string(result,
					servers[PACRUNNER_PROTOCOL_SOCKS5]);
		}
	}

	/* And/or we add the rest in the list */
	for (i = 0; i < PACRUNNER_PROTOCOL_MAXIMUM_NUMBER; i++) {
		if (i == proto || (proto == PACRUNNER_PROTOCOL_SOCKS &&
					(i == PACRUNNER_PROTOCOL_SOCKS4 ||
					 i == PACRUNNER_PROTOCOL_SOCKS5)))
			continue;

		if (servers[i] != NULL)
			result = append_servers_to_proxy_string(result,
								servers[i]);
	}

	return result;
}

char *__pacrunner_manual_execute(const char *url, const char *host,
				 GList **servers, GList **excludes)
{
	char *protocol = NULL;
	char *result = NULL;
	char *host_p = NULL;
	int proto;

	DBG("url %s host %s", url, host);

	if (servers == NULL || (url == NULL && host == NULL))
		return NULL;

	if (url == NULL)
		url = host;

	if (parse_uri((char *)url, &host_p, &protocol, FALSE, FALSE) < 0)
		goto direct;

	proto = get_protocol_from_string(protocol);

	if (is_url_excluded(excludes, host_p, proto) == TRUE)
		goto direct;

	result = generate_proxy_string(servers, proto);

direct:
	g_free(protocol);
	g_free(host_p);

	return result;
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
