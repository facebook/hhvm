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

#ifndef incl_HPHP_IMMUTABLE_MAP_H_
#define incl_HPHP_IMMUTABLE_MAP_H_

#include "hphp/runtime/base/types.h"
#include "hphp/util/lock.h"
#include "hphp/util/hash.h"
#include "hphp/util/atomic.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * an immutable map is a php-style array that can take strings and
 * ints as keys. the map also stores the order in which the elements
 * are inserted. once an element is added to the map, it can not be
 * removed.
 */
class ImmutableMap {
public:
  int indexOf(const StringData* key);
  int indexOf(int64_t key);

  Variant getKey(int index) {
    assert(index < size());
    Bucket* b = &buckets()[index];
    if (b->hasIntKey()) {
      return b->ikey;
    } else {
      return b->skey;
    }
  }

  SharedVariant* getValue(int index) {
    assert(index < size());
    return (SharedVariant*)&buckets()[index].val;
  }

  unsigned size() const {
    return m.m_num;
  }

  unsigned capacity() const {
    return m.m_capacity_mask + 1;
  }

  size_t getStructSize() {
    size_t size = sizeof(ImmutableMap) +
                  sizeof(Bucket) * m.m_num +
                  sizeof(int) * (m.m_capacity_mask + 1);
    return size;
  }

  static ImmutableMap* Create(ArrayData* arr,
                              bool unserializeObj);
  static void Destroy(ImmutableMap* im);

  struct Bucket {
    /** index of the next bucket, or -1 if the end of a chain */
    int next;
    /* similar to HphpArray::Elm */
    union {
      int64_t     ikey;
      StringData* skey;
    };
    // cannot declare SharedVariant here because of cyclic header
    // includes
    TypedValueAux val;
    bool hasStrKey() const {
      return val.hash() != 0;
    }
    bool hasIntKey() const {
      return val.hash() == 0;
    }
    int32_t hash() const {
      return val.hash();
    }
    void setStrKey(StringData* k, strhash_t h) {
      skey = k;
      val.hash() = int32_t(h) | 0x80000000;
    }
    void setIntKey(int64_t k) {
      ikey = k;
      val.hash() = 0;
    }
  };

private:
  ImmutableMap() {}
  ~ImmutableMap() {}
  void addVal(int pos, int hash_pos, CVarRef val, bool unserializeObj);
  void add(int pos, CVarRef key, CVarRef val, bool unserializeObj);

  /** index of the beginning of each hash chain */
  int *hash() const { return (int*)(this + 1); }
  /** buckets, stored in index order */
  Bucket* buckets() const { return (Bucket*)(hash() + m.m_capacity_mask + 1); }
  union {
    struct {
      unsigned int m_capacity_mask;
      unsigned int m_num;
    } m;
    SharedVariant* align_dummy;
  };
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* incl_HPHP_IMMUTABLE_MAP_H_ */
