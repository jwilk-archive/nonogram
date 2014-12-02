dnl | Copyright © 2014 Jakub Wilk <jwilk@jwilk.net>
dnl |
dnl | Permission is hereby granted, free of charge, to any person obtaining a
dnl | copy of this software and associated documentation files (the
dnl | “Software”), to deal in the Software without restriction, including
dnl | without limitation the rights to use, copy, modify, merge, publish,
dnl | distribute, sublicense, and/or sell copies of the Software, and to
dnl | permit persons to whom the Software is furnished to do so, subject to
dnl | the following conditions:
dnl |
dnl | The above copyright notice and this permission notice shall be included
dnl | in all copies or substantial portions of the Software.
dnl |
dnl | THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
dnl | OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
dnl | MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
dnl | IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
dnl | CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
dnl | TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
dnl | SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

AC_PREREQ([2.61])
AC_INIT([nonogram], [0.9], [jwilk@jwilk.net])
AC_CONFIG_HEADER([autoconfig.h])

AC_PROG_CC_C99
AC_PROG_CPP
AX_CFLAGS_WARN_ALL

AC_DEFINE([_ISOC99_SOURCE], [1], [Define if your system supports C99])
# _ISOC99_SOURCE enables MinGW ANSI stdio.

AC_CHECK_FUNC(
    [sigaction],
    [AC_DEFINE([HAVE_SIGACTION], [1], [Define if sigaction(2) is available])],
)

AC_ARG_WITH(
    [ncurses],
    [AS_HELP_STRING(
        [--with-ncurses],
        [use ncurses @<:@default=check@:>@]
    )],
    [],
    [with_ncurses=check],
)

if test "$with_ncurses" != no
then
    PKG_CHECK_MODULES(
        [NCURSES],
        [ncurses],
        [],
        [NCURSES_CFLAGS=; NCURSES_LIBS=-lncurses]
    )
    AC_MSG_CHECKING([for ncurses.h])
    AC_PREPROC_IFELSE(
        [AC_LANG_SOURCE([[#include <ncurses.h>]])],
        [
            AC_MSG_RESULT([yes])
            AC_DEFINE([HAVE_NCURSES], [1], [Define if ncurses is available])
        ], [
            AC_MSG_RESULT([no])
            if test "$with_ncurses" = check
            then
                NCURSES_CFLAGS=
                NCURSES_LIBS=
            else
                AC_MSG_ERROR([ncurses.h not found])
            fi
        ]
    )
fi

AC_ARG_ENABLE(
    [debug],
    [AS_HELP_STRING(
       [--enable-debug],
       [enable debugging features]
    )],
)

enable_debug=$(test "$enable_debug" = yes && echo 1 || echo 0)
AC_DEFINE_UNQUOTED([ENABLE_DEBUG], [$enable_debug], [Define to enable debugging features])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

dnl vim: ts=4 sts=4 sw=4 et
