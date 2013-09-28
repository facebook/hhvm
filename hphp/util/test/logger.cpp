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

#include "hphp/util/logger.h"
#include <gtest/gtest.h>

namespace HPHP {

class MockLogFileFlusher : public LogFileFlusher {
public:
  MockLogFileFlusher() : last_flush(0) {}
  off_t last_flush;
protected:
  void dropCache(int fd, off_t off) {
    last_flush = off;
  }
};

static void doWrite(LogFileFlusher* flusher, int fd, int bytes) {
  char* buf = new char[bytes];
  write(fd, buf, bytes);
  delete[] buf;
  flusher->recordWriteAndMaybeDropCaches(fd, bytes);
}

TEST(LogFileFlusherTest, flush) {
  MockLogFileFlusher flusher;

  LogFileFlusher::DropCacheChunkSize = 1 << 19;

  ASSERT_EQ(LogFileFlusher::kDropCacheTail, 1 << 20);

  char tmpl[] = "/tmp/LogFileFlusherTest.XXXXXX";
  int fd = mkstemp(tmpl);
  ASSERT_NE(fd, 0);
  unlink (tmpl);

  // 100 bytes written, not enough to flush
  doWrite(&flusher, fd, 100);
  EXPECT_EQ(flusher.last_flush, 0);

  // 512KB written, but file is not 1 MB large yet so no fadvise
  doWrite(&flusher, fd, (1 << 19));
  EXPECT_EQ(flusher.last_flush, 0);

  // Another 512KB, after one more byte a fadvise will be called
  doWrite(&flusher, fd, (1 << 19));
  EXPECT_EQ(flusher.last_flush, 0);

  // fadvise called
  doWrite(&flusher, fd, 1);
  EXPECT_EQ(flusher.last_flush, 101);

  // Almost ready for another flush
  doWrite(&flusher, fd, (1 << 19));
  EXPECT_EQ(flusher.last_flush, 101);

  // Another flush occurs
  doWrite(&flusher, fd, 1);
  EXPECT_EQ(flusher.last_flush, (1 << 19) + 100 + 2);
}
}

