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

// Native helpers for the interpreter + JIT used to implement *IterInit* ops.
// These helpers return 1 if the base has any elements and 0 otherwise.
// (They would return a bool, but native method calls from the JIT produce GP
// register outputs, so we extend the return type to an int64_t.)
//
// If these helpers return 1, they set `val` (and `key`, for key-value iters)
// from the first key-value pair of the base.
//
// For non-local iters, if these helpers return 0, they also dec-ref the base.
//
// For the array helpers, first provide a baseConst flag to get an IterInit
// helper to call, then call it. This indirection lets us burn the appropriate
// helper into the JIT (where we know the baseConst flag statically). For
// objects, we don't need it because they have reference semantics and do not
// change identity.
using IterInitArr    = int64_t(*)(Iter*, ArrayData*, TypedValue*);
using IterInitArrKey = int64_t(*)(Iter*, ArrayData*, TypedValue*, TypedValue*);

IterInitArr    new_iter_array_helper(bool baseConst);
IterInitArrKey new_iter_array_key_helper(bool baseConst);

int64_t new_iter_object(ObjectData* obj, TypedValue* val, TypedValue* key);


// Native helpers for the interpreter + JIT used to implement *IterInit* ops.
// These helpers return 1 if the base has more elements and 0 otherwise.
// (As above, they return a logical bool which we extend to a GP register.)
//
// If these helpers return 1, they set `val` (and `key`, for key-value iters)
// from the next key-value pair of the base.
//
// For non-local iters, if these helpers return 0, they also dec-ref the base.
template<bool BaseConst>
NEVER_INLINE int64_t iter_next_array(Iter*, ArrayData*, TypedValue*);
template<bool BaseConst>
NEVER_INLINE int64_t iter_next_array_key(Iter*, ArrayData*, TypedValue*, TypedValue*);

NEVER_INLINE int64_t iter_next_object(ObjectData*, TypedValue*);
NEVER_INLINE int64_t iter_next_object_key(ObjectData*, TypedValue*, TypedValue*);

//////////////////////////////////////////////////////////////////////

}
