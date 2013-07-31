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

#include <mutex>
#include <condition_variable>

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

  // operators for merging shorthand
  SiteAllocations &operator+=(size_t bytes) {
    m_count++;
    m_bytes += bytes;
    return *this;
  }
  SiteAllocations &operator-=(size_t bytes) {
    m_count--;
    m_bytes -= bytes;
    return *this;
  }

  SiteAllocations &operator+=(const SiteAllocations &allocs) {
    m_count += allocs.m_count;
    m_bytes += allocs.m_bytes;
    return *this;
  }
  SiteAllocations &operator-=(const SiteAllocations &allocs) {
    m_count -= allocs.m_count;
    m_bytes -= allocs.m_bytes;
    return *this;
  }
};

// Allocation data for each stack trace. pprof wants both what is currently
// being used and what was allocated across the lifetime of the heap.
struct ProfileDump {
  void clear() {
    m_currentlyAllocated.clear();
    m_accumAllocated.clear();
  }

  void addAlloc(size_t size, const ProfileStackTrace &trace) {
    m_currentlyAllocated[trace] += size;
    m_accumAllocated[trace] += size;
  }
  void removeAlloc(size_t size, const ProfileStackTrace &trace) {
    auto &current = m_currentlyAllocated[trace];
    current -= size;
    assert(current.m_count >= 0 && current.m_bytes >= 0);
  }

  std::string toPProfFormat() const;

  template<typename F>
  void forEachAddress(F fun) const {
    for (const auto &data : m_accumAllocated) {
      for (const SrcKey &sk : data.first) {
        fun(sk);
      }
    }
  }

  // merge operation: takes another dump and adds all of its data.
  // used for global dumps that require logging from multiple VM
  // threads
  ProfileDump &operator+=(const ProfileDump &dump) {
    for (const auto &pair : dump.m_currentlyAllocated) {
      m_currentlyAllocated[pair.first] += pair.second;
    }
    for (const auto &pair : dump.m_accumAllocated) {
      m_accumAllocated[pair.first] += pair.second;
    }
    return *this;
  }

private:
  std::map<ProfileStackTrace, SiteAllocations> m_currentlyAllocated;
  std::map<ProfileStackTrace, SiteAllocations> m_accumAllocated;
};

// Static controller for requesting and fetching profile dumps. The pprof
// server will place requests for dumps, and the VM threads will give
// their dumps to the controller if they satisfy the currently-active
// request. The pprof server will come and fetch the profile later, and if
// it needs to wait for a request to finish, it will.
struct ProfileController {
  // request API
  static bool requestNext();
  static bool requestNextURL(const std::string &url);
  static bool requestGlobal();
  static void cancelRequest();

  // give API
  static void offerProfile(const ProfileDump &dump);

  // get API
  static ProfileDump waitForProfile();
};

}

#endif // incl_HPHP_PROFILE_DUMP_H_
