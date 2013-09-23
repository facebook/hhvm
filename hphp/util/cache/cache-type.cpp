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

#include "hphp/util/cache/cache-type.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cstdint>
#include <string>

#include "folly/FileUtil.h"
#include "folly/Format.h"
#include "folly/ScopeGuard.h"
#include "hphp/util/logger.h"
#include "hphp/util/cache/magic-numbers.h"

namespace HPHP {

using folly::readFull;
using folly::format;
using std::string;

CacheType::CacheType() {}
CacheType::~CacheType() {}

bool CacheType::isNewCache(const std::string& filename) {
  int fd = open(filename.c_str(), O_RDONLY);

  if (fd < 0) {
    Logger::Error(format("Unable to open {}: {}",
                         filename, folly::errnoStr(errno)).str());
    return false;
  }

  SCOPE_EXIT{ close(fd); };

  uint64_t magic;
  ssize_t ret = readFull(fd, &magic, sizeof(magic));

  if (ret != sizeof(magic)) {
    Logger::Info(format("Can't read magic number from {}",
                        filename, folly::errnoStr(errno)).str());
    return false;
  }

  return magic == kCacheFileMagic;
}

}  // namespace HPHP
