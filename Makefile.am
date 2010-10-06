
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

AM_CFLAGS = @CAPNG_CFLAGS@ @DBUS_CFLAGS@ @GLIB_CFLAGS@ @MOZJS_CFLAGS@

INCLUDES = -I$(srcdir)/src -I$(srcdir)/gdbus

test_scripts = test/find-proxy-for-url test/create-proxy-config

if TEST
testdir = $(pkglibdir)/test
test_SCRIPTS = $(test_scripts)
endif

EXTRA_DIST = src/pacrunner.conf $(test_scripts) \
		doc/architecture.txt doc/manager-api.txt doc/client-api.txt

DISTCHECK_CONFIGURE_FLAGS = --disable-datafiles

MAINTAINERCLEANFILES = Makefile.in \
	aclocal.m4 configure config.h.in depcomp missing install-sh
