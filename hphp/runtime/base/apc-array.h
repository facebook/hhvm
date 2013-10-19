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

#include "hphp/runtime/base/types.h"
#include "hphp/util/lock.h"
#include "hphp/util/hash.h"
#include "hphp/util/atomic.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class APCVariant;

/**
 * APCArray is a php-style array that can take strings and
 * ints as keys. We also store the order in which the elements
 * are inserted. Once an element is added, it can not be
 * removed.
 */
struct APCArray {
  int indexOf(const StringData* key);
  int indexOf(int64_t key);

  APCVariant* getKeyIndex(int index) {
    assert(index < size());
    return buckets()[index].key;
  }

  APCVariant* getValIndex(int index) {
    assert(index < size());
    return buckets()[index].val;
  }

  unsigned size() const {
    return m.m_num;
  }

  unsigned capacity() const {
    return m.m_capacity_mask + 1;
  }

  size_t getStructSize() {
    size_t size = sizeof(APCArray) +
                  sizeof(Bucket) * m.m_num +
                  sizeof(int) * (m.m_capacity_mask + 1);
    return size;
  }

  static APCArray* Create(ArrayData* arr, bool unserializeObj,
                                bool& shouldCache);
  static void Destroy(APCArray* im);

private:
  struct Bucket {
    /** index of the next bucket, or -1 if the end of a chain */
    int next;
    /** the value of this bucket */
    APCVariant *key;
    APCVariant *val;
  };

private:
  APCArray() {}
  ~APCArray() {}
  void add(int pos, APCVariant *key, APCVariant *val);

  /** index of the beginning of each hash chain */
  int *hash() const { return (int*)(this + 1); }
  /** buckets, stored in index order */
  Bucket* buckets() const { return (Bucket*)(hash() + m.m_capacity_mask + 1); }

private:
  union {
    struct {
      unsigned int m_capacity_mask;
      unsigned int m_num;
    } m;
    APCVariant* align_dummy;
  };
};

/*
 * APCPackedArray is the APC version of a PackedArray,
 * i.e. an array with int keys in order 0..size-1.  We only store
 * the values.
 */
struct APCPackedArray {
  explicit APCPackedArray(size_t size) : m_size(size) {}
  ~APCPackedArray();
  APCVariant** vals() { return (APCVariant**)(this + 1); }
  void *operator new(size_t sz, int num) {
    assert(sz == sizeof(APCPackedArray));
    return malloc(sizeof(APCPackedArray) + num * sizeof(APCVariant*));
  }
  void operator delete(void* ptr) { free(ptr); }
  // just to keep the compiler happy; used if the constructor throws
  void operator delete(void* ptr, int num) { free(ptr); }
  size_t size() const { return m_size; }

private:
  size_t const m_size;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* incl_HPHP_APC_ARRAY_H_ */
