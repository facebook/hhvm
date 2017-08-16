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

#ifndef incl_HPHP_SET_ARRAY_H_
#define incl_HPHP_SET_ARRAY_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/hash-table.h"
#include "hphp/runtime/base/member-val.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/typed-value.h"

#include <folly/portability/Constexpr.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace jit {
struct ArrayOffsetProfile;
}
struct APCArray;
struct APCHandle;

struct SetArrayElm {
  using hash_t = strhash_t;
  /*
   * We store elements of the set here, but also some information
   * local to this array: tv.m_aux.u_hash contains either a negative
   * number (for an int key) or a string hashcode (31-bit and thus
   * non-negative).  The field tv.m_type == kInvalidDataType is used
   * to mark a deleted element (tombstone); tv.m_type == KindOfUninit
   * is used for debugging to mark empty elements.
   */
  TypedValueAux tv;

  static auto constexpr kTombstone = kInvalidDataType;
  static auto constexpr kEmpty = KindOfUninit;

  void setStrKey(StringData* k, strhash_t h) {
    assert(isEmpty());
    k->incRefCount();
    tv.m_type = KindOfString;
    tv.m_data.pstr = k;
    tv.hash() = h;
    assert(!isInvalid());
  }

  void setIntKey(int64_t k, inthash_t h) {
    assert(isEmpty());
    tv.m_type = KindOfInt64;
    tv.m_data.num = k;
    tv.hash() = h | STRHASH_MSB;
    assert(!isInvalid());
    assert(hasIntKey());
  }

  void setTombstone() {
    tv.m_type = kTombstone;
  }

  bool isEmpty() const {
    return tv.m_type == kEmpty;
  }

  // Members below here are required for HashTable implemenation.
  ALWAYS_INLINE const TypedValue* datatv() const {
    return &tv;
  }

  ALWAYS_INLINE bool hasStrKey() const {
    /*
     * Currently string hash is 31-bit, thus it saves us some
     * instructions to encode int keys as a negative hash, so
     * that we don't have to care about the MSB when working
     * with strhash_t.
     */
    assert(!isInvalid());
    return tv.hash() >= 0;
  }

  ALWAYS_INLINE StringData* strKey() const {
    assert(hasStrKey());
    return tv.m_data.pstr;
  }

  ALWAYS_INLINE bool hasIntKey() const {
    assert(!isInvalid());
    return tv.hash() < 0;
  }

  ALWAYS_INLINE int64_t intKey() const {
    return tv.m_data.num;
  }

  ALWAYS_INLINE
  Cell getKey() const {
    assert(!isInvalid());
    Cell out;
    cellDup(tv, out);
    return out;
  }


  ALWAYS_INLINE hash_t hash() const {
    return tv.hash();
  }

  ALWAYS_INLINE bool isTombstone() const {
    static_assert(
      kEmpty == 0 && kTombstone < 0 &&
      KindOfString > kEmpty && KindOfInt64 > kEmpty,
      "Fix the check below."
    );
    return tv.m_type < kEmpty;
  }

  ALWAYS_INLINE bool isInvalid() const {
    // An element is invalid if it is a tombstone or empty.
    static_assert(
      kTombstone < kEmpty &&
      kEmpty < KindOfInt64 &&
      kEmpty < KindOfString &&
      kEmpty < KindOfPersistentString,
      "Revise m_type choices."
    );
    return tv.m_type <= kEmpty;
  }

  static constexpr ptrdiff_t keyOff() {
    return offsetof(SetArrayElm, tv) + offsetof(TypedValue, m_data.pstr);
  }
  static constexpr ptrdiff_t dataOff() {
    return offsetof(SetArrayElm, tv);
  }
  static constexpr ptrdiff_t hashOff() {
    return offsetof(SetArrayElm, tv) + offsetof(TypedValue, m_aux);
  }
};

//////////////////////////////////////////////////////////////////////

