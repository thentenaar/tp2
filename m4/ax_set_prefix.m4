dnl
dnl tp2 - Tiled Powers of 2
dnl Copyright (C) 2016 Tim Hentenaar.
dnl
dnl This code is licenced under the Simplified BSD License.
dnl See the LICENSE file for details.
dnl
dnl SYNOPSIS
dnl
dnl AX_SET_PREFIX - Set $prefix unless the user has overridden it.
dnl

AC_DEFUN([AX_SET_PREFIX],[
	AS_IF([test "$prefix" == "NONE"],[
		AS_IF([test "x$ac_default_prefix" != "xNONE"],[
			prefix=$ac_default_prefix
		])
	])
]) dnl AX_SET_PREFIX
