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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include <jsapi.h>

#include "javascript.h"

#include "pacrunner.h"

#define DEFAULT_PACFILE \
	"function FindProxyForURL(url, host) {\n" \
	"  return \"DIRECT\";\n" \
	"}"

static char *current_pacfile = NULL;

int __pacrunner_mozjs_load(const char *url)
{
	const char *filename;
	struct stat st;
	int fd;

	DBG("url %s", url);

	g_free(current_pacfile);
	current_pacfile = NULL;

	if (g_str_has_prefix(url, "file://") == FALSE)
		return -EINVAL;

	filename = url + 7;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return -EIO;

	if (fstat(fd, &st) < 0) {
		close(fd);
		return -EIO;
	}

	current_pacfile = g_try_malloc(st.st_size);
	if (current_pacfile == NULL) {
		close(fd);
		return -ENOMEM;
	}

	if (read(fd, current_pacfile, st.st_size) < 0) {
		close(fd);
		g_free(current_pacfile);
		current_pacfile = NULL;
		return -EIO;
	}

	close(fd);

	DBG("%s loaded", filename);

	return 0;
}

void __pacrunner_mozjs_clear(void)
{
	DBG("");

	g_free(current_pacfile);
	current_pacfile = NULL;
}

static JSBool myipaddress(JSContext *ctx, JSObject *obj, uintN argc,
						jsval *argv, jsval *rval)
{
	char *addr = JS_strdup(ctx, "192.168.0.1");

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

const char *__pacrunner_mozjs_execute(const char *url, const char *host)
{
	JSObject *jsobj;
	JSBool result;
	jsval rval, args[2];
	char *answer, *tmpurl, *tmphost;
	const char *pacfile;

	DBG("url %s host %s", url, host);

	jsobj = JS_NewObject(jsctx, &jscls, NULL, NULL);

	if (!JS_InitStandardClasses(jsctx, jsobj))
		pacrunner_error("Failed to init JS standard classes");

	JS_DefineFunction(jsctx, jsobj, "myIpAddress", myipaddress, 0, 0);
	JS_DefineFunction(jsctx, jsobj, "dnsResolve", dnsresolve, 1, 0);

	JS_EvaluateScript(jsctx, jsobj, JAVASCRIPT_ROUTINES,
			strlen(JAVASCRIPT_ROUTINES), "javascript.js", 0, &rval);

	pacfile = current_pacfile ? current_pacfile : DEFAULT_PACFILE;

	JS_EvaluateScript(jsctx, jsobj, pacfile, strlen(pacfile),
						"wpad.dat", 0, &rval);

	tmpurl = JS_strdup(jsctx, url);
	tmphost = JS_strdup(jsctx, host);

	args[0] = STRING_TO_JSVAL(JS_NewString(jsctx,
					tmpurl, strlen(tmpurl)));
	args[1] = STRING_TO_JSVAL(JS_NewString(jsctx,
					tmphost, strlen(tmphost)));

	result = JS_CallFunctionName(jsctx, jsobj, "FindProxyForURL",
							2, args, &rval);

	JS_free(jsctx, tmphost);
	JS_free(jsctx, tmpurl);

	if (result) {
		answer = JS_GetStringBytes(JS_ValueToString(jsctx, rval));
		return answer;
	}

	return NULL;
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

	g_free(current_pacfile);
	current_pacfile = NULL;

	//JS_DestroyContext(jsctx);
	JS_DestroyRuntime(jsrun);
}
