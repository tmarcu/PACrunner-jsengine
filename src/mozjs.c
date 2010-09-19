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

#include <jsapi.h>

#include "pacrunner.h"

#define SAMPLE_PACFILE \
	"function FindProxyForURL(url, host) {\n" \
	"  var my_ip = myIpAddress();\n" \
	"  var resolved_ip = dnsResolve(host);\n" \
	"  return(\"DIRECT\");\n" \
	"}"

static JSBool myipaddress(JSContext *ctx, JSObject *obj, uintN argc,
						jsval *argv, jsval *rval)
{
	char *addr = JS_strdup(ctx, "127.0.0.1");

	DBG("");

	*rval = STRING_TO_JSVAL(JS_NewString(ctx, addr, strlen(addr)));

	return TRUE;
}

static JSBool dnsresolve(JSContext *ctx, JSObject *obj, uintN argc,
						jsval *argv, jsval *rval)
{
	char *host = JS_strdup(ctx,
			JS_GetStringBytes(JS_ValueToString(ctx, argv[0])));

	DBG("host %s", host);

	*rval = JSVAL_NULL;

	JS_free(ctx, host);

	return TRUE;
}

static JSClass jscls = {
	"global", JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static JSRuntime *jsrun;
static JSContext *jsctx;

void __pacrunner_mozjs_execute(const char *url, const char *host)
{
	JSObject *jsobj;
	JSBool result;
	jsval rval, args[2];
	char *answer, *tmpurl, *tmphost;

	DBG("url %s host %s", url, host);

	jsobj = JS_NewObject(jsctx, &jscls, NULL, NULL);

	if (!JS_InitStandardClasses(jsctx, jsobj))
		pacrunner_error("Failed to init JS standard classes");

	JS_DefineFunction(jsctx, jsobj, "myIpAddress", myipaddress, 0, 0);
	JS_DefineFunction(jsctx, jsobj, "dnsResolve", dnsresolve, 1, 0);

	JS_EvaluateScript(jsctx, jsobj, SAMPLE_PACFILE, strlen(SAMPLE_PACFILE),
							"wpad.dat", 0, &rval);

	tmpurl = JS_strdup(jsctx, url);
	tmphost = JS_strdup(jsctx, host);

	args[0] = STRING_TO_JSVAL(JS_NewString(jsctx,
					tmpurl, strlen(tmpurl)));
	args[1] = STRING_TO_JSVAL(JS_NewString(jsctx,
					tmphost, strlen(tmphost)));

	result = JS_CallFunctionName(jsctx, jsobj, "FindProxyForURL",
							2, args, &rval);

	if (result) {
		answer = JS_GetStringBytes(JS_ValueToString(jsctx, rval));
		pacrunner_info("PAC result = %s", answer);
	}

	JS_free(jsctx, tmphost);
	JS_free(jsctx, tmpurl);
}

int __pacrunner_mozjs_init(void)
{
	DBG("");

	jsrun = JS_NewRuntime(1024 * 1024);
	jsctx = JS_NewContext(jsrun, 1024 * 1024);

	return 0;
}

void __pacrunner_mozjs_cleanup(void)
{
	DBG("");

	//JS_DestroyContext(jsctx);
	JS_DestroyRuntime(jsrun);
}
