dnl
dnl tp2 - Tiled Powers of 2
dnl Copyright (C) 2016 Tim Hentenaar.
dnl
dnl This code is licenced under the Simplified BSD License.
dnl See the LICENSE file for details.
dnl
AC_PREREQ([2.63])
AC_INIT([tp2], [1.0], [http://github.com/thentenaar/tp2])
AC_CONFIG_AUX_DIR([.])
AC_LANG([C])

dnl Include local macros
m4_include([m4/ax_append_compile_flags.m4])
m4_include([m4/ax_append_flag.m4])
m4_include([m4/ax_check_compile_flag.m4])
m4_include([m4/ax_require_defined.m4])
m4_include([m4/ax_strict_cflags.m4])
m4_include([m4/ax_set_prefix.m4])
m4_include([m4/ax_check_curses.m4])

dnl Check for standard programs
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AC_PROG_INSTALL

dnl Check for headers
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS([limits.h time.h signal.h unistd.h term.h])

dnl Check compiler characteristics
AC_C_CONST
AC_TYPE_SIZE_T

dnl Check for curses
AX_CHECK_CURSES
AS_IF([test "$have_curses" != "yes"],[
	AC_MSG_ERROR("curses is required to build tp2.")
])

dnl Check for indent
AC_PATH_PROG([INDENT],[indent])

dnl Use strict CFLAGS and SUSv2 compliant interfaces
CFLAGS="-O2 -D_XOPEN_SOURCE=500 -ansi"
AX_STRICT_CFLAGS
CFLAGS="$CFLAGS -Werror"

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
