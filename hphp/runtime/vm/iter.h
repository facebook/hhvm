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
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/base/unaligned-typed-value.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/util/type-scan.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct Iter;

enum class IterTypeOp { NonLocal, LocalBaseConst, LocalBaseMutable };

enum class IterNextIndex : uint8_t {
  VanillaVec = 0,
  ArrayMixed,
  Array,
  Object,

  // JIT-only "pointer iteration", designed for good specialized code-gen.
  // In pointer iteration, the iterator has a pointer directly into the base.
  //
  // We only use this mode if all the following conditions are met:
  //  - The array is guaranteed to be unchanged during iteration
  //  - The array is a VanillaDict (a dict or a darray) or VanillaVec storing unaligned tvs
  //  - (For dicts) The array is free of tombstones
  ArrayMixedPointer,
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
  enum BaseType : uint8_t { Vec = 0, Dict, kNumBaseTypes };
  enum KeyTypes : uint8_t { ArrayKey = 0, Int, Str, StaticStr, kNumKeyTypes };

  // Returns a generic (unspecialized) IterSpecialization value.
  static IterSpecialization generic() {
    IterSpecialization result;
    result.as_byte = 0;
    assertx(!result.specialized);
    return result;
  }

  union {
    uint8_t as_byte;
    struct {
      // `base_type` and `key_types` are bit encodings of the enums above.
      uint8_t base_type: 1;
      uint8_t key_types: 2;

      // When we JIT a specialized iterator, we set `specialized` to true,
      bool specialized: 1;
      bool bespoke: 1;

      // 3 free bits. Maybe we'll need a 2-bit enum for the layout?
      bool padding: 3;
    };
  };
};

// Debugging output.
std::string show(IterSpecialization type);
std::string show(IterSpecialization::BaseType type);
std::string show(IterSpecialization::KeyTypes type);

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
struct IterImpl {
  enum NoInc { noInc = 0 };
  enum Local { local = 0 };

  /*
   * Constructors.  Note that sometimes IterImpl objects are created
   * without running their C++ constructor.  (See new_iter_array.)
   */
  IterImpl() = delete;
  explicit IterImpl(const ArrayData* data);
  IterImpl(const ArrayData* data, NoInc) {
    setArrayData<false>(data);
  }
  IterImpl(const ArrayData* data, Local) {
    setArrayData<true>(data);
  }
  explicit IterImpl(ObjectData* obj);
  IterImpl(ObjectData* obj, NoInc);

  // Destructor
  ~IterImpl();

  // Pass a non-NULL ad to checkInvariants iff this iterator is local.
  // These invariants hold as long as the iterator hasn't yet reached the end.
  bool checkInvariants(const ArrayData* ad = nullptr) const;

  explicit operator bool() { return !end(); }

  // Returns true if we've reached the end. endHelper is used for iterators
  // over objects implementing the Iterator interface.
  bool end() const {
    if (UNLIKELY(!hasArrayData())) return endHelper();
    return getArrayData() == nullptr || m_pos == m_end;
  }
  bool endHelper() const;

  // Advance the iterator's position. Assumes that end() is false. nextHelper
  // is used for iterators over objects implementing the Iterator interface.
  void next() {
    assertx(checkInvariants());
    if (UNLIKELY(!hasArrayData())) return nextHelper();
    m_pos = getArrayData()->iter_advance(m_pos);
  }
  void nextHelper();

  bool nextLocal(const ArrayData* ad) {
    assertx(checkInvariants(ad));
    m_pos = ad->iter_advance(m_pos);
    return m_pos == m_end;
  }

  // Return the key at the current position. firstHelper is used for Objects.
  // This method and its variants inc-ref the key before returning it.
  Variant first() {
    if (UNLIKELY(!hasArrayData())) return firstHelper();
    return getArrayData()->getKey(m_pos);
  }
  Variant firstHelper();

  // TypedValue versions of first. Used by the JIT iterator helpers.
  // These methods do NOT inc-ref the key before returning it.
  TypedValue nvFirst() const {
    return getArrayData()->nvGetKey(m_pos);
  }
  TypedValue nvFirstLocal(const ArrayData* ad) const {
    assertx(getArrayData() == nullptr);
    return ad->nvGetKey(m_pos);
  }

