/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2016 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef incl_HPHP_SNAPSHOT_H_
#define incl_HPHP_SNAPSHOT_H_

#include <stdio.h>
#include <string>
#include <utility>
#include <vector>

#include "hphp/runtime/base/type-string.h"
#include "hphp/util/logger.h"

namespace HPHP {

/*
 * Snapshot format
 *
 * The three sections of a serialized snapshot, and their current contents, are:
 *   1. Header - fixed-size metadata
 *   2. Index - all APC prime keys, and those values that always go into RAM
 *   3. Disk - all remaining APC prime values
 * When loading, the Index is fully parsed, but the Disk is just lazily mapped.
 */
struct SnapshotHeader {
  SnapshotHeader() = delete;  // Use factories below.
  static SnapshotHeader makeEmpty() { return SnapshotHeader(0, 0, 0, 0); }
  static SnapshotHeader makeValid(int64_t diskOffsetBytes, int64_t total) {
    return SnapshotHeader(kMagic, kLatestVersion, diskOffsetBytes, total);
  }
  bool isValid() const { return magic == kMagic; }
 private:
  SnapshotHeader(int64_t m, int64_t v, int64_t d, int64_t t)
      : magic(m), version(v), diskOffset(d), totalSize(t) {}
  static constexpr int64_t kMagic = 0xA31415926535C;
 public:
  static constexpr int64_t kLatestVersion = 1;
  const int64_t magic;
  const int64_t version;
  const int64_t diskOffset;  // Number of bytes from file start to Disk section.
  const int64_t totalSize;
};

}

#endif // incl_HPHP_SNAPSHOT_H_
