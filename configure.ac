AC_PREREQ([2.72])
AC_INIT([ped], [0.0.1], [], [], [https://github.com/tim-tm/ped])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

PKG_CHECK_MODULES([NCURSES], ncurses, [], AC_MSG_ERROR([Failed to find ncurses]))

AC_PROG_CC
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

AC_FUNC_REALLOC

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
