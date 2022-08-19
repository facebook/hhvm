# Copyright (c) 2007, 2016, Oracle and/or its affiliates. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

AC_DEFUN([_MYSQL_CONFIG],[
  AC_ARG_WITH([mysql-config],
  AS_HELP_STRING([--with-mysql-config=PATH], [A path to mysql_config script]),
                 [mysql_config="$withval"], [mysql_config=mysql_config])
])

dnl
dnl Usage:
dnl
dnl  MYSQL_CLIENT([version], [client|thread-safe])
dnl
dnl Two optional arguments:
dnl   first: The minimal version of the MySQL to use
dnl           if not specified, any version will be accepted.
dnl           The version should be specified as three numbers,
dnl           without suffixes. E.g. 4.10.15 or 5.0.3
dnl   second: force the specified library flavor to be selected,
dnl           if not specified, a user will be able to choose
dnl           between client (non-thread-safe) and thread-safe
dnl
dnl On successful execution sets MYSQL_CLIENT_CFLAGS and
dnl MYSQL_CLIENT_LIBS shell variables and makes substitutions
dnl out of them (calls AC_SUBST)
dnl

AC_DEFUN([MYSQL_CLIENT],[
  AC_REQUIRE([_MYSQL_CONFIG])
  AC_MSG_CHECKING([for MySQL])
  ifelse([$2], [client],
               [mysql_libs=--libs mysql_cflags=--cflags],
         [$2], [thread-safe],
               [mysql_libs=--libs_r mysql_cflags=--cflags],
         [$2], [], [
    AC_ARG_WITH([mysql-library],
    AS_HELP_STRING([--with-mysql-library], ['client' or 'thread-safe']),
                   [mysql_lib="$withval"], [mysql_lib=client])
[                   
    case "$mysql_lib" in
      client) mysql_libs=--libs mysql_cflags=--cflags ;;
      thread-safe) mysql_libs=--libs mysql_cflags=--cflags ;;
      *) ]AC_MSG_ERROR([Bad value for --with-mysql-library])[
    esac
]
                   ],
          [AC_FATAL([Bad second (library flavor) argument to MYSQL_CLIENT])])
[
    mysql_version=`$mysql_config --version`
    if test -z "$mysql_version" ; then
      ]AC_MSG_ERROR([Cannot execute $mysql_config])[
    fi
]
    ifelse([$1], [], [], [
      ifelse(regexp([$1], [^[0-9][0-9]?\.[0-9][0-9]?\.[0-9][0-9]?$]), -1,
      [AC_FATAL([Bad first (version) argument to MYSQL_CLIENT])], [
dnl
dnl Transformation below works as follows:
dnl   assume, we have a number 1.2.3-beta
dnl   *a* line removes the suffix and adds first and last dot to the version:
dnl             .1.2.3.
dnl   *b* line adds a 0 to a "single digit surrounded by dots"
dnl             .01.2.03.
dnl       note that the pattern that matched .1. has eaten the dot for .2.
dnl       and 2 still has no 0
dnl   *c* we repeat the same replacement as in *b*, matching .2. this time
dnl             .01.02.03.
dnl   the last replacement removes all dots
dnl             010203
dnl   giving us a number we can compare with
dnl
    mysql_ver=`echo ${mysql_version}|dnl
      sed 's/[[-a-z]].*//; s/.*/.&./;dnl   *a*
           s/\.\([[0-9]]\)\./.0\1./g;dnl   *b*
           s/\.\([[0-9]]\)\./.0\1./g;dnl   *c*
           s/\.//g'`
    if test "$mysql_ver" -lt]dnl
dnl the same as sed transformation above, without suffix-stripping, in m4
    patsubst(patsubst(patsubst(.[$1]., [\.\([0-9]\)\.], [.0\1.]), [\.\([0-9]\)\.], [.0\1.]), [\.], [])[ ; then
      AC_MSG_ERROR([MySQL version $mysql_version is too low, minimum of $1 is required])
    fi
    ])])

    MYSQL_CLIENT_CFLAGS=`$mysql_config $mysql_cflags`
    MYSQL_CLIENT_LIBS=`$mysql_config $mysql_libs`
    AC_SUBST(MYSQL_CLIENT_CFLAGS)
    AC_SUBST(MYSQL_CLIENT_LIBS)

    # should we try to build a test program ?

    AC_MSG_RESULT([$mysql_version])
])

