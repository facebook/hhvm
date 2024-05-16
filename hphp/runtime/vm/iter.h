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
#include "hphp/runtime/base/unaligned-typed-value.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/util/type-scan.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct Iter;

enum class IterNextIndex : uint8_t {
  VanillaVec = 0,
  VanillaDict,
  Array,

  // JIT-only "pointer iteration", designed for good specialized code-gen.
  // In pointer iteration, the iterator has a pointer directly into the base.
  //
  // We only use this mode if all the following conditions are met:
  //  - The array is guaranteed to be unchanged during iteration
  //  - The array is a VanillaDict or VanillaVec storing unaligned tvs
  //  - (For dicts) The array is free of tombstones
  VanillaDictPointer,
  VanillaVecPointer,

  // Helpers specific to bespoke array-likes.
  StructDict,
};

// For iterator specialization, we pack all the information we need to generate
// specialized code in a single byte so that we can check it in one comparison.
//
// This byte should be 0 for unspecialized iterators, as created by calling the
// normal IterImpl constructor instead of using a specialized initializer.
struct IterSpecialization {
  // Returns a generic (unspecialized) IterSpecialization value.
  static IterSpecialization generic() {
    IterSpecialization result;
    result.as_byte = 0;
    assertx(!result.specialized);
    return result;
  }

  ArrayKeyTypes keyTypes() const {
    assertx(specialized);
    return ArrayKeyTypes::FromBits(key_types);
  }
  void setKeyTypes(ArrayKeyTypes keyTypes) {
    assertx(specialized);
    key_types = keyTypes.toBits();
  }

  union {
    uint8_t as_byte;
    struct {
      uint8_t key_types: 4;  // bit encoding of ArrayKeyTypes

      // When we JIT a specialized iterator, we set `specialized` to true,
      bool specialized: 1;
      bool bespoke: 1;

      // 2 free bits
      bool padding: 2;
    };
  };
};

// Debugging output.
std::string show(IterSpecialization type);

/*
 * Iterator over an array, a collection, or an object implementing the Hack
 * Iterator interface. This iterator is used by the JIT and its usage is
 * mediated through the "Iter" wrapper below.
 *
 * By default, iterators inc-ref their base to ensure that it won't be mutated
 * during the iteration. HHBBC can do an analysis that marks certain iterators
 * as "local" iterators, which means that their base only changes in certain
 * controlled ways during iteration. (Specifically: either the base does not
 * change at all, or the current key is assigned a new value in the loop.)
 *
 * For local iterators, the base is kept in a frame local and passed to the
 * iterator on each iteration. Local iterators are never used for objects,
 * since we can't constrain writes to them in this way.
 *
 * The purpose of the local iter optimization is to try to keep local bases at
 * a refcount of 1, so that they won't be COWed by the "set the current key"
 * type of mutating operations. Apparently, this pattern is somewhat common...
 */
struct alignas(16) IterImpl {
  /*
   * Constructors.  Note that sometimes IterImpl objects are created
   * without running their C++ constructor.  (See new_iter_array.)
   */
  IterImpl() = delete;

  // Pass a non-NULL ad to checkInvariants iff this iterator is local.
  // These invariants hold as long as the iterator hasn't yet reached the end.
  bool checkInvariants(const ArrayData* ad) const;

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

  // JIT helpers used for specializing iterators.
  static constexpr size_t posOffset() {
    return offsetof(IterImpl, m_pos);
  }
  static constexpr size_t posSize() {
    return sizeof(m_pos);
  }
  static constexpr size_t endOffset() {
    return offsetof(IterImpl, m_end);
  }
  static constexpr size_t endSize() {
    return sizeof(m_end);
  }

private:
  template<bool BaseConst>
  friend int64_t new_iter_array(Iter*, ArrayData*, TypedValue*);
  template<bool BaseConst>
  friend int64_t new_iter_array_key(Iter*, ArrayData*, TypedValue*,
                                    TypedValue*);
  friend int64_t new_iter_object(Iter*, ObjectData* obj,
                                 TypedValue* val, TypedValue* key);

public:
  // Current position.
  // For the pointer iteration types, we use the appropriate pointers instead.
  union {
    size_t m_pos;
    UnalignedTypedValue* m_unaligned_elm;
    VanillaDictElm* m_mixed_elm;
  };
  union {
    size_t m_end;
    UnalignedTypedValue* m_unaligned_end;
    VanillaDictElm* m_mixed_end;
  };

  // These elements are always referenced via the base local.
  // of this iterator or in a local. (If we weren't using pointer iteration, we
  // would track elements by index, not by pointer, but GC would still work.)
  TYPE_SCAN_IGNORE_FIELD(m_mixed_end);
  TYPE_SCAN_IGNORE_FIELD(m_unaligned_end);
  TYPE_SCAN_IGNORE_FIELD(m_mixed_elm);
  TYPE_SCAN_IGNORE_FIELD(m_unaligned_elm);
};

///////////////////////////////////////////////////////////////////////////////

/*
 * The iterator API used by the interpreter and the JIT. This API is relatively
 * limited, because there are only two ways to interact with iterators in Hack:
 *  1. In a "foreach" loop, using the *IterInit* / *IterNext* bytecodes.
 *  2. As a delegated generator ("yield from").
 *
 * (*IterInit* here refers to {IterInit, IterInitK, IterInit, IterInitK}).
 *
 * The methods exposed here should be sufficient to implement both kinds of
 * iterator behavior. To speed up "foreach" loops, we also provide helpers
 * implementing *IterInit* / *IterNext* through helpers below.
 *
 * These helpers are faster than using the Iter class's methods directly
 * because they do one vtable lookup on the array type and then execute the
 * advance / bounds check / output key-value sequence based on that lookup,
 * rather than doing a separate vtable lookup for each step.
 *
 * NOTE: If you initialize an iterator using the faster init helpers, you MUST
 * use the faster next helpers for IterNext ops. That's because the helpers may
 * make iterators that use pointer iteration, which Iter::next doesn't handle.
 * doesn't handle. This invariant is checked in debug builds.
 *
 * In practice, this constraint shouldn't be a problem, because we always use
 * the helpers to do IterNext. That's true both in the interpreter and the JIT.
 */
struct alignas(16) Iter {
  Iter() = delete;
  ~Iter() = delete;

  // Validates iterator base and extract the underlying array or iterator.
  // Assumes the base is not an array.
  static TypedValue extractBase(TypedValue base, const Class* ctx);

  // It's valid to call end() on a killed iter, but the iter is otherwise dead.
  // In debug builds, this method will overwrite the iterator with garbage.
  void kill() { m_iter.kill(); }

private:
  // Used to implement the separate helper functions below. These functions
  // peek into the Iter and directly manipulate m_iter's fields.
  friend IterImpl* unwrap(Iter*);

  IterImpl m_iter;
};

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
