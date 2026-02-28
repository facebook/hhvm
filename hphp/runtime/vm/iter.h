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

#include <array>
#include <cstdint>

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/base/vanilla-keyset.h"
#include "hphp/runtime/base/unaligned-typed-value.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/iter-args-flags.h"
#include "hphp/util/type-scan.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * The iterator stack frame representation used for "foreach" loops over arrays
 * implemented by *IterInit* and *IterNext* bytecodes.
 *
 * Iterators store their base in a separate "base" local. By default, this base
 * local is immutable during the iteration. This is achieved by having an
 * unnamed local holding a reference, distinct from a local that might have been
 * specified by the programmer. HHBBC can do an analysis that deduplicates these
 * locals if it is safe to do so. Specifically: either the base does not change
 * at all, or the current key is assigned a new value in the loop. The latter
 * case is represented by the lack of the "BaseConst" flag. The purpose of this
 * optimization is to try to keep local bases at a refcount of 1, so that they
 * won't be COWed by the "set the current key" type of mutating operations.
 * Apparently, this pattern is somewhat common...
 *
 * Iterators store their current and end positions. The representation of these
 * positions depends on the array type, bespoke layout, whether iterating by
 * value or key/value pair, and whether the base local is immutable. The correct
 * flavor of iteration helpers must be used consistently across iteration steps.
 */
struct alignas(16) Iter {
  Iter() = delete;
  ~Iter() = delete;

  ssize_t getPos() const {
    return m_pos;
  }
  ssize_t getEnd() const {
    return m_end;
  }
  void setPos(ssize_t newPos) {
    m_pos = newPos;
  }

  // It's valid to call end() on a killed iter, but the iter is otherwise dead.
  // In debug builds, this method will overwrite the iterator with garbage.
  void kill();

  static TypedValue extractBase(TypedValue base, const Class* ctx);

  // JIT helpers used for specializing iterators.
  static constexpr size_t posOffset() {
    return offsetof(Iter, m_pos);
  }
  static constexpr size_t posSize() {
    return sizeof(m_pos);
  }
  static constexpr size_t endOffset() {
    return offsetof(Iter, m_end);
  }
  static constexpr size_t endSize() {
    return sizeof(m_end);
  }

  // Current position.
  // For the pointer iteration types, we use the appropriate pointers instead.
  union {
    size_t m_pos;
    UnalignedTypedValue* m_unaligned_elm;
    VanillaDictElm* m_dict_elm;
    VanillaKeysetElm* m_keyset_elm;
  };
  union {
    size_t m_end;
    UnalignedTypedValue* m_unaligned_end;
    VanillaDictElm* m_dict_end;
    VanillaKeysetElm* m_keyset_end;
  };

  // These elements are always referenced via the base local. (If we weren't
  // using pointer iteration, we would track elements by index, not by pointer,
  // but GC would still work.)
  TYPE_SCAN_IGNORE_FIELD(m_unaligned_elm);
  TYPE_SCAN_IGNORE_FIELD(m_dict_elm);
  TYPE_SCAN_IGNORE_FIELD(m_keyset_elm);
  TYPE_SCAN_IGNORE_FIELD(m_unaligned_end);
  TYPE_SCAN_IGNORE_FIELD(m_dict_end);
  TYPE_SCAN_IGNORE_FIELD(m_keyset_end);
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Native iteration helpers for arrays.
 *
 * Use iter_select() to pick the right specialization based on IterArgsFlags.
 *
 * iter_init_array():
 *   If the base is empty, return 0. Otherwise, initialize the iterator with
 *   the starting position of the base and return 1.
 * iter_next_array():
 *   Advance the iterator. If it has reached the end, free it and return 0.
 *   Otherwise, return 1.
 * (They would return a bool, but native method calls from the JIT produce GP
 *  register outputs, so we extend the return type to an int64_t.)
 *
 * iter_get_key_array():
 *   Get the key at the position of the iterator. Does not IncRef the key.
 * iter_get_value_array():
 *   Get the value at the position of the iterator. Does not IncRef the value.
 */
template<bool BaseConst, bool WithKeys>
NEVER_INLINE TypedValue iter_get_key_array(ArrayData*, ssize_t) noexcept;
template<bool BaseConst, bool WithKeys>
NEVER_INLINE TypedValue iter_get_value_array(ArrayData*, ssize_t) noexcept;
template<bool BaseConst, bool WithKeys>
NEVER_INLINE int64_t iter_init_array(Iter*, ArrayData*) noexcept;
template<bool BaseConst, bool WithKeys>
NEVER_INLINE int64_t iter_next_array(Iter*, ArrayData*) noexcept;

#define iter_select(fn, flags) (                 \
  has_flag(flags, IterArgs::Flags::BaseConst)    \
    ? has_flag(flags, IterArgs::Flags::WithKeys) \
      ? fn<true, true>                           \
      : fn<true, false>                          \
    : fn<false, true>                            \
)

/*
 * Native iteration helpers for objects implementing the Iterator interface.
 *
 * Unlike the array APIs, these methods do not operate on an iterator slot,
 * but instead call methods on the base object implementing the Iterator
 * interface.
 *
 * Since iter_get_key_object() and iter_get_value_object() call methods to
 * obtain the key and value, they have the "produces reference" semantics,
 * i.e. the returned values already hold a reference.
 */
NEVER_INLINE TypedValue iter_get_key_object(ObjectData*);
NEVER_INLINE TypedValue iter_get_value_object(ObjectData*);
NEVER_INLINE int64_t iter_init_object(ObjectData*);
NEVER_INLINE int64_t iter_next_object(ObjectData*);

//////////////////////////////////////////////////////////////////////

}
