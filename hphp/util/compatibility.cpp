/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/compatibility.h"
#include "hphp/util/vdso.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#if defined(__APPLE__) || defined(__FreeBSD__)
char *strndup(const char* str, size_t len) {
  size_t str_len = strlen(str);
  if (len < str_len) {
    str_len = len;
  }
  char *result = (char*)malloc(str_len + 1);
  if (result == nullptr) {
    return nullptr;
  }
  memcpy(result, str, str_len);
  result[str_len] = '\0';
  return result;
}

int dprintf(int fd, const char *format, ...) {
  va_list ap;
  char *ptr = nullptr;
  int ret = 0;

  va_start(ap, format);
  vasprintf(&ptr, format, ap);
  va_end(ap);

   if (ptr) {
     ret = write(fd, ptr, strlen(ptr));
     free(ptr);
   }

   return ret;
}
#endif

int gettime(clockid_t which_clock, struct timespec *tp) {
#if defined(__APPLE__) || defined(__FreeBSD__)
  // XXX: OSX doesn't support realtime so we ignore which_clock
  struct timeval tv;
  int ret = gettimeofday(&tv, nullptr);
  tp->tv_sec = tv.tv_sec;
  tp->tv_nsec = tv.tv_usec * 1000;
  return ret;
#else
  static int vdso_usable =
    Util::Vdso::ClockGetTime(which_clock, tp);
  if (vdso_usable == 0)
    return Util::Vdso::ClockGetTime(which_clock, tp);
  return clock_gettime(which_clock, tp);
#endif
}

int64_t gettime_diff_us(const timespec &start, const timespec &end) {
  int64_t dsec = end.tv_sec - start.tv_sec;
  int64_t dnsec = end.tv_nsec - start.tv_nsec;
  return dsec * 1000000 + dnsec / 1000;
}

///////////////////////////////////////////////////////////////////////////////
}
