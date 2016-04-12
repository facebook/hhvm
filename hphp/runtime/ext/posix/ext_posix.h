/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_POSIX_H_
#define incl_HPHP_EXT_POSIX_H_

#include "hphp/runtime/ext/extension.h"
#include <sys/types.h>
#include <grp.h>
#include <signal.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_POSIX_S_IFMT;
extern const int64_t k_POSIX_S_IFSOCK;
extern const int64_t k_POSIX_S_IFLNK;
extern const int64_t k_POSIX_S_IFREG;
extern const int64_t k_POSIX_S_IFBLK;
extern const int64_t k_POSIX_S_IFDIR;
extern const int64_t k_POSIX_S_IFCHR;
extern const int64_t k_POSIX_S_IFIFO;
extern const int64_t k_POSIX_S_ISUID;
extern const int64_t k_POSIX_S_ISGID;
extern const int64_t k_POSIX_S_ISVTX;
extern const int64_t k_POSIX_S_IRWXU;
extern const int64_t k_POSIX_S_IRUSR;
extern const int64_t k_POSIX_S_IWUSR;
extern const int64_t k_POSIX_S_IXUSR;
extern const int64_t k_POSIX_S_IRWXG;
extern const int64_t k_POSIX_S_IRGRP;
extern const int64_t k_POSIX_S_IWGRP;
extern const int64_t k_POSIX_S_IXGRP;
extern const int64_t k_POSIX_S_IRWXO;
extern const int64_t k_POSIX_S_IROTH;
extern const int64_t k_POSIX_S_IWOTH;
extern const int64_t k_POSIX_S_IXOTH;
extern const int64_t k_POSIX_F_OK;
extern const int64_t k_POSIX_X_OK;
extern const int64_t k_POSIX_W_OK;
extern const int64_t k_POSIX_R_OK;

bool HHVM_FUNCTION(posix_access,
                   const String& file,
                   int mode = 0);

String HHVM_FUNCTION(posix_ctermid);

int64_t HHVM_FUNCTION(posix_get_last_error);

int64_t HHVM_FUNCTION(posix_errno);

String HHVM_FUNCTION(posix_getcwd);

int64_t HHVM_FUNCTION(posix_getegid);

int64_t HHVM_FUNCTION(posix_geteuid);

int64_t HHVM_FUNCTION(posix_getgid);

Variant HHVM_FUNCTION(posix_getgrgid,
                      int gid);

Variant HHVM_FUNCTION(posix_getgrnam,
                      const String& name);

Variant HHVM_FUNCTION(posix_getgroups);

Variant HHVM_FUNCTION(posix_getlogin);

Variant HHVM_FUNCTION(posix_getpgid,
                      int pid);

int64_t HHVM_FUNCTION(posix_getpgrp);

int64_t HHVM_FUNCTION(posix_getpid);

int64_t HHVM_FUNCTION(posix_getppid);

Variant HHVM_FUNCTION(posix_getpwnam,
                      const String& username);

Variant HHVM_FUNCTION(posix_getpwuid,
                      int uid);

Variant HHVM_FUNCTION(posix_getrlimit);

Variant HHVM_FUNCTION(posix_getsid,
                      int pid);

int64_t HHVM_FUNCTION(posix_getuid);

bool HHVM_FUNCTION(posix_initgroups,
                   const String& name,
                   int base_group_id);

bool HHVM_FUNCTION(posix_isatty,
                   const Variant& fd);

bool HHVM_FUNCTION(posix_kill,
                   int pid,
                   int sig);

bool HHVM_FUNCTION(posix_mkfifo,
                   const String& pathname,
                   int mode);

bool HHVM_FUNCTION(posix_mknod,
                   const String& pathname,
                   int mode,
                   int major = 0,
                   int minor = 0);

bool HHVM_FUNCTION(posix_setegid,
                   int gid);

bool HHVM_FUNCTION(posix_seteuid,
                   int uid);

bool HHVM_FUNCTION(posix_setgid,
                   int gid);

bool HHVM_FUNCTION(posix_setpgid,
                   int pid,
                   int pgid);

int64_t HHVM_FUNCTION(posix_setsid);

bool HHVM_FUNCTION(posix_setuid,
                   int uid);

String HHVM_FUNCTION(posix_strerror,
                     int errnum);

Variant HHVM_FUNCTION(posix_times);

Variant HHVM_FUNCTION(posix_ttyname,
                      const Variant& fd);

Variant HHVM_FUNCTION(posix_uname);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_POSIX_H_
