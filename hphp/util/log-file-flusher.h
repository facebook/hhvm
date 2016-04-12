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
#ifndef incl_HPHP_LOG_FILE_FLUSHER_H_
#define incl_HPHP_LOG_FILE_FLUSHER_H_

#include <cstdio>
#include <atomic>
#include <unistd.h>

#include "hphp/util/compatibility.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct LogFileFlusher {
  LogFileFlusher() {}
  virtual ~LogFileFlusher() {}

  void recordWriteAndMaybeDropCaches(int fd, int bytes);
  inline void recordWriteAndMaybeDropCaches(FILE* f, int bytes) {
    recordWriteAndMaybeDropCaches(fileno(f), bytes);
  }

  enum {
    kDropCacheTail = 1 * 1024 * 1024
  };

  static int DropCacheChunkSize;

private: // For testing
  virtual void dropCache(int fd, off_t len) { fadvise_dontneed(fd, len); }

private:
  std::atomic<int> m_bytesWritten{0};
};

//////////////////////////////////////////////////////////////////////

}

#endif
