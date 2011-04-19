/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __HPHP_IMMUTABLE_MAP_H__
#define __HPHP_IMMUTABLE_MAP_H__

#include <runtime/base/types.h>
#include <util/lock.h>
#include <util/hash.h>
#include <util/atomic.h>

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
  ImmutableMap(int num);
  ~ImmutableMap();

  void add(SharedVariant *key, SharedVariant *val);

  int indexOf(StringData* key);
  int indexOf(int64 key);

  SharedVariant* getKeyIndex(int index) {
    ASSERT(index < size());
    return m_buckets[index].key;
  }

  SharedVariant* getValIndex(int index) {
    ASSERT(index < size());
    return m_buckets[index].val;
  }

  int size() {
    return m_curPos;
  }

  size_t getStructSize() {
    size_t size = sizeof(ImmutableMap) +
                  sizeof(Bucket) * (m_capacity_mask + 1) +
                  sizeof(int) * (m_capacity_mask + 1);
    return size;
  }

private:
  struct Bucket {
    /** index of the next bucket, or -1 if the end of a chain */
    int next;
    /** the value of this bucket */
    SharedVariant *key;
    SharedVariant *val;
  };
  /** index of the beginning of each hash chain */
  int *m_hash;
  /** buckets, stored in index order */
  Bucket* m_buckets;
  int m_curPos;
  unsigned int m_capacity_mask;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_IMMUTABLE_MAP_H__ */
