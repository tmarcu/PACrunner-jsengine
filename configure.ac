AC_PREREQ(2.60)
AC_INIT(pacrunner, 0.5)

AM_INIT_AUTOMAKE([foreign subdir-objects color-tests])
AM_CONFIG_HEADER(config.h)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_MAINTAINER_MODE

AC_PREFIX_DEFAULT(/usr/local)

if (test "${libdir}" = '${exec_prefix}/lib'); then
	libdir="${prefix}/lib"
fi

plugindir="${libdir}/pacrunner/plugins"

PKG_PROG_PKG_CONFIG

COMPILER_FLAGS

AC_LANG_C

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CC_PIE
AC_PROG_INSTALL

m4_define([_LT_AC_TAGCONFIG], [])
m4_ifdef([AC_LIBTOOL_TAGS], [AC_LIBTOOL_TAGS([])])

AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_ARG_ENABLE(optimization, AC_HELP_STRING([--disable-optimization],
			[disable code optimization through compiler]), [
	if (test "${enableval}" = "no"); then
		CFLAGS="$CFLAGS -O0"
	fi
])

AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug],
			[enable compiling with debugging information]), [
	if (test "${enableval}" = "yes" &&
				test "${ac_cv_prog_cc_g}" = "yes"); then
		CFLAGS="$CFLAGS -g"
	fi
])

AC_ARG_ENABLE(unit, AC_HELP_STRING([--enable-unit],
			[enable compiling unit test binaries]), [
	if (test "${enable_unit}" = "yes"); then
		AC_CHECK_LIB([cunit], [CU_initialize_registry], [], [
			AC_MSG_ERROR([CUnit is required to get the unit tests])
		])
		LDFLAGS="$LDFLAGS -lcunit"
	fi
])

AM_CONDITIONAL(UNIT, test "${enable_unit}" = "yes")

AC_ARG_ENABLE(pie, AC_HELP_STRING([--enable-pie],
			[enable position independent executables flag]), [
	if (test "${enableval}" = "yes" &&
				test "${ac_cv_prog_cc_pie}" = "yes"); then
		CFLAGS="$CFLAGS -fPIE"
		LDFLAGS="$LDFLAGS -pie"
	fi
])

AC_ARG_ENABLE(mozjs, AC_HELP_STRING([--enable-mozjs],
		[enable Mozilla Javascript plugin support]))
if (test "${enable_mozjs}" = "yes"); then
	PKG_CHECK_MODULES(MOZJS,  mozilla-js >= 1.9, dummy=yes,
			AC_MSG_ERROR(Mozilla Javascript >= 1.9 is required))

	mozjs_version=`pkg-config --modversion mozilla-js | cut -d '.' -f 1`
	if test ${mozjs_version} -gt 1; then
		MOZJS_CFLAGS="${MOZJS_CFLAGS} -DMOZJS_VERSION_2_OR_SUPERIOR=1"
	fi

	AC_SUBST(MOZJS_CFLAGS)
	AC_SUBST(MOZJS_LIBS)
fi
AM_CONDITIONAL(MOZJS, test "${enable_mozjs}" = "yes")


AC_ARG_ENABLE(v8, AC_HELP_STRING([--enable-v8],
		[enable V8 Javascript plugin support]))
if (test "${enable_v8}" = "yes"); then
	AC_CHECK_LIB(v8, _ZN2v82V810InitializeEv,
		[V8_LIBS="-lv8 -lpthread" AC_SUBST(V8_LIBS)],
		[AC_MSG_ERROR(Chrome v8 is required)], -lpthread)
fi
AM_CONDITIONAL(V8, test "${enable_v8}" = "yes")

PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.16, dummy=yes,
				AC_MSG_ERROR(GLib >= 2.16 is required))
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(DBUS, dbus-1 >= 1.2, dummy=yes,
				AC_MSG_ERROR(D-Bus >= 1.2 is required))
AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS)

AC_ARG_WITH(dbusconfdir, AC_HELP_STRING([--with-dbusconfdir=PATH],
	[path to D-Bus config directory]), [path_dbusconf=${withval}],
		[path_dbusconf="`$PKG_CONFIG --variable=sysconfdir dbus-1`"])
if (test -z "${path_dbusconf}"); then
	DBUS_CONFDIR="${sysconfdir}/dbus-1/system.d"
else
	DBUS_CONFDIR="${path_dbusconf}/dbus-1/system.d"
fi
AC_SUBST(DBUS_CONFDIR)

AC_ARG_WITH(dbusdatadir, AC_HELP_STRING([--with-dbusdatadir=PATH],
	[path to D-Bus data directory]), [path_dbusdata=${withval}],
		[path_dbusdata="`$PKG_CONFIG --variable=datadir dbus-1`"])
if (test -z "${path_dbusdata}"); then
	DBUS_DATADIR="${datadir}/dbus-1/system-services"
else
	DBUS_DATADIR="${path_dbusdata}/dbus-1/system-services"
fi
AC_SUBST(DBUS_DATADIR)

AC_ARG_ENABLE(capng, AC_HELP_STRING([--enable-capng],
		[enable capabilities support]), [enable_capng=${enableval}])
if (test "${enable_capng}" = "yes"); then
	PKG_CHECK_MODULES(CAPNG, libcap-ng, dummy=yes,
				AC_MSG_ERROR(Capabilities library is required))
	AC_SUBST(CAPNG_CFLAGS)
	AC_SUBST(CAPNG_LIBS)
	AC_DEFINE(HAVE_CAPNG, 1, [Define to 1 if you have capabilities library.])
fi

PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.16, dummy=yes,
			AC_MSG_ERROR(GThread >= 2.16 is required))
AC_SUBST(GTHREAD_CFLAGS)
AC_SUBST(GTHREAD_LIBS)

AC_ARG_ENABLE(curl, AC_HELP_STRING([--enable-curl],
		[enable CURL plugin support]), [enable_capng=${enableval}])
if (test "${enable_curl}" = "yes"); then
	PKG_CHECK_MODULES(CURL, libcurl >= 7.16, dummy=yes,
				AC_MSG_ERROR(CURL >= 7.16 is required))
	AC_SUBST(CURL_CFLAGS)
	AC_SUBST(CURL_LIBS)
fi
AM_CONDITIONAL(CURL, test "${enable_curl}" = "yes")

AC_ARG_ENABLE(test, AC_HELP_STRING([--enable-test],
		[enable test/example scripts]), [enable_test=${enableval}])
AM_CONDITIONAL(TEST, test "${enable_test}" = "yes")

AC_ARG_ENABLE(libproxy, AC_HELP_STRING([--enable-libproxy],
	[enable libproxy compatible library]), [enable_libproxy=${enableval}])
AM_CONDITIONAL(LIBPROXY, test "${enable_libproxy}" = "yes")

AC_ARG_ENABLE(datafiles, AC_HELP_STRING([--disable-datafiles],
			[don't install configuration and data files]),
					[enable_datafiles=${enableval}])
AM_CONDITIONAL(DATAFILES, test "${enable_datafiles}" != "no")

AC_OUTPUT(Makefile src/pacrunner.service libproxy/libproxy-1.0.pc)
