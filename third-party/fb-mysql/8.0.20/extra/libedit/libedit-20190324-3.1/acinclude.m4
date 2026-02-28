
dnl
dnl read lib version from file (and trim trailing newline)
dnl
define([EL_RELEASE], [patsubst(esyscmd([. src/shlib_version; echo $major.$minor]), [
])])

dnl
dnl read cvsexport timestamp from file (and trim trailing newline)
dnl
define([EL_TIMESTAMP], [patsubst(esyscmd([date +"%Y%m%d"]), [
])])


dnl
dnl NetBSD use the -mdoc macro package for manpages, but e.g.
dnl AIX and Solaris only support the -man package.
dnl
AC_DEFUN([EL_MANTYPE],
[
   MANTYPE=
   TestPath="/usr/bin${PATH_SEPARATOR}/usr/ucb"
   AC_PATH_PROGS(NROFF, nroff awf, /bin/false, $TestPath)
   if ${NROFF} -mdoc ${srcdir}/doc/editrc.5.roff >/dev/null 2>&1; then
      MANTYPE=mdoc
   fi
   AC_SUBST(MANTYPE)
])


dnl
dnl Check if getpwnam_r and getpwuid_r are POSIX.1 compatible
dnl POSIX draft version returns 'struct passwd *' (used on Solaris)
dnl NOTE: getpwent_r is not POSIX so we always use getpwent
dnl
AC_DEFUN([EL_GETPW_R_POSIX],
[
   AC_MSG_CHECKING([whether getpwnam_r and getpwuid_r are posix like])
      # The prototype for the POSIX version is:
      # int getpwnam_r(char *, struct passwd *, char *, size_t, struct passwd **)
      # int getpwuid_r(uid_t, struct passwd *, char *, size_t, struct passwd **);
   AC_TRY_LINK([#include <stdlib.h>
                #include <sys/types.h>
                #include <pwd.h>],
               [getpwnam_r(NULL, NULL, NULL, (size_t)0, NULL);
                getpwuid_r((uid_t)0, NULL, NULL, (size_t)0, NULL);],
      [AC_DEFINE([HAVE_GETPW_R_POSIX], 1, [Define to 1 if you have getpwnam_r and getpwuid_r that are POSIX.1 compatible.])
       AC_MSG_RESULT(yes)],
      [AC_MSG_RESULT(no)])
])

AC_DEFUN([EL_GETPW_R_DRAFT],
[
   AC_MSG_CHECKING([whether getpwnam_r and getpwuid_r are posix _draft_ like])
      # The prototype for the POSIX draft version is:
      # struct passwd *getpwuid_r(uid_t, struct passwd *, char *, int);
      # struct passwd *getpwnam_r(char *, struct passwd *,  char *, int);
   AC_TRY_LINK([#include <stdlib.h>
                #include <sys/types.h>
                #include <pwd.h>],
               [getpwnam_r(NULL, NULL, NULL, (size_t)0);
                getpwuid_r((uid_t)0, NULL, NULL, (size_t)0);],
      [AC_DEFINE([HAVE_GETPW_R_DRAFT], 1, [Define to 1 if you have getpwnam_r and getpwuid_r that are draft POSIX.1 versions.])
       AC_MSG_RESULT(yes)],
      [AC_MSG_RESULT(no)])
])


dnl
dnl deprecate option --enable-widec to turn on use of wide-character support
dnl
AC_DEFUN([EL_DEPRECATE_WIDEC],
[
   AC_MSG_CHECKING(if you want wide-character code)
   AC_ARG_ENABLE(widec,
      [  --enable-widec          deprecated, wide-character/UTF-8 is always enabled],
      [with_widec=$enableval],
      [with_widec=no])
   AC_MSG_RESULT($with_widec)
   AC_MSG_WARN([--enable-widec is deprecated, wide-character/UTF-8 is always enabled])
])

