/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef __EXT_POSIX_H__
#define __EXT_POSIX_H__

#include <runtime/base/base_includes.h>
#include <sys/types.h>
#include <grp.h>
#include <signal.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool f_posix_access(CStrRef file, int mode = 0);

String f_posix_ctermid();

int64_t f_posix_get_last_error();

String f_posix_getcwd();

int64_t f_posix_getegid();

int64_t f_posix_geteuid();

int64_t f_posix_getgid();

Variant f_posix_getgrgid(int gid);

Variant f_posix_getgrnam(CStrRef name);

Variant f_posix_getgroups();

Variant f_posix_getlogin();

Variant f_posix_getpgid(int pid);

int64_t f_posix_getpgrp();

int64_t f_posix_getpid();

int64_t f_posix_getppid();

Variant f_posix_getpwnam(CStrRef username);

Variant f_posix_getpwuid(int uid);

Variant f_posix_getrlimit();

Variant f_posix_getsid(int pid);

int64_t f_posix_getuid();

bool f_posix_initgroups(CStrRef name, int base_group_id);

bool f_posix_isatty(CVarRef fd);

bool f_posix_kill(int pid, int sig);

bool f_posix_mkfifo(CStrRef pathname, int mode);

bool f_posix_mknod(CStrRef pathname, int mode, int major = 0, int minor = 0);

bool f_posix_setegid(int gid);

bool f_posix_seteuid(int uid);

bool f_posix_setgid(int gid);

bool f_posix_setpgid(int pid, int pgid);

int64_t f_posix_setsid();

bool f_posix_setuid(int uid);

String f_posix_strerror(int errnum);

Variant f_posix_times();

Variant f_posix_ttyname(CVarRef fd);

Variant f_posix_uname();

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_POSIX_H__
