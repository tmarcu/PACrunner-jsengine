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

struct pacrunner_proxy *current_proxy = NULL;

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
	const char *interface;
	char address[NI_MAXHOST];
	char *result;

	DBG("");

	*rval = JSVAL_NULL;

	if (current_proxy == NULL)
		return JS_TRUE;

	interface = pacrunner_proxy_get_interface(current_proxy);
	if (interface == NULL)
		return JS_TRUE;

	if (getaddr(interface, address, sizeof(address)) < 0)
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
	const char *script;
	jsval rval;

	if (current_proxy == NULL)
		return;

	script = pacrunner_proxy_get_script(current_proxy);
	if (script == NULL)
		return;

	jsctx = JS_NewContext(jsrun, 8 * 1024);

	jsobj = JS_NewObject(jsctx, &jscls, NULL, NULL);

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

int __pacrunner_mozjs_set_proxy(struct pacrunner_proxy *proxy)
{
	DBG("proxy %p", proxy);

	if (current_proxy != NULL)
		destroy_object();

	current_proxy = proxy;

	if (current_proxy != NULL)
		create_object();

	return 0;
}

const char *__pacrunner_mozjs_execute(const char *url, const char *host)
{
	JSBool result;
	jsval rval, args[2];
	char *answer, *tmpurl, *tmphost;

	DBG("url %s host %s", url, host);

	if (jsctx == NULL)
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

	__pacrunner_mozjs_set_proxy(NULL);

	JS_DestroyRuntime(jsrun);
}
