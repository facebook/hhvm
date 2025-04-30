/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <string>
#include <fcntl.h>
#include <sys/types.h>

#include <folly/portability/Fcntl.h>
#include <folly/portability/Unistd.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

int fadvise_dontneed(int fd, off_t len) {
  return posix_fadvise(fd, 0, len, POSIX_FADV_DONTNEED);
}

int advise_out(const std::string& fileName) {
  if (fileName.empty()) return -1;
  int fd = open(fileName.c_str(), O_RDONLY);
  if (fd == -1) return -1;
  int result = fadvise_dontneed(fd, 0);
  close(fd);
  return result;
}

///////////////////////////////////////////////////////////////////////////////
}