  // Return the value at the current position. firstHelper is used for Objects.
  // This method and its variants inc-ref the value before returning it.
  Variant second();

  /*
   * Get the value at the current iterator position, without refcount ops.
   *
   * If called when iterating an Iterable object the secondVal() will fatal.
   */
  TypedValue secondVal() const;

  // TypedValue versions of second. Used by the JIT iterator helpers.
  // These methods do NOT inc-ref the value before returning it.
  TypedValue nvSecond() const {
    return getArrayData()->nvGetVal(m_pos);
  }
  TypedValue nvSecondLocal(const ArrayData* ad) const {
    assertx(getArrayData() == nullptr);
    return ad->nvGetVal(m_pos);
  }

  // This method returns null for local iterators, and for non-local iterators
  // with an empty array base. It must be checked in end() for this reason.
  bool hasArrayData() const {
    return !((intptr_t)m_data & objectBaseTag());
  }

  const ArrayData* getArrayData() const {
    assertx(hasArrayData());
    return m_data;
  }
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

  IterNextIndex getHelperIndex() {
    return m_nextHelperIdx;
  }

  ObjectData* getObject() const {
    assertx(!hasArrayData());
    return (ObjectData*)((intptr_t)m_obj & ~objectBaseTag());
  }

  // Used by native code and by the JIT to pack the m_typeFields components.
  static uint32_t packTypeFields(IterNextIndex index) {
    return static_cast<uint32_t>(index) << 24;
  }
  static uint32_t packTypeFields(
      IterNextIndex index, IterSpecialization spec, uint16_t layout) {
    return static_cast<uint32_t>(index) << 24 |
           static_cast<uint32_t>(spec.as_byte) << 16 |
           static_cast<uint32_t>(layout);
  }

  // JIT helpers used for specializing iterators.
  static constexpr size_t baseOffset() {
    return offsetof(IterImpl, m_data);
  }
  static constexpr size_t baseSize() {
    return sizeof(m_data);
  }
  static constexpr size_t typeOffset() {
    return offsetof(IterImpl, m_typeFields);
  }
  static constexpr size_t typeSize() {
    return sizeof(m_typeFields);
  }
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

  // When we specialize an iterator, we must *set* all m_type components (so as
  // to be compatible with native helpers) but we only need to check this byte.
  static constexpr size_t specializationOffset() {
    return offsetof(IterImpl, m_specialization);
  }

  // ObjectData bases have this additional bit set; ArrayData bases do not.
  static constexpr intptr_t objectBaseTag() {
    return 0b1;
  }

private:
  template<IterTypeOp Type>
  friend int64_t new_iter_array(Iter*, ArrayData*, TypedValue*);
  template<IterTypeOp Type>
  friend int64_t new_iter_array_key(Iter*, ArrayData*, TypedValue*,
                                    TypedValue*);
  template<bool HasKey, bool Local>
  friend int64_t iter_next_packed_pointer(
    Iter*, TypedValue*, TypedValue*, ArrayData*);
  template<bool HasKey, bool Local>
  friend int64_t iter_next_mixed_pointer(
    Iter*, TypedValue*, TypedValue*, ArrayData*);

  template <bool incRef = true>
  void arrInit(const ArrayData* arr);

  template <bool incRef>
  void objInit(ObjectData* obj);

  // Set all IterImpl fields for iteration over an array:
  //  - m_data is either the array, or null (for local iterators).
  //  - The type fields union is set based on the array type.
  //  - m_pos and m_end are set based on its virtual iter helpers.
  template <bool Local = false>
  void setArrayData(const ArrayData* ad) {
    assertx((intptr_t(ad) & objectBaseTag()) == 0);
    assertx(!Local || ad);
    m_data = Local ? nullptr : ad;
    setArrayNext(IterNextIndex::Array);
    if (ad != nullptr) {
      if (ad->isVanillaVec()) {
        setArrayNext(IterNextIndex::VanillaVec);
      } else if (ad->isVanillaDict()) {
        setArrayNext(IterNextIndex::ArrayMixed);
      }
      m_pos = ad->iter_begin();
      m_end = ad->iter_end();
    }
  }

