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

#ifndef incl_HPHP_APC_ARRAY_H_
#define incl_HPHP_APC_ARRAY_H_

#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/types.h"
#include "hphp/util/atomic.h"
#include "hphp/util/lock.h"
#include "hphp/util/hash.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class APCLocalArray;

/**
 * APCArray is a php-style array that can take strings and
 * ints as keys. We also store the order in which the elements
 * are inserted. Once an element is added, it can not be
 * removed.
 */
struct APCArray {
  // Entry point to create an APCArray of any kind
  static APCHandle* MakeShared(ArrayData* data,
                               bool inner,
                               bool unserializeObj);
  static APCHandle* MakeShared();

  static Variant MakeArray(APCHandle* handle);

  static void Delete(APCHandle* handle);

  static APCArray* fromHandle(APCHandle* handle) {
    assert(offsetof(APCArray, m_handle) == 0);
    return reinterpret_cast<APCArray*>(handle);
  }

  APCHandle* getHandle() {
    return &m_handle;
  }

  bool shouldCache() const {
    return m_handle.m_shouldCache;
  }

  //
  // Array API
  //

  size_t size() const {
    return isPacked() ? m_size : m.m_num;
  }

  unsigned capacity() const {
    return isPacked() ? m_size : m.m_capacity_mask + 1;
  }

  Variant getKey(ssize_t pos) const {
    if (isPacked()) {
      assert(static_cast<size_t>(pos) < m_size);
      return pos;
    }
    assert(static_cast<size_t>(pos) < m.m_num);
    return buckets()[pos].key->toLocal();
  }

  APCHandle* getValue(ssize_t pos) const {
    if (isPacked()) {
      assert(static_cast<size_t>(pos) < m_size);
      return vals()[pos];
    }
    assert(static_cast<size_t>(pos) < m.m_num);
    return buckets()[pos].val;
  }

  ssize_t getIndex(const StringData* key) const {
    return isPacked() ? -1 : indexOf(key);
  }

  ssize_t getIndex(int64_t key) const {
    if (isPacked()) {
      return (static_cast<uint64_t>(key) >= m_size) ? -1 : key;
    }
    return indexOf(key);
  }

  void incRef() { m_handle.incRef(); }
  void decRef() { m_handle.decRef(); }
  bool isPacked() const { return m_handle.isPacked(); }

private:
  explicit APCArray(size_t size) : m_handle(KindOfArray), m_size(size) {
    m_handle.setPacked();
  }
  explicit APCArray(unsigned int cap)
      : m_handle(KindOfArray) {
    m.m_capacity_mask = cap - 1;
    m.m_num = 0;
  }
  ~APCArray();

  APCArray(const APCArray&) = delete;
  APCArray& operator=(const APCArray&) = delete;

  void operator delete(void* ptr) { free(ptr); }

  struct Bucket {
    /** index of the next bucket, or -1 if the end of a chain */
    int next;
    /** the value of this bucket */
    APCHandle *key;
    APCHandle *val;
  };

  //
  // Create API
  //
  static APCHandle* MakeShared(ArrayData* data,
                               bool unserializeObj);
  static APCHandle* MakePackedShared(ArrayData* data,
                                     bool unserializeObj);

  void mustCache() { m_handle.m_shouldCache = true; }
  void setPacked() { m_handle.setPacked(); }

  //
  // Array internal API
  //
  void add(APCHandle *key, APCHandle *val);
  ssize_t indexOf(const StringData* key) const;
  ssize_t indexOf(int64_t key) const;

  /* index of the beginning of each hash chain */
  int *hash() const { return (int*)(this + 1); }
  /* buckets, stored in index order */
  Bucket* buckets() const { return (Bucket*)(hash() + m.m_capacity_mask + 1); }
  /* start of the data for packed array */
  APCHandle** vals() const { return (APCHandle**)(this + 1); }

private:
  friend struct APCHandle;

  APCHandle m_handle;
  union {
    // for map style arrays
    struct {
      unsigned int m_capacity_mask;
      unsigned int m_num;
    } m;
    // for packed arrays
    size_t m_size;
  };
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* incl_HPHP_APC_ARRAY_H_ */
