/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/shared/thread_shared_variant.h>
#include <runtime/base/shared/immutable_map.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ImmutableMap::~ImmutableMap() {
  for (int i = 0; i < m_curPos; i++) {
    m_buckets[i].key->decRef();
    m_buckets[i].val->decRef();
  }
  free(m_buckets);
  free(m_hash);
}

static string_data_hash g_hash;
static string_data_equal g_equal;

void ImmutableMap::add(ThreadSharedVariant *key, ThreadSharedVariant *val) {
  // NOTE: no check on duplication because we assume the original array has no
  // duplication
  int pos = m_curPos++;
  ASSERT(pos < m_capacity);
  m_buckets[pos].key = key;
  m_buckets[pos].val = val;
  if (key->is(KindOfInt64)) {
    size_t hash_pos = (size_t)(key->intData()) % m_capacity;
    m_buckets[pos].next = m_hash[hash_pos];
    m_hash[hash_pos] = pos;
  } else {
    ASSERT(key->is(KindOfString));
    size_t hash_pos = g_hash(key->getStringData()) % m_capacity;
    m_buckets[pos].next = m_hash[hash_pos];
    m_hash[hash_pos] = pos;
  }
}

int ImmutableMap::indexOf(StringData* key) {
  size_t hash_pos = g_hash(key) % m_capacity;
  for (int bucket = m_hash[hash_pos]; bucket != -1;
       bucket = m_buckets[bucket].next) {
    if (m_buckets[bucket].key->is(KindOfString) &&
        g_equal(m_buckets[bucket].key->getStringData(), key)) {
      return bucket;
    }
  }
  return -1;
}

int ImmutableMap::indexOf(int64 key) {
  size_t hash_pos = (size_t)key % m_capacity;
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
