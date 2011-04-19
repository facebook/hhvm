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

#include <runtime/base/shared/shared_variant.h>
#include <runtime/base/shared/immutable_map.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ImmutableMap::ImmutableMap(int num) : m_curPos(0) {
  if (num <= 0) {
    num = 1;
  }
  int pow_2 = 1;
  while (num >>= 1) pow_2++;
  int capacity = 1 << pow_2;
  m_capacity_mask = capacity - 1;
  m_hash = (int*)malloc(sizeof(int) * capacity);
  for (int i = 0; i < capacity; i++) m_hash[i] = -1;
  m_buckets = (Bucket*)malloc(sizeof(Bucket) * capacity);
}

ImmutableMap::~ImmutableMap() {
  for (int i = 0; i < m_curPos; i++) {
    m_buckets[i].key->decRef();
    m_buckets[i].val->decRef();
  }
  free(m_buckets);
  free(m_hash);
}

void ImmutableMap::add(SharedVariant *key, SharedVariant *val) {
  // NOTE: no check on duplication because we assume the original array has no
  // duplication
  int pos = m_curPos++;
  ASSERT(pos <= (int)m_capacity_mask);
  m_buckets[pos].key = key;
  m_buckets[pos].val = val;
  if (key->is(KindOfInt64)) {
    size_t hash_pos = (int64)(key->intData()) & (int64)m_capacity_mask;
    m_buckets[pos].next = m_hash[hash_pos];
    m_hash[hash_pos] = pos;
  } else {
    ASSERT(key->is(KindOfString) || key->is(KindOfStaticString));
    size_t hash_pos = key->getStringData()->hash() & (int64)m_capacity_mask;
    m_buckets[pos].next = m_hash[hash_pos];
    m_hash[hash_pos] = pos;
  }
}

int ImmutableMap::indexOf(StringData* key) {
  int64 hash = key->hash();
  size_t hash_pos = hash & (int64)m_capacity_mask;
  for (int bucket = m_hash[hash_pos]; bucket != -1;
       bucket = m_buckets[bucket].next) {
    if ((m_buckets[bucket].key->is(KindOfString) ||
         m_buckets[bucket].key->is(KindOfStaticString)) &&
        key->same(m_buckets[bucket].key->getStringData())) {
      return bucket;
    }
  }
  return -1;
}

int ImmutableMap::indexOf(int64 key) {
  size_t hash_pos = (int64)key & (int64)m_capacity_mask;
  for (int bucket = m_hash[hash_pos]; bucket != -1;
      bucket = m_buckets[bucket].next) {
    if (m_buckets[bucket].key->is(KindOfInt64) &&
        m_buckets[bucket].key->intData() == key) {
      return bucket;
    }
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
}
