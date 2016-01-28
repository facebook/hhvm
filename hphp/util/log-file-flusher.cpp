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
#include "hphp/util/log-file-flusher.h"

#include <sys/types.h>
#include <unistd.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

int LogFileFlusher::DropCacheChunkSize = 1 << 20;

void LogFileFlusher::recordWriteAndMaybeDropCaches(int fd, int bytes) {
  int oldBytesWritten = m_bytesWritten.fetch_add(bytes);
  int newBytesWritten = oldBytesWritten + bytes;

  if (!(newBytesWritten > DropCacheChunkSize &&
        oldBytesWritten <= DropCacheChunkSize)) {
    return;
  }

  off_t offset = lseek(fd, 0, SEEK_CUR);
  if (offset > kDropCacheTail) {
    dropCache(fd, offset - kDropCacheTail);
  }

  m_bytesWritten = 0;
}

//////////////////////////////////////////////////////////////////////

}
