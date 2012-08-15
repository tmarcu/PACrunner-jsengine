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
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <linux/if_arp.h>

#pragma GCC diagnostic ignored "-Wredundant-decls"
#include <jsapi.h>
#pragma GCC diagnostic error "-Wredundant-decls"

#include "javascript.h"

#include "pacrunner.h"
#include "js.h"

static GStaticMutex mozjs_mutex = G_STATIC_MUTEX_INIT;

static struct pacrunner_proxy *current_proxy = NULL;

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

static JSBool myipaddress(JSContext *ctx, uintN argc, jsval *vp)
{
	const char *interface;
	char address[NI_MAXHOST];

	DBG("");

	JS_SET_RVAL(ctx, vp, JSVAL_NULL);

	if (current_proxy == NULL)
		return JS_TRUE;

	interface = pacrunner_proxy_get_interface(current_proxy);
	if (interface == NULL)
		return JS_TRUE;

	if (getaddr(interface, address, sizeof(address)) < 0)
		return JS_TRUE;

	DBG("address %s", address);

	JS_SET_RVAL(ctx, vp, STRING_TO_JSVAL(JS_NewStringCopyZ(ctx, address)));

	return JS_TRUE;
}

static JSBool dnsresolve(JSContext *ctx, uintN argc, jsval *vp)
{
	char address[NI_MAXHOST];
	jsval *argv = JS_ARGV(ctx, vp);
	char *host = JS_EncodeString(ctx, JS_ValueToString(ctx, argv[0]));

	DBG("host %s", host);

	JS_SET_RVAL(ctx, vp, JSVAL_NULL);

	if (resolve(host, address, sizeof(address)) < 0)
		goto out;

	DBG("address %s", address);

	JS_SET_RVAL(ctx, vp, STRING_TO_JSVAL(JS_NewStringCopyZ(ctx, address)));

 out:
	JS_free(ctx, host);
	return JS_TRUE;

}

static JSClass jscls = {
	"global", JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_StrictPropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static JSRuntime *jsrun;
static JSContext *jsctx = NULL;
static JSObject *jsobj = NULL;

static void create_object(void)
{
	const char *script;
	jsval rval;

	if (current_proxy == NULL)
		return;

	script = pacrunner_proxy_get_script(current_proxy);
	if (script == NULL)
		return;

	jsctx = JS_NewContext(jsrun, 8 * 1024);

#if JS_VERSION >= 185
	jsobj = JS_NewCompartmentAndGlobalObject(jsctx, &jscls, NULL);
#else
	jsobj = JS_NewObject(jsctx, &jscls, NULL, NULL);
#endif

	if (!JS_InitStandardClasses(jsctx, jsobj))
		pacrunner_error("Failed to init JS standard classes");

	JS_DefineFunction(jsctx, jsobj, "myIpAddress", myipaddress, 0, 0);
	JS_DefineFunction(jsctx, jsobj, "dnsResolve", dnsresolve, 1, 0);

	JS_EvaluateScript(jsctx, jsobj, JAVASCRIPT_ROUTINES,
				strlen(JAVASCRIPT_ROUTINES), NULL, 0, &rval);

	JS_EvaluateScript(jsctx, jsobj, script, strlen(script),
						"wpad.dat", 0, &rval);
}

static void destroy_object(void)
{
	if (jsctx == NULL)
		return;

	JS_DestroyContext(jsctx);
	jsctx = NULL;

	jsobj = NULL;
}

static int mozjs_set_proxy(struct pacrunner_proxy *proxy)
{
	DBG("proxy %p", proxy);

	if (current_proxy != NULL)
		destroy_object();

	current_proxy = proxy;

	if (current_proxy != NULL)
		create_object();

	return 0;
}

static char * mozjs_execute(const char *url, const char *host)
{
	JSBool result;
	jsval rval, args[2];
	char *answer, *g_answer;

	DBG("url %s host %s", url, host);

	if (jsctx == NULL)
		return NULL;

	g_static_mutex_lock(&mozjs_mutex);

	JS_BeginRequest(jsctx);

	args[0] = STRING_TO_JSVAL(JS_NewStringCopyZ(jsctx, url));
	args[1] = STRING_TO_JSVAL(JS_NewStringCopyZ(jsctx, host));

	result = JS_CallFunctionName(jsctx, jsobj, "FindProxyForURL",
							2, args, &rval);

	JS_EndRequest(jsctx);

	JS_MaybeGC(jsctx);

	g_static_mutex_unlock(&mozjs_mutex);

	if (result) {
		answer = JS_EncodeString(jsctx, JS_ValueToString(jsctx, rval));
		g_answer = g_strdup(answer);
		JS_free(jsctx, answer);
		return g_answer;
	}

	return NULL;
}

static struct pacrunner_js_driver mozjs_driver = {
	.name		= "mozjs",
	.priority	= PACRUNNER_JS_PRIORITY_DEFAULT,
	.set_proxy	= mozjs_set_proxy,
	.execute	= mozjs_execute,
};

static int mozjs_init(void)
{
	DBG("");

	jsrun = JS_NewRuntime(8 * 1024 * 1024);

	return pacrunner_js_driver_register(&mozjs_driver);
}

static void mozjs_exit(void)
{
	DBG("");

	pacrunner_js_driver_unregister(&mozjs_driver);

	mozjs_set_proxy(NULL);

	JS_DestroyRuntime(jsrun);
}

PACRUNNER_PLUGIN_DEFINE(mozjs, mozjs_init, mozjs_exit)
