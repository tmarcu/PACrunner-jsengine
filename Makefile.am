
AM_MAKEFLAGS = --no-print-directory

sbin_PROGRAMS = src/pacrunner

src_pacrunner_SOURCES = src/main.c src/pacrunner.h src/log.c

src_pacrunner_LDADD = @GLIB_LIBS@ @CAPNG_LIBS@

AM_CFLAGS = @CAPNG_CFLAGS@ @GLIB_CFLAGS@

INCLUDES = -I$(builddir)/src

DISTCHECK_CONFIGURE_FLAGS = --disable-datafiles

MAINTAINERCLEANFILES = Makefile.in \
	aclocal.m4 configure config.h.in depcomp missing install-sh
