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
#include "hphp/runtime/base/stat-cache.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sstream>
#include <vector>

#include <folly/MapUtil.h>
#include <folly/portability/Unistd.h>

#include "hphp/util/trace.h"
#include "hphp/util/text-util.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/tracing.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(stat);

static std::string statToString(const struct stat* buf) {
  std::ostringstream os;
  os << "struct stat {";
  os <<   "dev="                << buf->st_dev                 << ", ";
  os <<   "ino="                << buf->st_ino                 << ", ";
  os <<   "mode=0"  << std::oct << buf->st_mode    << std::dec << ", ";
  os <<   "nlink="              << buf->st_nlink               << ", ";
  os <<   "uid="                << buf->st_uid                 << ", ";
  os <<   "gid="                << buf->st_gid                 << ", ";
  os <<   "rdev="               << buf->st_rdev                << ", ";
  os <<   "size="               << buf->st_size                << ", ";
  os <<   "blksize="            << buf->st_blksize             << ", ";
  os <<   "blocks="             << buf->st_blocks              << ", ";
  os <<   "atime="              << buf->st_atime               << ", ";
  os <<   "mtime="              << buf->st_mtime               << ", ";
  os <<   "ctime="              << buf->st_ctime;
  os << "}";
  return os.str();
}

int statSyscall(const std::string& path, struct stat* buf) {
  int ret = ::stat(path.c_str(), buf);
  if (ret == 0) {
    TRACE(5, "StatCache: stat '%s' %s\n",
             path.c_str(), statToString(buf).c_str());
  } else {
    TRACE(5, "StatCache: stat '%s' --> error\n", path.c_str());
  }
  return ret;
}

int lstatSyscall(const std::string& path, struct stat* buf) {
  int ret = ::lstat(path.c_str(), buf);
  if (ret == 0) {
    TRACE(5, "StatCache: lstat '%s' %s\n",
             path.c_str(), statToString(buf).c_str());
  } else {
    TRACE(5, "StatCache: lstat '%s' --> error\n", path.c_str());
  }
  return ret;
}

std::string realpathLibc(const char* path) {
  tracing::BlockNoTrace _{"realpath"};
  char buf[PATH_MAX];

  std::string ret;
  if (!::realpath(path, buf)) {
    TRACE(5, "StatCache: realpath('%s') --> error\n", path);
    return ret;
  }
  TRACE(5, "StatCache: realpath('%s') --> '%s'\n", path, buf);
  ret = buf;
  return ret;
}

}
