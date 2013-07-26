/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_PROFILE_DUMP_H_
#define incl_HPHP_PROFILE_DUMP_H_

#include "hphp/runtime/vm/srckey.h"

namespace HPHP {

// A StackTrace is represented by SrcKeys, which uniquely identify logical
// source locations. The lowermost stack frame recorded should go in the
// lowest index of the vector.
typedef std::vector<SrcKey> ProfileStackTrace;

// pprof format requires us to keep track of the number of live objects
// (or total in the accumulative case), and the number of bytes they occupy.
struct SiteAllocations {
  size_t m_count;
  size_t m_bytes;
};

// Allocation data for each stack trace. pprof wants both what is currently
// being used and what was allocated across the lifetime of the heap.
struct ProfileDump {
  void clear() {
    m_currentlyAllocated.clear();
    m_accumAllocated.clear();
  }

  void addAlloc(size_t size, const ProfileStackTrace &trace) {
    auto &current = m_currentlyAllocated[trace];
    current.m_count++;
    current.m_bytes += size;
    auto &accum = m_accumAllocated[trace];
    accum.m_count++;
    accum.m_bytes += size;
  }
  void removeAlloc(size_t size, const ProfileStackTrace &trace) {
    auto &current = m_currentlyAllocated[trace];
    current.m_count--;
    current.m_bytes -= size;
    assert(current.m_count >= 0 && current.m_bytes >= 0);
  }

private:
  std::map<ProfileStackTrace, SiteAllocations> m_currentlyAllocated;
  std::map<ProfileStackTrace, SiteAllocations> m_accumAllocated;
};

}

#endif // incl_HPHP_PROFILE_DUMP_H_
