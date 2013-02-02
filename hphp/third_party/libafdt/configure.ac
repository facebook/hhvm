AC_PREREQ([2.63])
AC_INIT([libafdt], [0.1.0], [dreiss@facebook.com])
AC_CONFIG_SRCDIR([src/afdt.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

AC_PROG_CC
AC_PROG_MAKE_SET

LT_INIT([disable-shared])

dnl TODO(dreiss): Make configure fail if these tests fail.

AC_C_INLINE

AC_HEADER_STDBOOL
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([stddef.h])
AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([unistd.h])

AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([strerror])

AC_CHECK_DECLS([AF_LOCAL], [], [], [[#include <sys/socket.h>]])
AC_CHECK_DECLS([PF_LOCAL], [], [], [[#include <sys/socket.h>]])

if test "x$ax_have_libevent" = "xyes" ; then : ; else
  AX_LIB_EVENT([2.0.9])
fi
if test "x$ax_have_libevent" = "xyes" ; then : ; else
  AX_LIB_EVENT([1.4.5])
fi
if test "x$ax_have_libevent" = "xyes" ; then : ; else
  AX_LIB_EVENT([1.4.3])
fi
if test "x$ax_have_libevent" = "xyes" ; then : ; else
  AX_LIB_EVENT
fi

if test "x$ax_hav9_libevent_2_0_9" = "xyes" ; then
  have_libevent_209="yes"
fi
if test "x$ax_have_libevent_2_0_9$ax_have_libevent_1_4_5" = "xyes" ; then
  AC_DEFINE([HAVE_LIBEVENT_145], 1,
            [libevent 1.4.5 or greater is present])
fi
if test "x$ax_have_libevent_2_0_9$ax_have_libevent_1_4_5$ax_have_libevent_1_4_3" = "xyes" ; then
  have_libevent_143="yes"
fi

AM_CONDITIONAL([HAVE_LIBEVENT_143], [test "x$have_libevent_143" = "xyes"])
AM_CONDITIONAL([HAVE_LIBEVENT_209], [test "x$have_libevent_209" = "xyes"])
AM_CONDITIONAL([HAVE_LIBEVENT], [test "x$ax_have_libevent" = "xyes"])

AM_PATH_PYTHON([2.6],, [:])
AM_CONDITIONAL([HAVE_PYTHON], [test "x$PYTHON" != "x:"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
