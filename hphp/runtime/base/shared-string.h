/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_CPP_BASE_SHARED_SHARED_STRING_H_
#define incl_HPHP_CPP_BASE_SHARED_SHARED_STRING_H_

#include "hphp/runtime/base/atomic-shared-ptr.h"
#include "hphp/runtime/base/types.h"
#include <tbb/concurrent_hash_map.h>
#include <tbb/atomic.h>
#include "hphp/util/atomic.h"
#include "hphp/util/hash.h"
#include "hphp/util/hash-map-typedefs.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
/**
 * Interned immutable strings sharable across threads.
 */
class SharedStringData {
public:
  explicit SharedStringData(const std::string &data);
  void incAtomicCount() const {
    m_count.fetch_and_increment();
  }
  int decAtomicCount() const;
  void atomicRelease();
  const std::string &getString() const;

  typedef tbb::concurrent_hash_map<std::string, SharedStringData*> InternMap;
  static void Create(InternMap::accessor &acc, const std::string &data);
protected:
  mutable tbb::atomic<int> m_count;
  const std::string m_data;
  static InternMap s_intern;
};

class SharedString : public AtomicSharedPtr<SharedStringData> {
public:
  SharedString() {}
  /* implicit */ SharedString(SharedStringData *px)
    : AtomicSharedPtr<SharedStringData>(px) {}
  /* implicit */ SharedString(const SharedString &src)
    : AtomicSharedPtr<SharedStringData>(src) {}
  /* implicit */ SharedString(const std::string &data) {
    operator=(data);
  }
  /* implicit */ SharedString(const char *data) {
    operator=(data);
  }
  SharedString &operator=(const std::string &data);
  SharedString &operator=(const char *data) {
    return operator=(std::string(data));
  }
};

struct shared_string_eq {
  bool operator()(const SharedString &s1, const SharedString &s2) const {
    // Because pointers and strings are 1-1, can just compare the ptr.
    return s1.get() == s2.get();
  }
};

struct shared_string_hash {
  size_t operator()(const SharedString &s) const {
    // Because pointers and strings are 1-1, can just hash the ptr.
    return hash_int64((int64_t)s.get());
  }
};

template<typename T>
class hphp_shared_string_map :
    public hphp_hash_map<SharedString, T, shared_string_hash,
                         shared_string_eq> {
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_CPP_BASE_SHARED_SHARED_STRING_H_
