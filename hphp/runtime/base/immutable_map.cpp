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

#include "hphp/runtime/base/immutable_map.h"
#include "hphp/runtime/base/shared_variant.h"
#include "hphp/runtime/base/array_iterator.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

HOT_FUNC
ImmutableMap* ImmutableMap::Create(ArrayData* arr,
                                   bool unserializeObj) {
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
      ret->add(ret->m.m_num, it.first(), it.secondRef(), unserializeObj);
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
    (*(SharedVariant*)&buckets[i].val).~SharedVariant();
  }
  free(map);
}

HOT_FUNC
void ImmutableMap::addVal(int pos, int hash_pos,
                          CVarRef val, bool unserializeObj) {
  // NOTE: no check on duplication because we assume the original array has no
  // duplication
  Bucket* bucket = buckets() + pos;
  new (&bucket->val) SharedVariant(val, false, true, unserializeObj);
  int& hp = hash()[hash_pos];
  bucket->next = hp;
  hp = pos;
}

HOT_FUNC
void ImmutableMap::add(int pos, CVarRef key, CVarRef val, bool unserializeObj) {
  int64_t ikey;
  StringData* skey;
  int32_t hash;
  Bucket* b = buckets() + pos;

  switch (key.getType()) {
    case KindOfInt64: {
      hash = ikey = key.toInt64();
      b->setIntKey(ikey);
      break;
    }
    case KindOfString: {
      skey = StringData::GetStaticString(key.getStringData());
      goto static_case;
    }
    case KindOfStaticString: {
      skey = key.getStringData();
static_case:
      hash = skey->hash();
      b->setStrKey(skey, hash);
      break;
    }
    default: not_reached();
  }
  addVal(pos, hash & m.m_capacity_mask, val, unserializeObj);
}

#define STR_HASH(x)   (int32_t(x) | 0x80000000)

HOT_FUNC
int ImmutableMap::indexOf(const StringData* key) {
  strhash_t h = STR_HASH(key->hash());
  int bucket = hash()[h & m.m_capacity_mask];
  Bucket* b = buckets();
  while (bucket != -1) {
    Bucket* cand = &b[bucket];
    if (cand->hash() == h && (cand->skey == key || key->same(cand->skey))) {
      return bucket;
    }
    bucket = b[bucket].next;
  }
  return -1;
}

HOT_FUNC
int ImmutableMap::indexOf(int64_t key) {
  int bucket = hash()[key & m.m_capacity_mask];
  Bucket* b = buckets();
  while (bucket != -1) {
    if (b[bucket].hasIntKey() && key == b[bucket].ikey) {
      return bucket;
    }
    bucket = b[bucket].next;
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
}
