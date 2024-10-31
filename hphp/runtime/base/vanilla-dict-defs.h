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

#include "hphp/runtime/base/vanilla-dict.h"

#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-val.h"

#include "hphp/util/word-mem.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

inline void VanillaDict::scan(type_scan::Scanner& scanner) const {
  if (isZombie()) return;
  auto data = this->data();
  scanner.scan(*data, m_used * sizeof(*data));
}

inline void
VanillaDict::copyElmsNextUnsafe(VanillaDict* to, const VanillaDict* from,
                               uint32_t nElems) {
  static_assert(offsetof(VanillaDict, m_nextKI) + 8 == sizeof(VanillaDict),
                "Revisit this if VanillaDict layout changes");
  static_assert(sizeof(Elm) == 24, "");
  // Copy `m_nextKI' (8 bytes), data (oldUsed * 24), and optionally 24 more
  // bytes to make sure we can use bcopy32(), which rounds the length down to
  // 32-byte chunks. The additional bytes are guaranteed not to exceed the
  // space allocated for the array, because the hash table has at least 16
  // bytes, and when it is only 16 bytes (capacity = 3), we overrun the buffer
  // by only 16 bytes instead of 24.
  bcopy32_inline(&(to->m_nextKI), &(from->m_nextKI), sizeof(Elm) * nElems + 32);
}

extern int32_t* warnUnbalanced(VanillaDict*, size_t n, int32_t* ei);

inline bool VanillaDict::isTombstone(ssize_t pos) const {
  assertx(size_t(pos) <= m_used);
  return isTombstone(data()[pos].data.m_type);
}

ALWAYS_INLINE
const TypedValue* VanillaDict::getArrayElmPtr(ssize_t pos) const {
  assertx(validPos(pos));
  if (size_t(pos) >= m_used) return nullptr;
  auto& elm = data()[pos];
  return !isTombstone(elm.data.m_type) ? &elm.data : nullptr;
}

template <class K>
arr_lval VanillaDict::addLvalImpl(K k) {
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