struct SetArray final : ArrayData,
                        array::HashTable<SetArray, SetArrayElm>,
                        type_scan::MarkCountable<SetArray> {

//////////////////////////////////////////////////////////////////////
// Set Layout

public:
  void scan(type_scan::Scanner& scanner) const {
    auto const elms = data();
    scanner.scan(*elms, m_used * sizeof(*elms));
  }

//////////////////////////////////////////////////////////////////////
// Initialization, Copies, and Conversions

public:
  /*
   * Allocate a new, empty, request-local set array, with enough space
   * reserved for `size' members.
   *
   * The returned array is already incref'd.
   */
  static ArrayData* MakeReserveSet(uint32_t size);
  static constexpr auto MakeReserve = &MakeReserveSet;

  static ArrayData* MakeSet(uint32_t size, const TypedValue* values);

  /*
   * Allocate an uncounted SetArray and copy the values from the
   * input 'array' into the uncounted one.
   *
   * 'extra' bytes may be allocated in front of the returned pointer,
   * must be a multiple of 16, and later be passed to ReleaseUncounted.
   * (This is used to co-allocate a TypedValue with its array data.)
   */
  static ArrayData* MakeUncounted(ArrayData* array, size_t extra = 0);

  static void Release(ArrayData*);
  static void ReleaseUncounted(ArrayData*, size_t);
  /*
   * Recursively register {allocation, rootAPCHandle} with APCGCManager
   */
  static void RegisterUncountedAllocations(ArrayData* ad,
                                              APCHandle* rootAPCHandle);

  /*
   * Safe downcast helpers.
   */
  static SetArray* asSet(ArrayData* ad);
  static const SetArray* asSet(const ArrayData* ad);

  static ArrayData* MakeSetFromAPC(const APCArray*);

  /*
   * For array initialization using KeysetInit.
   */
  static ArrayData* AddToSet(ArrayData*, int64_t, bool);
  static ArrayData* AddToSet(ArrayData*, StringData*, bool);

private:
  static bool ClearElms(Elm* elms, uint32_t count);

  enum class AllocMode : bool { Request, Static };

  static SetArray* CopySet(const SetArray& other, AllocMode);
  static SetArray* CopyReserve(const SetArray* src, size_t expectedSize);
  SetArray* copySet() const { return CopySet(*this, AllocMode::Request); }

private:
  SetArray() = delete;
  SetArray(const SetArray&) = delete;
  SetArray& operator=(const SetArray&) = delete;
  ~SetArray() = delete;

//////////////////////////////////////////////////////////////////////
// Iteration

private:
  Cell getElm(ssize_t ei) const;

public:
  const TypedValue* tvOfPos(uint32_t) const;

  template <class F, bool inc = true>
  static void Iterate(const SetArray* a, F fn) {
    if (inc) a->incRefCount();
    SCOPE_EXIT { if (inc) decRefArr(const_cast<SetArray*>(a)); };
    auto const* elm = a->data();
    for (auto i = a->m_used; i--; elm++) {
      if (LIKELY(!elm->isTombstone())) {
        if (ArrayData::call_helper(fn, elm->tv)) break;
      }
    }
  }

//////////////////////////////////////////////////////////////////////
// Sorting

public:
  static ArrayData* EscalateForSort(ArrayData* ad, SortFunction);

  static void Sort(ArrayData*, int, bool);
  static void Ksort(ArrayData*, int, bool);
  static constexpr auto Asort = &Ksort;

  static bool Usort(ArrayData*, const Variant&);
  static bool Uksort(ArrayData*, const Variant&);
  static constexpr auto Uasort = &Uksort;

private:
  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc, bool checkTypes);
  void postSort();

//////////////////////////////////////////////////////////////////////
// Set Internals

private:
  bool checkInvariants() const;

  /*
   * These functions require !isFull().  The index of the position
   * where the element was inserted is returned.
   */
  void insert(int64_t k, inthash_t h);
  void insert(int64_t k);
  void insert(StringData* k, strhash_t h);
  void insert(StringData* k);

  ssize_t findElm(const Elm& e) const;

  void erase(int32_t);

  /*
   * Append idx at the end of the linked list containing the set
   * insertion order.
   */
  void linkLast(uint32_t idx);

  /*
   * Helper routine for inserting elements into a new array
   * when grow()ing the array, that also checks for potentially
   * unbalanced entries because of hash collision.
   */
  SetArray* insertCheckUnbalanced(SetArray* ad, Elm* table,
                                  uint32_t mask,
                                  Elm* iter, Elm* stop);


  /*
   * Returns a copy of the set with twice the scale of the original. It
   * rebuilds the hash table, but it does not compact the elements. If copy is
   * true, it will copy elements instead of taking ownership of them.
   */
  SetArray* grow(bool copy);

  /*
   * prepareForInsert ensures that the set has room to insert an element and
   * has a refcount of 1, copying if requested and growing if needed.
   */
  SetArray* prepareForInsert(bool copy);

  /*
   * compact() removes all tombstones from the hash table by going
   * through the inner linked list.
   */
  void compact();

  /*
   * Zombie arrays!
   */
  bool isZombie() const { return m_used + 1 == 0; }
  void setZombie() { m_used = -uint32_t{1}; }

  /*
   * Comparison helper.
   */
  static bool EqualHelper(const ArrayData*, const ArrayData*, bool);

//////////////////////////////////////////////////////////////////////
// Elements

public:

//////////////////////////////////////////////////////////////////////
// JIT Supporting Routines

  static constexpr ptrdiff_t tvOff(uint32_t pos) {
    return dataOff() + pos * sizeof(Elm) + offsetof(Elm, tv);
  }

//////////////////////////////////////////////////////////////////////
// Reference Counting

public:
  using ArrayData::decRefCount;
  using ArrayData::hasMultipleRefs;
  using ArrayData::hasExactlyOneRef;
  using ArrayData::decWillRelease;
  using ArrayData::incRefCount;

//////////////////////////////////////////////////////////////////////
// Misc ArrayData Methods

  /*
   * These using directives ensure the full set of overloaded functions
   * are visible in this class, to avoid triggering implicit conversions
   * from a const Variant& key to int64.
   */
