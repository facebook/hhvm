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

#include "hphp/util/current-executable.h"
#include <unistd.h>
#include <limits.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef __FreeBSD__
#include <sys/sysctl.h>
#endif

namespace HPHP {

std::string current_executable_path() {
#ifdef __linux__
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  return std::string(result, (count > 0) ? count : 0);
#elif defined(__APPLE__)
  char result[PATH_MAX];
  uint32_t size = sizeof(result);
  uint32_t success = _NSGetExecutablePath(result, &size);
  return std::string(result, (success == 0) ? size : 0);
#elif defined(__FreeBSD__)
  char result[PATH_MAX];
  size_t size = sizeof(result);
  int mib[4];

  mib[0] = CTL_KERN;
  mib[1] = KERN_PROC;
  mib[2] = KERN_PROC_PATHNAME;
  mib[3] = -1;

  if (sysctl(mib, 4, result, &size, nullptr, 0) < 0) {
      return std::string();
  }
  return std::string(result, (size > 0) ? size : 0);
#else
  // XXX: How do you do this on your platform?
  return std::string();
#endif
}

std::string current_executable_directory() {
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
  std::string path = current_executable_path();
  return path.substr(0, path.find_last_of("/"));
#else
  // XXX: How do you do this on your platform?
  return std::string();
#endif
}

}
