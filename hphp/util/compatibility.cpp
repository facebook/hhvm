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

#include "hphp/util/compatibility.h"
#include "hphp/util/assertions.h"
#include "hphp/util/logger.h"
#include "hphp/util/vdso.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

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

int pipe2(int pipefd[2], int flags) {
  if (flags & ~O_CLOEXEC) {
    Logger::Error("Unknown flag passed to pipe2 compatibility layer");
    abort();
  }

  if (pipe(pipefd) < 0) {
    return -1;
  }

  if (flags & O_CLOEXEC) {
    if (fcntl(pipefd[0], F_SETFD, FD_CLOEXEC) == -1 ||
        fcntl(pipefd[1], F_SETFD, FD_CLOEXEC) == -1) {
      close(pipefd[0]);
      close(pipefd[1]);
      return -1;
    }
  }

  return 0;
}
#endif

static int gettime_helper(clockid_t which_clock, struct timespec *tp) {
#if defined(__CYGWIN__) || defined(_MSC_VER)
  // let's bypass trying to load vdso
  return clock_gettime(which_clock, tp);
#elif defined(__APPLE__) || defined(__FreeBSD__)
  // XXX: OSX doesn't support realtime so we ignore which_clock
  struct timeval tv;
  int ret = gettimeofday(&tv, nullptr);
  tp->tv_sec = tv.tv_sec;
  tp->tv_nsec = tv.tv_usec * 1000;
  return ret;
#else
  static int vdso_usable = Vdso::ClockGetTime(which_clock, tp);
  if (vdso_usable == 0) {
    return Vdso::ClockGetTime(which_clock, tp);
  }
  return clock_gettime(which_clock, tp);
#endif
}

__thread int64_t s_extra_request_microseconds;
int gettime(clockid_t which_clock, struct timespec* tp) {
  auto ret = gettime_helper(which_clock, tp);
#ifdef CLOCK_THREAD_CPUTIME_ID
  if (which_clock == CLOCK_THREAD_CPUTIME_ID) {
    always_assert(tp->tv_nsec < 1000000000);

    tp->tv_sec += s_extra_request_microseconds / 1000000;
    auto res = tp->tv_nsec + (s_extra_request_microseconds % 1000000) * 1000;
    if (res > 1000000000) {
      res -= 1000000000;
      tp->tv_sec += 1;
    }
    tp->tv_nsec = res;
  }
#endif
  return ret;
}

int64_t gettime_diff_us(const timespec &start, const timespec &end) {
  int64_t dsec = end.tv_sec - start.tv_sec;
  int64_t dnsec = end.tv_nsec - start.tv_nsec;
  return dsec * 1000000 + dnsec / 1000;
}

int fadvise_dontneed(int fd, off_t len) {
#if defined(__FreeBSD__) || defined(__APPLE__) || defined(_MSC_VER)
  return 0;
#else
  return posix_fadvise(fd, 0, len, POSIX_FADV_DONTNEED);
#endif
}

#if defined(__CYGWIN__) || defined(_MSC_VER)
#include <windows.h>

// since we only support win 7+
// capturestackbacktrace is always available in kernel
int backtrace (void **buffer, int size) {
  USHORT frames;

  if (size <= 0) {
    return 0;
  }

  frames = CaptureStackBackTrace(0, (DWORD) size, buffer, nullptr);

  return (int) frames;
}

int dladdr(const void *addr, Dl_info *info) {
  MEMORY_BASIC_INFORMATION mem_info;
  char moduleName[MAX_PATH];

  if(!VirtualQuery(addr, &mem_info, sizeof(mem_info))) {
    return 0;
  }

  if(!GetModuleFileNameA(nullptr, moduleName, sizeof(moduleName))) {
    return 0;
  }

  info->dli_fname = (char *)(malloc(strlen(moduleName) + 1));
  strcpy((char *)info->dli_fname, moduleName);
  info->dli_fbase = mem_info.BaseAddress;
  info->dli_sname = nullptr;
  info->dli_saddr = (void *) addr;

  return 1;
}

#ifdef __CYGWIN__
#include <libintl.h>
// libbfd on cygwin is broken, stub dgettext to make linker unstupid
char * libintl_dgettext(const char *domainname, const char *msgid) {
  return dgettext(domainname, msgid);
}
#endif

#endif

///////////////////////////////////////////////////////////////////////////////
}
