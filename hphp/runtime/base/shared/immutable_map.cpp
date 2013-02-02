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
#include <runtime/base/array/array_iterator.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

HOT_FUNC
ImmutableMap* ImmutableMap::Create(ArrayData* arr,
                                   bool unserializeObj,
                                   bool &shouldCache) {
  int num = arr->size();
  int cap = num > 2 ? Util::roundUpToPowerOfTwo(num) : 2;

  ImmutableMap* ret = (ImmutableMap*)malloc(sizeof(ImmutableMap) +
                                            sizeof(int) * cap +
                                            sizeof(Bucket) * num);

  ret->m.m_capacity_mask = cap - 1;
  ret->m.m_num = 0;
  for (int i = 0; i < cap; i++) ret->hash()[i] = -1;

  try {
    for (ArrayIter it(arr); !it.end(); it.next()) {
      SharedVariant* key = SharedVariant::Create(it.first(), false, true,
                                                 unserializeObj);
      SharedVariant* val = SharedVariant::Create(it.secondRef(), false, true,
                                                 unserializeObj);
      if (val->m_shouldCache) shouldCache = true;
      ret->add(ret->m.m_num, key, val);
      ++ret->m.m_num;
    }
  } catch (...) {
    Destroy(ret);
    throw;
  }

  return ret;
}

HOT_FUNC
void ImmutableMap::Destroy(ImmutableMap* map) {
  Bucket* buckets = map->buckets();
  for (int i = 0; i < map->m.m_num; i++) {
    buckets[i].key->decRef();
    buckets[i].val->decRef();
  }
  free(map);
}

HOT_FUNC
void ImmutableMap::add(int pos, SharedVariant *key, SharedVariant *val) {
  // NOTE: no check on duplication because we assume the original array has no
  // duplication
  Bucket* bucket = buckets() + pos;
  bucket->key = key;
  bucket->val = val;
  int hash_pos =
    (key->is(KindOfInt64) ?
     key->intData() : key->getStringData()->hash()) & m.m_capacity_mask;

  int& hp = hash()[hash_pos];
  bucket->next = hp;
  hp = pos;
}

HOT_FUNC
int ImmutableMap::indexOf(const StringData* key) {
  strhash_t h = key->hash();
  int bucket = hash()[h & m.m_capacity_mask];
  Bucket* b = buckets();
  while (bucket != -1) {
    if (!b[bucket].key->is(KindOfInt64) &&
        key->same(b[bucket].key->getStringData())) {
      return bucket;
    }
    bucket = b[bucket].next;
  }
  return -1;
}

HOT_FUNC
int ImmutableMap::indexOf(int64 key) {
  int bucket = hash()[key & m.m_capacity_mask];
  Bucket* b = buckets();
  while (bucket != -1) {
    if (b[bucket].key->is(KindOfInt64) &&
        key == b[bucket].key->intData()) {
      return bucket;
    }
    bucket = b[bucket].next;
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
}
