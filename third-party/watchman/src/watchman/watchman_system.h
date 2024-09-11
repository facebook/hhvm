/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef WATCHMAN_SYSTEM_H
#define WATCHMAN_SYSTEM_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <folly/portability/SysTypes.h>
#include "watchman/config.h"

// This header plays tricks with posix IO functions and
// can result in ambiguous overloads on Windows if io.h
// is included before this header, so we pull it in early.
#include <folly/portability/Unistd.h>

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS 1

#define _ALLOW_KEYWORD_MACROS
#ifndef __cplusplus
#define inline __inline
#endif

// Tell windows.h not to #define min/max
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define EX_USAGE 1
#include <errno.h>
#include <process.h> // @manual
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef ptrdiff_t ssize_t;

#define snprintf _snprintf
char* dirname(char* path);

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

char* realpath(const char* filename, char* target);

#define O_DIRECTORY _O_OBTAIN_DIR
#define O_CLOEXEC _O_NOINHERIT
#define O_NOFOLLOW 0 /* clowny, but there's no translation */

#ifdef __cplusplus
}
#endif

#endif // WIN32

#ifdef WATCHMAN_FACEBOOK_INTERNAL
#include "common/base/BuildInfo.h"
#undef PACKAGE_VERSION
#undef WATCHMAN_BUILD_INFO
#define PACKAGE_VERSION BuildInfo_kTimeISO8601
#define WATCHMAN_BUILD_INFO BuildInfo_kUpstreamRevision
#endif

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/stat.h>
#if HAVE_SYS_INOTIFY_H
#include <sys/inotify.h>
#endif
#if HAVE_SYS_EVENT_H
#include <sys/event.h> // @manual
#endif
#if HAVE_PORT_H
#include <port.h> // @manual
#endif
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#ifndef _WIN32
#include <grp.h>
#include <libgen.h>
#endif
#include <inttypes.h>
#include <limits.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <sys/un.h>
#endif
#include <fcntl.h>
#if defined(__linux__) && !defined(O_CLOEXEC)
#define O_CLOEXEC 02000000 /* set close_on_exec, from asm/fcntl.h */
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef _WIN32
#include <poll.h>
#include <sys/wait.h>
#endif
#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#ifndef _WIN32
#include <pwd.h>
#include <sys/uio.h>
#include <sysexits.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#if defined(__clang__)
#if __has_feature(address_sanitizer)
#define WATCHMAN_ASAN 1
#endif
#elif defined(__GNUC__) &&                                             \
    (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ >= 5)) && \
    __SANITIZE_ADDRESS__
#define WATCHMAN_ASAN 1
#endif

#ifndef WATCHMAN_ASAN
#define WATCHMAN_ASAN 0
#endif

#ifdef HAVE_CORESERVICES_CORESERVICES_H
#include <CoreServices/CoreServices.h> // @manual
#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070
#define HAVE_FSEVENTS 0
#else
#define HAVE_FSEVENTS 1
#endif
#endif

// We make use of constructors to glue together modules
// without maintaining static lists of things in the build
// configuration.  These are helpers to make this work
// more portably
#ifdef _WIN32
#define w_ctor_fn_type(sym) void sym()
// Define a helper struct and its constructor; the constructor
// will call the function symbol we desire.  Also emit an
// instance of this struct as a global.  It will be triggered
// prior to main() being invoked.
#define w_ctor_fn_reg(sym)            \
  static struct w_paste1(sym, _reg) { \
    w_paste1(sym, _reg)() {           \
      sym();                          \
    }                                 \
  }                                   \
  w_paste1(sym, _reg_inst);

#else
#define w_ctor_fn_type(sym) __attribute__((constructor)) void sym()
#define w_ctor_fn_reg(sym) /* not needed */
#endif

/* sane, reasonably large filename size that we'll use
 * throughout; POSIX seems to define smallish buffers
 * that seem risky */
#define WATCHMAN_NAME_MAX 4096

// rpmbuild may enable fortify which turns on
// warn_unused_result on a number of system functions.
// This gives us a reasonably clean way to suppress
// these warnings when we're using stack protection.
#if __USE_FORTIFY_LEVEL > 0
#define ignore_result(x)    \
  do {                      \
    __typeof__(x) _res = x; \
    (void)_res;             \
  } while (0)
#elif _MSC_VER >= 1400
#define ignore_result(x) \
  do {                   \
    int _res = (int)x;   \
    (void)_res;          \
  } while (0)
#else
#define ignore_result(x) x
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
// Not explicitly exported on Darwin, so we get to define it.
extern char** environ;
#endif

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:et:
 */
