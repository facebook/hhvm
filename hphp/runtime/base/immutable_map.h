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

#ifndef incl_HPHP_IMMUTABLE_MAP_H_
#define incl_HPHP_IMMUTABLE_MAP_H_

#include "hphp/runtime/base/types.h"
#include "hphp/util/lock.h"
#include "hphp/util/hash.h"
#include "hphp/util/atomic.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SharedVariant;
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

  SharedVariant* getKeyIndex(int index) {
    assert(index < size());
    return buckets()[index].key;
  }

  SharedVariant* getValIndex(int index) {
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
    size_t size = sizeof(ImmutableMap) +
                  sizeof(Bucket) * m.m_num +
                  sizeof(int) * (m.m_capacity_mask + 1);
    return size;
  }

  static ImmutableMap* Create(ArrayData* arr,
                              bool unserializeObj,
                              bool& shouldCache);
  static void Destroy(ImmutableMap* im);
private:
  ImmutableMap() {}
  ~ImmutableMap() {}
  void add(int pos, SharedVariant *key, SharedVariant *val);

  struct Bucket {
    /** index of the next bucket, or -1 if the end of a chain */
    int next;
    /** the value of this bucket */
    SharedVariant *key;
    SharedVariant *val;
  };
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