private:
  using ArrayData::exists;
  using ArrayData::at;
  using ArrayData::rval;
  using ArrayData::lval;
  using ArrayData::lvalNew;
  using ArrayData::set;
  using ArrayData::setRef;
  using ArrayData::add;
  using ArrayData::remove;
  using ArrayData::release;

//////////////////////////////////////////////////////////////////////
// Friends

private:
  friend struct array::HashTable<SetArray, SetArrayElm>;
  friend struct ArrayInit;
  friend struct MemoryProfile;
  friend struct jit::ArrayOffsetProfile;
  friend struct EmptyArray;
  friend struct PackedArray;
  friend struct StructArray;
  friend struct MixedArray;
  friend struct HashCollection;
  friend struct BaseMap;
  friend struct c_Map;
  friend struct c_ImmMap;
  friend struct BaseSet;
  friend struct c_Set;
  friend struct c_ImmSet;
  friend struct c_AwaitAllWaitHandle;

  friend size_t getMemSize(const ArrayData*);
  template <typename AccessorT, class ArrayT>
  friend SortFlavor genericPreSort(ArrayT&, const AccessorT&, bool);

//////////////////////////////////////////////////////////////////////
// ArrayData API

public:
  static member_rval::ptr_u NvTryGetInt(const ArrayData*, int64_t);
  static member_rval::ptr_u NvTryGetStr(const ArrayData*, const StringData*);
  static member_rval RvalIntStrict(const ArrayData* ad, int64_t k) {
    return member_rval { ad, NvTryGetInt(ad, k) };
  }
  static member_rval RvalStrStrict(const ArrayData* ad, const StringData* k) {
    return member_rval { ad, NvTryGetStr(ad, k) };
  }
  static member_rval RvalAtPos(const ArrayData* ad, ssize_t pos) {
    return member_rval { ad, GetValueRef(ad, pos) };
  }
  static size_t Vsize(const ArrayData*);
  static member_rval::ptr_u GetValueRef(const ArrayData*, ssize_t);
  static bool IsVectorData(const ArrayData*);
  static bool ExistsInt(const ArrayData*, int64_t);
  static bool ExistsStr(const ArrayData*, const StringData*);
  static member_lval LvalInt(ArrayData*, int64_t, bool);
  static member_lval LvalIntRef(ArrayData*, int64_t, bool);
  static member_lval LvalStr(ArrayData*, StringData*, bool);
  static member_lval LvalStrRef(ArrayData*, StringData*, bool);
  static member_lval LvalNew(ArrayData*, bool);
  static member_lval LvalNewRef(ArrayData*, bool);
  static ArrayData* SetInt(ArrayData*, int64_t, Cell, bool);
  static ArrayData* SetStr(ArrayData*, StringData*, Cell, bool);
  static ArrayData* SetWithRefInt(ArrayData*, int64_t, TypedValue, bool);
  static ArrayData* SetWithRefStr(ArrayData*, StringData*, TypedValue, bool);
  static ArrayData* SetRefInt(ArrayData*, int64_t, Variant&, bool);
  static ArrayData* SetRefStr(ArrayData*, StringData*, Variant&, bool);
  static ArrayData* RemoveInt(ArrayData*, int64_t, bool);
  static ArrayData* RemoveStr(ArrayData*, const StringData*, bool);
  static constexpr auto ValidMArrayIter = &ArrayCommon::ValidMArrayIter;
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter&);
  static ArrayData* Copy(const ArrayData*);
  static ArrayData* CopyStatic(const ArrayData*);
  static ArrayData* Append(ArrayData*, Cell, bool);
  static ArrayData* AppendRef(ArrayData*, Variant&, bool);
  static ArrayData* AppendWithRef(ArrayData*, TypedValue, bool);
  static ArrayData* PlusEq(ArrayData*, const ArrayData*);
  static ArrayData* Merge(ArrayData*, const ArrayData*);
  static ArrayData* Pop(ArrayData*, Variant&);
  static ArrayData* Dequeue(ArrayData*, Variant&);
  static ArrayData* Prepend(ArrayData*, Cell, bool);
  static void Renumber(ArrayData*);
  static void OnSetEvalScalar(ArrayData*);
  static ArrayData* Escalate(const ArrayData*);
  static constexpr auto ToDict = &ArrayCommon::ToDict;
  static constexpr auto ToVec = &ArrayCommon::ToVec;
  static ArrayData* ToPHPArray(ArrayData*, bool);
  static ArrayData* ToKeyset(ArrayData*, bool);
  static constexpr auto ToVArray = &ArrayCommon::ToVArray;
  static bool Equal(const ArrayData*, const ArrayData*);
  static bool NotEqual(const ArrayData*, const ArrayData*);
  static bool Same(const ArrayData*, const ArrayData*);
  static bool NotSame(const ArrayData*, const ArrayData*);

//////////////////////////////////////////////////////////////////////

private:
  struct Initializer;
  static Initializer s_initializer;

  uint64_t m_padding;
};

HASH_TABLE_CHECK_OFFSETS(SetArray, SetArrayElm)
//////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_SET_ARRAY_H_
