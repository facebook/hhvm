/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_COMPATIBILITY_H_
#define incl_HPHP_COMPATIBILITY_H_

#include <cstdint>
#include <time.h>
#include <unistd.h>

#include <folly/Singleton.h>

#include "hphp/util/portability.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

template <typename T>
std::shared_ptr<T> getSingleton() {
  return folly::Singleton<T>::try_get();
}

#define PHP_DIR_SEPARATOR '/'

extern __thread int64_t s_extra_request_microseconds;

#if defined(__APPLE__) || defined(__FreeBSD__)
char *strndup(const char* str, size_t len);
int dprintf(int fd, ATTRIBUTE_PRINTF_STRING const char *format, ...)
  ATTRIBUTE_PRINTF(2,3);
typedef int clockid_t;
int pipe2(int pipefd[2], int flags);
#endif

int gettime(clockid_t which_clock, struct timespec *tp);
int64_t gettime_diff_us(const timespec &start, const timespec &end);

/*
 * Drop the cached pages associated with the file from the file system
 * cache, if supported on our build target.
 *
 * Returns: -1 on error, setting errno according to posix_fadvise
 * values.
 */
int fadvise_dontneed(int fd, off_t len);

#if defined(__CYGWIN__) || defined(_MSC_VER)

typedef struct {
  const char *dli_fname;
  void *dli_fbase;
  const char *dli_sname;
  void *dli_saddr;
} Dl_info;

int dladdr(const void *addr, Dl_info *info);
int backtrace (void **buffer, int size);

#endif

//////////////////////////////////////////////////////////////////////

}

#endif
