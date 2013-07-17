/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_COLLECTION_DEF_H_
#define incl_HPHP_EXT_COLLECTION_DEF_H_

#include "hphp/runtime/ext/ext_collections.h"

namespace HPHP {

// Vector JIT helpers
inline int64_t c_Vector::iterInit(TypedValue* valOut) {
  if (UNLIKELY(0 >= m_size)) {
    return 0LL;
  }
  cellDup(m_data[0], *valOut);
  return 1LL;
}

inline int64_t c_Vector::iterInitK(TypedValue* valOut, TypedValue* keyOut) {
  if (UNLIKELY(0 >= m_size)) {
    return 0LL;
  }
  cellDup(m_data[0], *valOut);
  keyOut->m_data.num = 0;
  keyOut->m_type = KindOfInt64;
  return 1LL;
}

inline int64_t c_Vector::iterNext(ssize_t pos, TypedValue* valOut) {
  if (UNLIKELY(uint64_t(++pos) >= m_size)) {
    return 0LL;
  }
  cellDup(m_data[pos], *valOut);
  return pos;
}

inline int64_t c_Vector::iterNextK(
    ssize_t pos, TypedValue* valOut, TypedValue* keyOut) {
  if (UNLIKELY(uint64_t(++pos) >= m_size)) {
    return 0LL;
  }
  cellDup(m_data[pos], *valOut);
  keyOut->m_data.num = pos;
  keyOut->m_type = KindOfInt64;
  return pos;
}

// Map JIT helpers
inline int64_t c_Map::iterInit(TypedValue* valOut) {
  ssize_t key = iter_begin();
  if (UNLIKELY(key == 0)) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(key);
  cellDup(p->data, *valOut);
  return key;
}

inline int64_t c_Map::iterInitK(TypedValue* valOut, TypedValue* keyOut) {
  ssize_t key = iter_begin();
  if (UNLIKELY(key == 0)) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(key);
  cellDup(p->data, *valOut);
  if (p->hasStrKey()) {
    Variant v(p->skey);
    cellDup(*v.asTypedValue(), *keyOut);
  } else {
    keyOut->m_data.num = (int64_t)p->ikey;
    keyOut->m_type = KindOfInt64;
  }
  return key;
}

inline int64_t c_Map::iterNext(ssize_t key, TypedValue* valOut) {
  key = iter_next(key);
  if (UNLIKELY(key == 0)) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(key);
  cellDup(p->data, *valOut);
  return key;
}

inline int64_t c_Map::iterNextK(
    ssize_t key, TypedValue* valOut, TypedValue* keyOut) {
  key = iter_next(key);
  if (UNLIKELY(key == 0)) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(key);
  cellDup(p->data, *valOut);
  if (p->hasStrKey()) {
    Variant v(p->skey);
    cellDup(*v.asTypedValue(), *keyOut);
  } else {
    keyOut->m_data.num = (int64_t)p->ikey;
    keyOut->m_type = KindOfInt64;
  }
  return key;
}

// StableMap JIT helpers
inline int64_t c_StableMap::iterInit(TypedValue* valOut) {
  ssize_t key = iter_begin();
  if (UNLIKELY(key == 0)) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(key);
  cellDup(p->data, *valOut);
  return key;
}

inline int64_t c_StableMap::iterInitK(TypedValue* valOut, TypedValue* keyOut) {
  ssize_t key = iter_begin();
  if (UNLIKELY(key == 0)) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(key);
  cellDup(p->data, *valOut);
  if (p->hasStrKey()) {
    Variant v(p->skey);
    cellDup(*v.asTypedValue(), *keyOut);
  } else {
    keyOut->m_data.num = (int64_t)p->ikey;
    keyOut->m_type = KindOfInt64;
  }
  return key;
}

inline int64_t c_StableMap::iterNext(ssize_t key, TypedValue* valOut) {
  key = iter_next(key);
  if (UNLIKELY(key == 0)) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(key);
  cellDup(p->data, *valOut);
  return key;
}

inline int64_t c_StableMap::iterNextK(
    ssize_t key, TypedValue* valOut, TypedValue* keyOut) {
  key = iter_next(key);
  if (UNLIKELY(key == 0)) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(key);
  cellDup(p->data, *valOut);
  if (p->hasStrKey()) {
    Variant v(p->skey);
    cellDup(*v.asTypedValue(), *keyOut);
  } else {
    keyOut->m_data.num = (int64_t)p->ikey;
    keyOut->m_type = KindOfInt64;
  }
  return key;
}

// Set JIT helpers
inline int64_t c_Set::iterInit(TypedValue* valOut) {
  ssize_t key = iter_begin();
  if (UNLIKELY(key == 0)) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(key);
  cellDup(p->data, *valOut);
  return key;
}

inline int64_t c_Set::iterInitK(TypedValue* valOut, TypedValue* keyOut) {
  ssize_t key = iter_begin();
  if (UNLIKELY(key == 0)) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(key);
  cellDup(p->data, *valOut);
  keyOut->m_type = KindOfUninit;
  return key;
}

inline int64_t c_Set::iterNext(ssize_t key, TypedValue* valOut) {
  key = iter_next(key);
  if (UNLIKELY(key == 0)) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(key);
  cellDup(p->data, *valOut);
  return key;
}

inline int64_t c_Set::iterNextK(
    ssize_t key, TypedValue* valOut, TypedValue* keyOut) {
  key = iter_next(key);
  if (UNLIKELY(key != 0)) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(key);
  cellDup(p->data, *valOut);
  keyOut->m_type = KindOfUninit;
  return key;
}

// Pair JIT helpers
inline int64_t c_Pair::iterInit(TypedValue* valOut) {
  cellDup(getElms()[0], *valOut);
  return 1LL;
}

inline int64_t c_Pair::iterInitK(TypedValue* valOut, TypedValue* keyOut) {
  cellDup(getElms()[0], *valOut);
  keyOut->m_data.num = 0;
  keyOut->m_type = KindOfInt64;
  return 1LL;
}

inline int64_t c_Pair::iterNext(ssize_t pos, TypedValue* valOut) {
  if (uint64_t(++pos) >= uint64_t(2)) {
    return 0LL;
  }
  cellDup(getElms()[pos], *valOut);
  return pos;
}

inline int64_t c_Pair::iterNextK(
    ssize_t pos, TypedValue* valOut, TypedValue* keyOut) {
  if (uint64_t(++pos) >= uint64_t(2)) {
    return 0LL;
  }
  cellDup(getElms()[pos], *valOut);
  keyOut->m_data.num = pos;
  keyOut->m_type = KindOfInt64;
  return pos;
}

}

#endif // incl_HPHP_EXT_COLLECTION_DEF_H_
