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
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <linux/if_arp.h>

#include <jsapi.h>

#include "javascript.h"

#include "pacrunner.h"

static char *current_interface = NULL;
static char *current_script = NULL;
static char *current_server = NULL;

static int getaddr(const char *node, char *host, size_t hostlen)
{
	struct sockaddr_in addr;
	struct ifreq ifr;
	int sk, err;

	sk = socket(PF_INET, SOCK_DGRAM, 0);
	if (sk < 0)
		return -EIO;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, node, sizeof(ifr.ifr_name));

	err = ioctl(sk, SIOCGIFADDR, &ifr);

	close(sk);

	if (err < 0)
		return -EIO;

	memcpy(&addr, &ifr.ifr_addr, sizeof(addr));
	snprintf(host, hostlen, "%s", inet_ntoa(addr.sin_addr));

	return 0;
}

static int resolve(const char *node, char *host, size_t hostlen)
{
	struct addrinfo *info;
	int err;

	if (getaddrinfo(node, NULL, NULL, &info) < 0)
		return -EIO;

	err = getnameinfo(info->ai_addr, info->ai_addrlen,
				host, hostlen, NULL, 0, NI_NUMERICHOST);

	freeaddrinfo(info);

	if (err < 0)
		return -EIO;

	return 0;
}

static JSBool myipaddress(JSContext *ctx, JSObject *obj, uintN argc,
						jsval *argv, jsval *rval)
{
	char address[NI_MAXHOST];
	char *result;

	DBG("");

	*rval = JSVAL_NULL;

	if (current_interface == NULL)
		return JS_TRUE;

	if (getaddr(current_interface, address, sizeof(address)) < 0)
		return JS_TRUE;

	DBG("address %s", address);

	result = JS_strdup(ctx, address);
	if (result == NULL)
		return JS_TRUE;

	*rval = STRING_TO_JSVAL(JS_NewString(ctx, result, strlen(result)));

	return JS_TRUE;
}

static JSBool dnsresolve(JSContext *ctx, JSObject *obj, uintN argc,
						jsval *argv, jsval *rval)
{
	char address[NI_MAXHOST];
	char *host = JS_GetStringBytes(JS_ValueToString(ctx, argv[0]));
	char *result;

	DBG("host %s", host);

	*rval = JSVAL_NULL;

	if (resolve(host, address, sizeof(address)) < 0)
		return JS_TRUE;

	DBG("address %s", address);

	result = JS_strdup(ctx, address);
	if (result == NULL)
		return JS_TRUE;

	*rval = STRING_TO_JSVAL(JS_NewString(ctx, result, strlen(result)));

	return JS_TRUE;
}

static JSClass jscls = {
	"global", JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static JSRuntime *jsrun;
static JSContext *jsctx = NULL;
static JSObject *jsobj = NULL;

static void create_object(void)
{
	jsval rval;

	jsctx = JS_NewContext(jsrun, 8 * 1024);

	jsobj = JS_NewObject(jsctx, &jscls, NULL, NULL);

	if (!JS_InitStandardClasses(jsctx, jsobj))
		pacrunner_error("Failed to init JS standard classes");

	JS_DefineFunction(jsctx, jsobj, "myIpAddress", myipaddress, 0, 0);
	JS_DefineFunction(jsctx, jsobj, "dnsResolve", dnsresolve, 1, 0);

	JS_EvaluateScript(jsctx, jsobj, JAVASCRIPT_ROUTINES,
				strlen(JAVASCRIPT_ROUTINES), NULL, 0, &rval);

	JS_EvaluateScript(jsctx, jsobj, current_script, strlen(current_script),
						"wpad.dat", 0, &rval);
}

static void destory_object(void)
{
	if (jsctx == NULL)
		return;

	JS_DestroyContext(jsctx);
	jsctx = NULL;
}

int __pacrunner_mozjs_set_server(const char *interface, const char *server)
{
	if (server == NULL)
		return -EINVAL;

	DBG("interface %s server %s", interface, server);

	destory_object();

	g_free(current_interface);
	current_interface = g_strdup(interface);

	g_free(current_script);
	current_script = NULL;

	g_free(current_server);
	current_server = g_strdup_printf("PROXY %s", server);

	return 0;
}

int __pacrunner_mozjs_set_script(const char *interface, const char *script)
{
	if (script == NULL)
		return -EINVAL;

	DBG("interface %s script %p", interface, script);

	destory_object();

	g_free(current_interface);
	current_interface = g_strdup(interface);

	g_free(current_script);
	current_script = g_strdup(script);

	g_free(current_server);
	current_server = NULL;

	create_object();

	return 0;
}

void __pacrunner_mozjs_clear(void)
{
	DBG("");

	destory_object();

	g_free(current_interface);
	current_interface = NULL;

	g_free(current_script);
	current_script = NULL;

	g_free(current_server);
	current_server = NULL;
}

const char *__pacrunner_mozjs_execute(const char *url, const char *host)
{
	JSBool result;
	jsval rval, args[2];
	char *answer, *tmpurl, *tmphost;

	DBG("url %s host %s", url, host);

	if (current_server != NULL)
		return current_server;

	if (current_script == NULL)
		return "DIRECT";

	tmpurl = JS_strdup(jsctx, url);
	tmphost = JS_strdup(jsctx, host);

	if (tmpurl == NULL || tmphost == NULL) {
		JS_free(jsctx, tmphost);
		JS_free(jsctx, tmpurl);
		return NULL;
	}

	JS_BeginRequest(jsctx);

	args[0] = STRING_TO_JSVAL(JS_NewString(jsctx,
					tmpurl, strlen(tmpurl)));
	args[1] = STRING_TO_JSVAL(JS_NewString(jsctx,
					tmphost, strlen(tmphost)));

	result = JS_CallFunctionName(jsctx, jsobj, "FindProxyForURL",
							2, args, &rval);

	JS_EndRequest(jsctx);

	JS_MaybeGC(jsctx);

	if (result) {
		answer = JS_GetStringBytes(JS_ValueToString(jsctx, rval));
		return answer;
	}

	return NULL;
}

int __pacrunner_mozjs_init(void)
{
	DBG("");

	jsrun = JS_NewRuntime(8 * 1024 * 1024);

	return 0;
}

void __pacrunner_mozjs_cleanup(void)
{
	DBG("");

	__pacrunner_mozjs_clear();

	JS_DestroyRuntime(jsrun);
}
