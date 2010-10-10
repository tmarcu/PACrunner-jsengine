
AM_MAKEFLAGS = --no-print-directory

if DATAFILES
dbusconfdir = @DBUS_CONFDIR@

dbusconf_DATA = src/pacrunner.conf

dbusdatadir = @DBUS_DATADIR@

dbusdata_DATA = src/pacrunner.service
endif

gdbus_sources = gdbus/gdbus.h gdbus/mainloop.c gdbus/watch.c \
					gdbus/object.c gdbus/polkit.c

sbin_PROGRAMS = src/pacrunner

src_pacrunner_SOURCES = $(gdbus_sources) src/pacrunner.h \
					src/main.c src/log.c \
					src/manager.c src/client.c \
					src/mozjs.c src/javascript.h

src_pacrunner_LDADD = @MOZJS_LIBS@ @GLIB_LIBS@ @DBUS_LIBS@ @CAPNG_LIBS@

if LIBPROXY
include_HEADERS = libproxy/proxy.h

lib_LTLIBRARIES = libproxy/libproxy.la

libproxy_libproxy_la_SOURCES = libproxy/proxy.h libproxy/proxy.c

libproxy_libproxy_la_LIBADD = @DBUS_LIBS@

libproxy_libproxy_la_LDFLAGS = -version-info 0:0:0

if DATAFILES
pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = libproxy/libproxy-1.0.pc
endif

bin_PROGRAMS = tools/proxy

tools_proxy_SOURCES = tools/proxy.c

tools_proxy_LDADD = libproxy/libproxy.la
endif

noinst_PROGRAMS = unit/test-mozjs

unit_test_mozjs_SOURCES = unit/test-mozjs.c \
				src/pacrunner.h src/mozjs.c

unit_test_mozjs_LDADD = @MOZJS_LIBS@ @GLIB_LIBS@

AM_CFLAGS = @CAPNG_CFLAGS@ @DBUS_CFLAGS@ @GLIB_CFLAGS@ @MOZJS_CFLAGS@

INCLUDES = -I$(srcdir)/src -I$(srcdir)/gdbus

test_scripts = test/find-proxy-for-url test/create-proxy-config

if TEST
testdir = $(pkglibdir)/test
test_SCRIPTS = $(test_scripts)
endif

EXTRA_DIST = src/pacrunner.conf $(test_scripts) \
		doc/architecture.txt doc/manager-api.txt doc/client-api.txt

DISTCHECK_CONFIGURE_FLAGS = --enable-libproxy --disable-datafiles

MAINTAINERCLEANFILES = Makefile.in \
	aclocal.m4 configure config.h.in config.sub config.guess \
	ltmain.sh depcomp compile missing install-sh mkinstalldirs
