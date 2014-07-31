#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE(fnkey,1.0.1)
AC_CONFIG_SRCDIR([OnScrDsp.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
pkg_modules="gtk+-2.0 glib-2.0 xtst gthread-2.0"
PKG_CHECK_MODULES(PKGMODULES, [$pkg_modules], , )
AC_SUBST(CFLAGS, [$PKGMODULES_CFLAGS])
AC_SUBST(LDADD, [$PKGMODULES_LIBS])

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset mkfifo strchr strerror strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
