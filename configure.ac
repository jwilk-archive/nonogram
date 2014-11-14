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

AC_CHECK_FUNC(
    [sigaction],
    [AC_DEFINE([HAVE_SIGACTION], [1], [Define if sigaction(2) is available])],
)

PKG_CHECK_MODULES(
    [NCURSES],
    [ncurses],
    [AC_DEFINE([HAVE_NCURSES], [1], [Define if ncurses is available])],
    [NCURSES_CFLAGS=; NCURSES_LIBS=]
)

AC_ARG_ENABLE(
    [debug],
    [AS_HELP_STRING(
       [--enable-debug],
       [enable debugging features]
    )],
)

AS_IF(
    [test "$enable_debug" = yes],
    [AC_DEFINE([ENABLE_DEBUG], [1], [Define to enable debugging features])]
)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

dnl vim: ts=4 sts=4 sw=4 et
