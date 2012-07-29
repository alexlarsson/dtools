AC_PREREQ([2.63])
AC_INIT([dtools],[0.0.1])
AC_CONFIG_SRCDIR([pipeneg.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
# Enable silent rules is available
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE([enable])
AC_CONFIG_FILES([Makefile])

AC_SYS_LARGEFILE
AC_PROG_CC
AC_PROG_INSTALL

glib_modules="glib-2.0"
PKG_CHECK_MODULES(GLIB, [$glib_modules])

gio_modules="glib-2.0 gthread-2.0 gio-2.0 >= 2.26"
PKG_CHECK_MODULES(GIO, [$gio_modules])

if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[[\ \	]]-Wall[[\ \	]]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi

AC_OUTPUT
