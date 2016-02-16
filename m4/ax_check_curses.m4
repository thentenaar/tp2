dnl
dnl tp2 - Tiled Powers of 2
dnl Copyright (C) 2016 Tim Hentenaar.
dnl
dnl This code is licenced under the Simplified BSD License.
dnl See the LICENSE file for details.
dnl
dnl SYNOPSIS
dnl
dnl AX_CHECK_CURSES - Check for a usable curses installation.
dnl
dnl DESCRIPTION
dnl
dnl This macro sets `have_curses` to either "yes" or "no" to indicate
dnl the presence of curses, and appends the necessary options to
dnl CPPFLAGS, LDFLAGS, and LIBS.
dnl

AC_DEFUN([AX_CHECK_CURSES],[
	have_curses=no
	AC_ARG_WITH([curses],
		[AS_HELP_STRING(
			[--with-curses=PATH],
			[Set the path to the curses installation])],
		[with_curses=$withval],
		[with_curses=yes]
	)

	AS_IF([test "$with_curses" != "no"],[
		have_curses=yes
		save_cppflags=$CPPFLAGS
		save_ldflags=$LDFLAGS
		save_libs=$LIBS

		dnl Check for curses.h
		AS_IF([test "$with_curses" == "yes"],[with_curses=/usr])
		CPPFLAGS="$CPPFLAGS -I$with_curses/include"
		AC_CHECK_HEADER([curses.h], [], [have_curses=no])

		dnl Check for libcurses
		AS_IF([test "$have_curses" != "no"],[
			LDFLAGS="$LDFLAGS -L$with_curses/lib"
			AC_CHECK_LIB([curses], [tgetent], [], [have_curses=no])
			LIBS=$save_LIBS

			dnl See if -ltinfo or -ltermcap is needed
			AS_IF([test "$have_curses" == "no"],[
				have_curses=yes
				AC_CHECK_LIB([tinfo], [tgetent], [], [have_curses=no])
				AS_IF([test "$have_curses" == "no"],[
					AC_CHECK_LIB([termcap], [tgetent], [], [])
					have_curses=yes
				])
			])
			AC_CHECK_LIB([curses], [initscr], [], [have_curses=no])
		])
	])
]) dnl AX_CHECK_CURSES
