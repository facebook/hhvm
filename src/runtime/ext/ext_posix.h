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

inline String f_posix_ctermid() {
  char *buffer = (char *)malloc(L_ctermid);
  ctermid(buffer);
  return String(buffer, AttachString);
}

inline int f_posix_get_last_error() {
  return errno;
}

inline String f_posix_getcwd() {
  char *buffer = (char *)malloc(PATH_MAX);
  if (getcwd(buffer, PATH_MAX) == NULL) {
    free(buffer);
    return "/";
  }
  return String(buffer, AttachString);
}

inline int f_posix_getegid() {
  return getegid();
}

inline int f_posix_geteuid() {
  return geteuid();
}

inline int f_posix_getgid() {
  return getgid();
}

Variant f_posix_getgrgid(int gid);

Variant f_posix_getgrnam(CStrRef name);

Variant f_posix_getgroups();

inline Variant f_posix_getlogin() {
  char buf[L_cuserid];
  if (!getlogin_r(buf, sizeof(buf) - 1)) {
    return String(buf, CopyString);
  }
  return false;
}

inline Variant f_posix_getpgid(int pid) {
  int ret = getpgid(pid);
  if (ret < 0) return false;
  return ret;
}

inline int f_posix_getpgrp() {
  return getpgrp();
}

inline int f_posix_getpid() {
  return getpid();
}

inline int f_posix_getppid() {
  return getppid();
}

Variant f_posix_getpwnam(CStrRef username);

Variant f_posix_getpwuid(int uid);

Variant f_posix_getrlimit();

inline Variant f_posix_getsid(int pid) {
  int ret = getsid(pid);
  if (ret < 0) return false;
  return ret;
}

inline int f_posix_getuid() {
  return getuid();
}

inline bool f_posix_initgroups(CStrRef name, int base_group_id) {
  if (name.empty()) return false;
  return !initgroups(name.data(), base_group_id);
}

bool f_posix_isatty(CVarRef fd);

inline bool f_posix_kill(int pid, int sig) {
  return kill(pid, sig) >= 0;
}

inline bool f_posix_mkfifo(CStrRef pathname, int mode) {
  return mkfifo(pathname.data(), mode) >= 0;
}

bool f_posix_mknod(CStrRef pathname, int mode, int major = 0, int minor = 0);

inline bool f_posix_setegid(int gid) {
  return setegid(gid);
}

inline bool f_posix_seteuid(int uid) {
  return seteuid(uid);
}

inline bool f_posix_setgid(int gid) {
  return setgid(gid);
}

inline bool f_posix_setpgid(int pid, int pgid) {
  return setpgid(pid, pgid) >= 0;
}

inline int f_posix_setsid() {
  return setsid();
}

inline bool f_posix_setuid(int uid) {
  return setuid(uid);
}

inline String f_posix_strerror(int errnum) {
  return String(Util::safe_strerror(errnum));
}

Variant f_posix_times();

Variant f_posix_ttyname(CVarRef fd);

Variant f_posix_uname();

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_POSIX_H__
