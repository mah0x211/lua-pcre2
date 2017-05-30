#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
AC_PREREQ(2.61)
AC_INIT(pcre2, 10.23)
AM_INIT_AUTOMAKE([foreign no-dist no-installinfo no-installman -Wall])
AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_MACRO_DIR([m4])


#
# Checks for programs.
#
AC_LANG_C
AC_PROG_CC

#
# define luarocks variables
#
AC_SUBST([LIBFLAG], ["$LIBFLAG"])
AC_SUBST([OBJ_EXTENSION], ["$OBJ_EXTENSION"])
AC_SUBST([LIB_EXTENSION], ["$LIB_EXTENSION"])
AC_SUBST([LIBDIR], ["$LIBDIR"])
AC_SUBST([LIBS], ["$LIBS"])

#
# checking required headers
#
AC_CHECK_HEADERS(
    stdio.h string.h errno.h,,
    AC_MSG_FAILURE([required header not found])
)


AC_CONFIG_FILES([ \
    Makefile \
    src/Makefile \
])
AC_OUTPUT
