/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include <sys/types.h>
#include <grp.h>
#include <signal.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(posix_access,
                   const String& file,
                   int64_t mode = 0);

String HHVM_FUNCTION(posix_ctermid);

int64_t HHVM_FUNCTION(posix_get_last_error);

int64_t HHVM_FUNCTION(posix_errno);

String HHVM_FUNCTION(posix_getcwd);

int64_t HHVM_FUNCTION(posix_getegid);

int64_t HHVM_FUNCTION(posix_geteuid);

int64_t HHVM_FUNCTION(posix_getgid);

Variant HHVM_FUNCTION(posix_getgrgid,
                      int64_t gid);

Variant HHVM_FUNCTION(posix_getgrnam,
                      const String& name);

Variant HHVM_FUNCTION(posix_getgroups);

Variant HHVM_FUNCTION(posix_getlogin);

Variant HHVM_FUNCTION(posix_getpgid,
                      int64_t pid);

int64_t HHVM_FUNCTION(posix_getpgrp);

int64_t HHVM_FUNCTION(posix_getpid);

int64_t HHVM_FUNCTION(posix_getppid);

Variant HHVM_FUNCTION(posix_getpwnam,
                      const String& username);

Variant HHVM_FUNCTION(posix_getpwuid,
                      int64_t uid);

Variant HHVM_FUNCTION(posix_getrlimit);

Variant HHVM_FUNCTION(posix_getsid,
                      int64_t pid);

int64_t HHVM_FUNCTION(posix_getuid);

bool HHVM_FUNCTION(posix_initgroups,
                   const String& name,
                   int64_t base_group_id);

bool HHVM_FUNCTION(posix_isatty,
                   const Variant& fd);

bool HHVM_FUNCTION(posix_kill,
                   int64_t pid,
                   int64_t sig);

bool HHVM_FUNCTION(posix_mkfifo,
                   const String& pathname,
                   int64_t mode);

bool HHVM_FUNCTION(posix_mknod,
                   const String& pathname,
                   int64_t mode,
                   int64_t major = 0,
                   int64_t minor = 0);

bool HHVM_FUNCTION(posix_setegid,
                   int64_t gid);

bool HHVM_FUNCTION(posix_seteuid,
                   int64_t uid);

bool HHVM_FUNCTION(posix_setgid,
                   int64_t gid);

bool HHVM_FUNCTION(posix_setpgid,
                   int64_t pid,
                   int64_t pgid);

int64_t HHVM_FUNCTION(posix_setsid);

bool HHVM_FUNCTION(posix_setuid,
                   int64_t uid);

String HHVM_FUNCTION(posix_strerror,
                     int64_t errnum);

Variant HHVM_FUNCTION(posix_times);

Variant HHVM_FUNCTION(posix_ttyname,
                      const Variant& fd);

Variant HHVM_FUNCTION(posix_uname);

///////////////////////////////////////////////////////////////////////////////
}