  // Set all IterImpl fields for iteration over an object:
  //  - m_data is is always the object, with the lowest bit set as a flag.
  //  - We set the type fields union here.
  void setObject(ObjectData* obj) {
    assertx((intptr_t(obj) & objectBaseTag()) == 0);
    m_obj = (ObjectData*)((intptr_t)obj | objectBaseTag());
    m_typeFields = packTypeFields(IterNextIndex::Object);
    assertx(m_nextHelperIdx == IterNextIndex::Object);
    assertx(!m_specialization.specialized);
  }

  // Set the type fields of an array. These fields are packed so that we
  // can set them with a single mov-immediate to the union.
  void setArrayNext(IterNextIndex index) {
    m_typeFields = packTypeFields(index);
    assertx(m_nextHelperIdx == index);
    assertx(!m_specialization.specialized);
  }

public:
  // The iterator base. Will be null for local iterators. We set the lowest
  // bit for object iterators to distinguish them from array iterators.
  union {
    const ArrayData* m_data;
    ObjectData* m_obj;
  };
  // This field is a union so new_iter_array can set it in one instruction.
  union {
    struct {
      uint16_t m_layout;
      IterSpecialization m_specialization;
      IterNextIndex m_nextHelperIdx;
    };
    uint32_t m_typeFields;
  };
  // Current position. Beware that when m_data is null, m_pos is uninitialized.
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

  // These elements are always referenced elsewhere, either in the m_data field
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
 * (*IterInit* here refers to {IterInit, IterInitK, LIterInit, LIterInitK}).
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

  // Returns true if the base is non-empty. Only used for non-local iterators.
  // For local iterators, use new_iter_array / new_iter_array_key below.
  bool init(TypedValue* base);

  // Returns true if there are more elems. Only used for non-local iterators.
  // For local iterators, use liter_next_ind / liter_next_key_ind below.
  bool next();

  // Returns true if the iterator is at its end.
  bool end() const { return m_iter.end(); }

  // Get the current key and value. Assumes that the iter is not at its end.
  // These methods will inc-ref the key and value before returning it.
  Variant key() { return m_iter.first(); }
  Variant val() { return m_iter.second(); };

  // It's valid to call end() on a killed iter, but the iter is otherwise dead.
  // In debug builds, this method will overwrite the iterator with garbage.
  void kill() { m_iter.kill(); }

  // Dec-refs the base, for non-local iters. Safe to call for local iters.
  void free();

  // Debug string, used when printing a frame.
  std::string toString() const;

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
// For the array helpers, first provide an IterTypeOp to get an IterInit helper
// to call, then call it. This indirection lets us burn the appropriate helper
// into the JIT (where we know IterTypeOp statically). For objects, we don't
// need it because the type is always NonLocal.
using IterInitArr    = int64_t(*)(Iter*, ArrayData*, TypedValue*);
using IterInitArrKey = int64_t(*)(Iter*, ArrayData*, TypedValue*, TypedValue*);

IterInitArr    new_iter_array_helper(IterTypeOp type);
IterInitArrKey new_iter_array_key_helper(IterTypeOp type);

int64_t new_iter_object(Iter* dest, ObjectData* obj, Class* ctx,
                        TypedValue* val, TypedValue* key);


// Native helpers for the interpreter + JIT used to implement *IterInit* ops.
// These helpers return 1 if the base has more elements and 0 otherwise.
// (As above, they return a logical bool which we extend to a GP register.)
//
// If these helpers return 1, they set `val` (and `key`, for key-value iters)
// from the next key-value pair of the base.
//
// For non-local iters, if these helpers return 0, they also dec-ref the base.
NEVER_INLINE int64_t iter_next_ind(Iter* iter, TypedValue* valOut);
NEVER_INLINE int64_t iter_next_key_ind(Iter* iter, TypedValue* valOut, TypedValue* keyOut);
NEVER_INLINE int64_t liter_next_ind(Iter*, TypedValue*, ArrayData*);
NEVER_INLINE int64_t liter_next_key_ind(Iter*, TypedValue*, TypedValue*, ArrayData*);

//////////////////////////////////////////////////////////////////////

}
