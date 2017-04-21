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

#ifndef incl_HPHP_SNAPSHOT_LOADER_H_
#define incl_HPHP_SNAPSHOT_LOADER_H_

#include <stdio.h>
#include <string>
#include <utility>
#include <vector>

#include <folly/String.h>

#include "hphp/runtime/ext/apc/snapshot.h"

namespace HPHP {

struct ConcurrentTableSharedStore;

/*
 * Used to map+load the content of a snapshot during system startup.
 * Usage: Initialize (probing file format), then 'load'.
 */
struct SnapshotLoader {
  // Returns false if unable to read a valid header.
  bool tryInitializeFromFile(const char* filename);
  // Loads the contents from the snapshot (and leaves the Disk mapped).
  void load(ConcurrentTableSharedStore& s);
  // Evict the file-backed memory from OS page cache.
  void adviseOut();
 private:
  const SnapshotHeader& header() const {
    return *reinterpret_cast<const SnapshotHeader*>(m_begin);
  }

  // Deserialization primitives for snapshot format.
  folly::StringPiece readString() {
    auto size = read32();
    auto s = m_cur;
    m_cur += size + 1; // \0
    return folly::StringPiece(s, size);
  }
  int32_t read32() { return read<int32_t>(); }
  int64_t read64() { return read<int64_t>(); }
  template<class T> const T& read() {
    m_cur += sizeof(T);
    return reinterpret_cast<const T*>(m_cur)[-1];
  }

  const char* m_begin{nullptr};
  const char* m_cur{nullptr};
  int64_t m_size{-1};
  int m_fd{-1};
};

}

#endif // incl_HPHP_SNAPSHOT_LOADER_H_
