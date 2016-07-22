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

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
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
    always_assert(!"Unknown flag passed to pipe2 compatibility layer");
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

int fadvise_dontneed(int fd, off_t len) {
#if defined(__FreeBSD__) || defined(__APPLE__) || defined(_MSC_VER)
  return 0;
#else
  return posix_fadvise(fd, 0, len, POSIX_FADV_DONTNEED);
#endif
}

int advise_out(const std::string& fileName) {
  if (fileName.empty()) return -1;
  int fd = open(fileName.c_str(), O_RDONLY);
  if (fd == -1) return -1;
  int result = fadvise_dontneed(fd, 0);
  close(fd);
  return result;
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
