/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/base/mixed-array.h"

#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/data-walker.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"

#include "hphp/util/stacktrace-profiler.h"
#include "hphp/util/word-mem.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

inline void MixedArray::scan(type_scan::Scanner& scanner) const {
  if (isZombie()) return;
  auto data = this->data();
  scanner.scan(*data, m_used * sizeof(*data));
}

inline void
MixedArray::copyElmsNextUnsafe(MixedArray* to, const MixedArray* from,
                               uint32_t nElems) {
  static_assert(offsetof(MixedArray, m_nextKI) + 8 == sizeof(MixedArray),
                "Revisit this if MixedArray layout changes");
  static_assert(sizeof(Elm) == 24, "");
  // Copy `m_nextKI' (8 bytes), data (oldUsed * 24), and optionally 24 more
  // bytes to make sure we can use bcopy32(), which rounds the length down to
  // 32-byte chunks. The additional bytes are guaranteed not to exceed the
  // space allocated for the array, because the hash table has at least 16
  // bytes, and when it is only 16 bytes (capacity = 3), we overrun the buffer
  // by only 16 bytes instead of 24.
  bcopy32_inline(&(to->m_nextKI), &(from->m_nextKI), sizeof(Elm) * nElems + 32);
}

extern int32_t* warnUnbalanced(MixedArray*, size_t n, int32_t* ei);

inline bool MixedArray::isTombstone(ssize_t pos) const {
  assertx(size_t(pos) <= m_used);
  return isTombstone(data()[pos].data.m_type);
}

ALWAYS_INLINE
TypedValue MixedArray::getElmKey(const Elm& e) {
  if (e.hasIntKey()) {
    return make_tv<KindOfInt64>(e.ikey);
  }
  auto str = e.skey;
  if (str->isRefCounted()) {
    str->rawIncRefCount();
    return make_tv<KindOfString>(str);
  }
  return make_tv<KindOfPersistentString>(str);
}

ALWAYS_INLINE
void MixedArray::getArrayElm(ssize_t pos,
                            TypedValue* valOut,
                            TypedValue* keyOut) const {
  assertx(size_t(pos) < m_used);
  auto& elm = data()[pos];
  tvDup(elm.data, *valOut);
  tvCopy(getElmKey(elm), *keyOut);
}

ALWAYS_INLINE
void MixedArray::getArrayElm(ssize_t pos, TypedValue* valOut) const {
  assertx(size_t(pos) < m_used);
  auto& elm = data()[pos];
  tvDup(elm.data, *valOut);
}

ALWAYS_INLINE
const TypedValue* MixedArray::getArrayElmPtr(ssize_t pos) const {
  assertx(validPos(pos));
  if (size_t(pos) >= m_used) return nullptr;
  auto& elm = data()[pos];
  return !isTombstone(elm.data.m_type) ? &elm.data : nullptr;
}

ALWAYS_INLINE
TypedValue MixedArray::getArrayElmKey(ssize_t pos) const {
  assertx(validPos(pos));
  if (size_t(pos) >= m_used) return make_tv<KindOfUninit>();
  auto& elm = data()[pos];
  if (isTombstone(elm.data.m_type)) return make_tv<KindOfUninit>();
  return getElmKey(elm);
}

inline ArrayData* MixedArray::addVal(int64_t ki, TypedValue data) {
  assertx(!exists(ki));
  assertx(!isFull());
  auto h = hash_int64(ki);
  auto ei = findForNewInsert(h);
  auto e = allocElm(ei);
  e->setIntKey(ki, h);
  mutableKeyTypes()->recordInt();
  if (ki >= m_nextKI && m_nextKI >= 0) m_nextKI = ki + 1;
  tvDup(data, e->data);
  // TODO(#3888164): should avoid needing these KindOfUninit checks.
  if (UNLIKELY(e->data.m_type == KindOfUninit)) {
    e->data.m_type = KindOfNull;
  }
  return this;
}

inline ArrayData* MixedArray::addVal(StringData* key, TypedValue data) {
  assertx(!exists(key));
  assertx(!isFull());
  return addValNoAsserts(key, data);
}

inline ArrayData* MixedArray::addValNoAsserts(StringData* key, TypedValue data) {
  assertx(data.m_type != KindOfUninit);
  strhash_t h = key->hash();
  auto ei = findForNewInsert(h);
  auto e = allocElm(ei);
  e->setStrKey(key, h);
  mutableKeyTypes()->recordStr(key);
  tvDup(data, e->data);
  return this;
}

template <class K>
arr_lval MixedArray::addLvalImpl(K k) {
  assertx(!isFull());
  auto p = insert(k);
  if (!p.found) tvWriteNull(p.tv);
  return arr_lval { this, &p.tv };
}

//////////////////////////////////////////////////////////////////////

/*
 * Extra space that gets prepended to uncounted arrays.
 */
ALWAYS_INLINE size_t uncountedAllocExtra(const ArrayData* ad, bool apc_tv) {
  auto const extra = (apc_tv ? sizeof(APCTypedValue) : 0) +
                     (ad->hasStrKeyTable() ? sizeof(StrKeyTable) : 0);
  return (extra + 15) & ~15ull;
}

//////////////////////////////////////////////////////////////////////

}
