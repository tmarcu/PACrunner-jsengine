AM_MAKEFLAGS = --no-print-directory

if DATAFILES
dbusconfdir = @DBUS_CONFDIR@

dbusconf_DATA = src/pacrunner.conf

dbusdatadir = @DBUS_DATADIR@

dbusdata_DATA = src/pacrunner.service
endif

gdbus_sources = gdbus/gdbus.h gdbus/mainloop.c gdbus/watch.c \
				gdbus/object.c gdbus/client.c gdbus/polkit.c

builtin_modules =
builtin_sources =
builtin_cflags =
builtin_libadd =

if CURL
builtin_modules += curl
builtin_sources += plugins/curl.c
builtin_cflags += @CURL_CFLAGS@
builtin_libadd += @CURL_LIBS@
endif

if MOZJS
builtin_modules += mozjs
builtin_sources += plugins/mozjs.c
builtin_cflags += @MOZJS_CFLAGS@
builtin_libadd += @MOZJS_LIBS@
endif

if V8
builtin_modules += v8
builtin_sources += plugins/v8.cc
builtin_libadd += @V8_LIBS@
endif

noinst_PROGRAMS =

BUILT_SOURCES = jsengine/parser.h

AM_YFLAGS = -d

sbin_PROGRAMS = src/pacrunner

src_pacrunner_SOURCES = $(gdbus_sources) $(builtin_sources) \
			src/main.c src/pacrunner.h \
			src/log.h src/log.c src/plugin.h src/plugin.c \
			src/download.h src/download.c \
			src/client.c src/manager.c src/proxy.c \
			src/manual.c src/js.h src/js.c src/javascript.h

if JSENGINE
builtin_modules += jsengine
builtin_sources += plugins/jsengine.c
builtin_sources += jsengine/parser.y jsengine/lexer.l jsengine/tree.c \
			jsengine/symbols.c jsengine/spider.c \
			jsengine/math_functions.c jsengine/string_methods.c \
			jsengine/array_methods.c jsengine/reg_expr_methods.c \
			jsengine/boolean_methods.c \
			jsengine/javascript_functions.c
builtin_libadd += @JSENGINE_LIBS@ -lfl

jsengine_jsengine_LDADD = -lfl -lm
jsenginedir = jsengine
jsengine_jsengine_SOURCES = jsengine/parser.y jsengine/lexer.l jsengine/tree.c \
			jsengine/symbols.c jsengine/spider.c \
			jsengine/math_functions.c jsengine/string_methods.c \
			jsengine/array_methods.c jsengine/reg_expr_methods.c \
			jsengine/boolean_methods.c \
			jsengine/javascript_functions.c jsengine/main.c
noinst_PROGRAMS += jsengine/jsengine
endif

src_pacrunner_LDADD = $(builtin_libadd) @GLIB_LIBS@ @PTHREAD_LIBS@ \
							@DBUS_LIBS@ -ldl

src_pacrunner_LDFLAGS = -Wl,--export-dynamic

nodist_src_pacrunner_SOURCES = src/builtin.h

plugindir = $(libdir)/pacrunner/plugins

src/plugin.$(OBJEXT): src/builtin.h

src/builtin.h: src/genbuiltin $(builtin_sources)
	$(AM_V_GEN)$(srcdir)/src/genbuiltin $(builtin_modules) > $@

if LIBPROXY
include_HEADERS = libproxy/proxy.h

lib_LTLIBRARIES = libproxy/libproxy.la

libproxy_libproxy_la_SOURCES = libproxy/proxy.h libproxy/proxy.c

libproxy_libproxy_la_LIBADD = @DBUS_LIBS@

libproxy_libproxy_la_LDFLAGS = -version-info 1:0:0

if DATAFILES
pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = libproxy/libproxy-1.0.pc
endif

bin_PROGRAMS = tools/proxy tools/manual-proxy-test

tools_proxy_SOURCES = tools/proxy.c

tools_proxy_LDADD = libproxy/libproxy.la

tools_manual_proxy_test_LDADD = @GLIB_LIBS@

tools_manual_proxy_test_SOURCES = tools/manual-proxy-test.c

endif

if UNIT

noinst_PROGRAMS = unit/test-pacrunner

unit_test_pacrunner_SOURCES = unit/test-pacrunner.c src/pacrunner.h \
			src/proxy.c src/manual.c src/download.c \
			src/js.c src/log.c src/plugin.c

unit_test_pacrunner_LDADD = @GLIB_LIBS@ -ldl

if CURL

unit_test_pacrunner_SOURCES += plugins/curl.c

unit_test_pacrunner_LDADD += @CURL_LIBS@

endif

if MOZJS
unit_test_pacrunner_SOURCES += plugins/mozjs.c

unit_test_pacrunner_LDADD += @MOZJS_LIBS@ @PTHREAD_LIBS@

noinst_PROGRAMS += unit/test-mozjs

unit_test_mozjs_SOURCES = unit/test-mozjs.c src/pacrunner.h \
			src/proxy.c src/manual.c src/download.c \
				src/js.c plugins/mozjs.c

unit_test_mozjs_LDADD = @MOZJS_LIBS@ @GLIB_LIBS@ @PTHREAD_LIBS@
endif

if V8

unit_test_pacrunner_SOURCES += plugins/v8.cc

unit_test_pacrunner_LDADD += @V8_LIBS@

endif

endif #UNIT

AM_CFLAGS = @GLIB_CFLAGS@ @PTHREAD_LIBS@ @DBUS_CFLAGS@ \
				$(builtin_cflags) \
				-DPACRUNNER_PLUGIN_BUILTIN \
				-DPLUGINDIR=\""$(plugindir)"\"

AM_CXXFLAGS = $(AM_CFLAGS)

AM_CPPFLAGS = -I$(builddir)/src -I$(srcdir)/src -I$(srcdir)/gdbus

test_scripts = test/find-proxy-for-url test/create-proxy-config

if TEST
testdir = $(pkglibdir)/test
test_SCRIPTS = $(test_scripts)
endif

CLEANFILES = src/builtin.h jsengine/parser.c jsengine/parser.h \
		jsengine/lexer.c

EXTRA_DIST = src/genbuiltin src/pacrunner.conf $(test_scripts) \
			doc/architecture.txt doc/libproxy.txt \
			doc/manager-api.txt doc/client-api.txt \
			doc/manual-configuration.txt

DISTCHECK_CONFIGURE_FLAGS = --enable-libproxy --disable-datafiles \
				--enable-mozjs --enable-curl

MAINTAINERCLEANFILES = Makefile.in \
	aclocal.m4 configure config.h.in config.sub config.guess \
	ltmain.sh depcomp compile missing install-sh mkinstalldirs
