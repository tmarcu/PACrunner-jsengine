
AM_MAKEFLAGS = --no-print-directory

if DATAFILES
dbusdir = @DBUS_DATADIR@

dbus_DATA = src/pacrunner.conf
endif

gdbus_sources = gdbus/gdbus.h gdbus/mainloop.c gdbus/watch.c \
					gdbus/object.c gdbus/polkit.c

sbin_PROGRAMS = src/pacrunner

src_pacrunner_SOURCES = $(gdbus_sources) \
			src/main.c src/pacrunner.h src/log.c src/mozjs.c

src_pacrunner_LDADD = @XUL_LIBS@ @GLIB_LIBS@ @DBUS_LIBS@ @CAPNG_LIBS@

AM_CFLAGS = @CAPNG_CFLAGS@ @DBUS_CFLAGS@ @GLIB_CFLAGS@ @XUL_CFLAGS@

INCLUDES = -I$(builddir)/src -I$(builddir)/gdbus

EXTRA_DIST = src/pacrunner.conf

DISTCHECK_CONFIGURE_FLAGS = --disable-datafiles

MAINTAINERCLEANFILES = Makefile.in \
	aclocal.m4 configure config.h.in depcomp missing install-sh
