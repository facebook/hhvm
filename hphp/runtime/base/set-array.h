/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"

#include <folly/portability/Constexpr.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace jit {
struct ArrayOffsetProfile;
}
struct APCArray;

//////////////////////////////////////////////////////////////////////

struct SetArray final
  : private ArrayData, type_scan::MarkCountable<SetArray> {

//////////////////////////////////////////////////////////////////////
// Set Layout

public:
  struct Elm;

  /*
   * Load factor scaler. If S is the # of elements, C is the
   * power-of-2 capacity, and L=LoadScale, we grow when S > C-C/L.
   * So 2 gives 0.5 load factor, 4 gives 0.75 load factor, 8 gives
   * 0.875 load factor. Use powers of 2 to enable shift-divide.
   *
   * The LoadScale also is the minimum size of the hash table.
   */
  static constexpr uint32_t LoadScale = 4;

  constexpr static uint32_t HashSize(uint32_t scale) { return 4 * scale; }
  constexpr static uint32_t Mask(uint32_t scale) { return HashSize(scale) - 1; }
  constexpr static uint32_t Capacity(uint32_t scale) { return 3 * scale; }
  static_assert(LoadScale == 4, "Change Capacity()");

  /*
   * The minimum hash size for a set is 4.
   */
  constexpr static uint32_t SmallScale = 1;

  uint32_t capacity() const { return Capacity(m_scale); }
  uint32_t mask() const     { return Mask(m_scale); }
  uint32_t scale() const    { return m_scale; }

  /*
   * A set array has a header of type SetArray followed by an array
   * of Capacity(m_scale) Elms, and then a hash table of
   * HashSize(m_scale) uint32_t indices.
   */
  ALWAYS_INLINE static Elm* SetData(const SetArray* a) {
    return const_cast<Elm*>(
      reinterpret_cast<Elm const*>(a + 1)
    );
  }
  Elm* data() const { return SetData(this); }

  ALWAYS_INLINE static uint32_t* SetHashTab(const SetArray* a, uint32_t scale) {
    return const_cast<uint32_t*>(
      reinterpret_cast<uint32_t const*>(SetData(a) + Capacity(scale))
    );
  }
  uint32_t* hashTab() const { return SetHashTab(this, m_scale); }

  constexpr static size_t ComputeAllocBytes(uint32_t scale) {
    return sizeof(SetArray) +
      sizeof(Elm) * Capacity(scale) +
      sizeof(uint32_t) * HashSize(scale);
  }
  size_t heapSize() const { return ComputeAllocBytes(m_scale); }

  /*
   * These two indices are used in hash tables.  It is safe
   * to choose small "negative" integers since the maximum
   * capacity is 3 * 2^30.
   */
  constexpr static uint32_t Empty     = -uint32_t{1};
  constexpr static uint32_t Tombstone = -uint32_t{2};

  template<class F> void scan(F& mark) const {
    auto const elms = data();
    auto const used = m_used;
    for (uint32_t i = 0; i < used; ++i) {
      elms[i].scan(mark);
    }
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
  static void InitHash(uint32_t* table, uint32_t scale);
  static void CopyHash(uint32_t* dest, uint32_t* src, uint32_t scale);
  static bool ClearElms(Elm* elms, uint32_t count);

  enum class AllocMode : bool { Request, Static };

  static SetArray* CopySet(const SetArray& other, AllocMode);
  static SetArray* CopyReserve(const SetArray* src, size_t expectedSize);
  SetArray* copySet() const { return CopySet(*this, AllocMode::Request); }
  SetArray* copyAndResizeIfNeeded() const;

private:
  SetArray() = delete;
  SetArray(const SetArray&) = delete;
  SetArray& operator=(const SetArray&) = delete;
  ~SetArray() = delete;

//////////////////////////////////////////////////////////////////////
// Iteration

private:
  ssize_t getIterBegin() const;
  ssize_t getIterLast() const;
  ssize_t getIterEnd() const { return m_used; }
  void getElm(ssize_t ei, TypedValue* out) const;

  ssize_t nextElm(Elm* elms, ssize_t ei) const;
  ssize_t prevElm(Elm* elms, ssize_t ei) const;
  ssize_t nextElm(ssize_t ei) const { return nextElm(data(), ei); }

public:
  const TypedValue* tvOfPos(uint32_t) const;

  template <class F, bool inc = true>
  static void Iterate(const SetArray* a, F fn) {
    if (inc) a->incRefCount();
    SCOPE_EXIT { if (inc) decRefArr(const_cast<SetArray*>(a)); };
    auto const* elm = a->data();
    for (auto i = a->m_used; i--; elm++) {
      if (LIKELY(!elm->isTombstone())) {
        if (ArrayData::call_helper(fn, &elm->tv)) break;
      }
    }
  }

//////////////////////////////////////////////////////////////////////
// Set Internals

private:
  bool checkInvariants() const;
  bool isFull() const;

  using hash_t = strhash_t;

  /*
   * These functions require !isFull().  The index of the position
   * where the element was inserted is returned.
   */
  void insert(int64_t k, inthash_t h);
  void insert(int64_t k);
  void insert(StringData* k, strhash_t h);
  void insert(StringData* k);

  Elm* allocElm(uint32_t*);

  enum FindType { Lookup, Insert, Remove };

  template <FindType type, class Hit>
  typename std::conditional<
    type == FindType::Lookup,
    ssize_t,
    uint32_t*
  >::type findImpl(hash_t h0, Hit) const;

  ssize_t find(int64_t ki, inthash_t h) const;
  ssize_t find(const StringData* s, strhash_t h) const;
  ssize_t findElm(const Elm& e) const;
  template<FindType t> uint32_t* findHash(int64_t, inthash_t) const;
  template<FindType t> uint32_t* findHash(const StringData*, strhash_t) const;
  uint32_t* findForNewInsert(hash_t h) const;

  void erase(uint32_t*);

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
   * Returns a new set containing all the elements of the current set
   * with the new specified scale.  The original set must not be used
   * afterwards.  If the passed scale is smaller than the original
   * one, grow() can shrink too!
   */
  SetArray* grow(uint32_t newScale);

  /*
   * compact() removes all tombstones from the hash table by going
   * through the inner linked list.
   */
  void compact();

  /*
   * resize() and resizeIfNeeded() will grow the array as necessary to
   * ensure that there is room for a new element and a new hash entry.
   *
   * resize() assumes isFull().  resizeIfNeeded() will first check if
   * there is room for a new element and hash entry before growing the
   * array.
   *
   * Both functions return the new SetArray* to use (or the old one
   * if they didn't need to grow).  The old SetArray is left in a
   * zombie state where the only legal action is to decref and then
   * throw it away.
   */
  SetArray* resize();
  SetArray* resizeIfNeeded();

  /*
   * Zombie arrays!
   */
  bool isZombie() const { return m_used + 1 == 0; }
  void setZombie() { m_used = -uint32_t{1}; }

  /*
   * Comparison helper.
   */
  static bool EqualHelper(const ArrayData*, const ArrayData*);

//////////////////////////////////////////////////////////////////////
// Elements

public:
  struct Elm {
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

    bool hasStrKey() const {
      /*
       * Currently string hash is 31-bit, thus it saves us some
       * instructions to encode int keys as a negative hash, so
       * that we don't have to care about the MSB when working
       * with strhash_t.
       */
      assert(!isInvalid());
      return tv.hash() >= 0;
    }

    bool hasIntKey() const {
      assert(!isInvalid());
      return tv.hash() < 0;
    }

    void setStrKey(StringData* k, strhash_t h) {
      assert(isEmpty());
      k->incRefCount();
      tv.m_type = KindOfString;
      tv.m_data.pstr = k;
      tv.hash() = h;
      assert(!isInvalid());
    }

    StringData* strKey() const {
      assert(hasStrKey());
      return tv.m_data.pstr;
    }

    int64_t intKey() const {
      assert(hasIntKey());
      return tv.m_data.num;
    }

    void setIntKey(int64_t k, inthash_t h) {
      assert(isEmpty());
      tv.m_type = KindOfInt64;
      tv.m_data.num = k;
      tv.hash() = h | STRHASH_MSB;
      assert(!isInvalid());
      assert(hasIntKey());
      static_assert(STRHASH_MSB < 0, "using strhash_t = int32_t");
    }

    void setTombstone() {
      tv.m_type = kTombstone;
    }

    bool isTombstone() const {
      static_assert(
        kEmpty == 0 && kTombstone < 0 &&
        KindOfString > kEmpty && KindOfInt64 > kEmpty,
        "Fix the check below."
      );
      return tv.m_type < kEmpty;
    }

    bool isEmpty() const {
      return tv.m_type == kEmpty;
    }

    bool isInvalid() const {
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

    hash_t hash() const {
      return tv.hash();
    }

    template<class F> void scan(F& mark) const {
      if (!isTombstone()) mark(tv);
    }
  };

//////////////////////////////////////////////////////////////////////
// JIT Supporting Routines

  static constexpr ptrdiff_t usedOff() {
    return offsetof(SetArray, m_used);
  }

  static constexpr ptrdiff_t dataOff() {
    return sizeof(SetArray);
  }

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

  // These using directives ensure the full set of overloaded functions
  // are visible in this class, to avoid triggering implicit conversions
  // from a const Variant& key to int64.
private:
  using ArrayData::exists;
  using ArrayData::lval;
  using ArrayData::lvalNew;
  using ArrayData::set;
  using ArrayData::setRef;
  using ArrayData::add;
  using ArrayData::remove;
  using ArrayData::nvGet;
  using ArrayData::release;

//////////////////////////////////////////////////////////////////////
// Friends

private:
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

//////////////////////////////////////////////////////////////////////
// ArrayData API

public:
  static const TypedValue* NvGetInt(const ArrayData*, int64_t);
  static const TypedValue* NvGetStr(const ArrayData*, const StringData*);
  static const TypedValue* NvTryGetInt(const ArrayData*, int64_t);
  static const TypedValue* NvTryGetStr(const ArrayData*, const StringData*);
  static void NvGetKey(const ArrayData*, TypedValue*, ssize_t);
  static size_t Vsize(const ArrayData*);
  static const Variant& GetValueRef(const ArrayData*, ssize_t);
  static bool IsVectorData(const ArrayData*);
  static bool ExistsInt(const ArrayData*, int64_t);
  static bool ExistsStr(const ArrayData*, const StringData*);
  static ArrayData* LvalInt(ArrayData*, int64_t, Variant*&, bool);
  static ArrayData* LvalIntRef(ArrayData*, int64_t, Variant*&, bool);
  static ArrayData* LvalStr(ArrayData*, StringData*, Variant*&, bool);
  static ArrayData* LvalStrRef(ArrayData*, StringData*, Variant*&, bool);
  static ArrayData* LvalNew(ArrayData*, Variant*&, bool);
  static ArrayData* LvalNewRef(ArrayData*, Variant*&, bool);
  static ArrayData* SetRefInt(ArrayData*, int64_t, Variant&, bool);
  static ArrayData* SetRefStr(ArrayData*, StringData*, Variant&, bool);
  static ArrayData* SetInt(ArrayData*, int64_t, Cell, bool);
  static ArrayData* SetStr(ArrayData*, StringData*, Cell, bool);
  static ArrayData* RemoveInt(ArrayData*, int64_t, bool);
  static ArrayData* RemoveStr(ArrayData*, const StringData*, bool);
  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterLast(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t);
  static ssize_t IterRewind(const ArrayData*, ssize_t);
  static constexpr auto ValidMArrayIter = &ArrayCommon::ValidMArrayIter;
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter&);
  static ArrayData* EscalateForSort(ArrayData* ad, SortFunction) { return ad; }
  static void SortThrow(ArrayData*, int, bool);
  static constexpr auto Ksort = &SortThrow;
  static constexpr auto Sort = &SortThrow;
  static constexpr auto Asort = &SortThrow;
  static bool USortThrow(ArrayData*, const Variant&);
  static constexpr auto Uksort = &USortThrow;
  static constexpr auto Usort = &USortThrow;
  static constexpr auto Uasort = &USortThrow;
  static ArrayData* Copy(const ArrayData*);
  static ArrayData* CopyWithStrongIterators(const ArrayData*);
  static ArrayData* CopyStatic(const ArrayData*);
  static ArrayData* Append(ArrayData*, Cell, bool);
  static ArrayData* AppendRef(ArrayData*, Variant&, bool);
  static ArrayData* AppendWithRef(ArrayData*, const Variant&, bool);
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
  static bool Equal(const ArrayData*, const ArrayData*);
  static bool NotEqual(const ArrayData*, const ArrayData*);
  static constexpr auto Same = &Equal;

//////////////////////////////////////////////////////////////////////

private:
  struct Initializer;
  static Initializer s_initializer;

  /*
   * Some of these are packed into qword-sized unions so we can
   * combine stores during initialization. (gcc won't do it on its own.)
   */

  union {
    struct {
      uint32_t m_scale; // Size-class equal to 1/4 table size.
      uint32_t m_used;  // Number of used entries in the hash.
                        // (includes tombstones)
    };
    uint64_t m_scale_used;
  };
  uint64_t m_padding;

};

//////////////////////////////////////////////////////////////////////

extern std::aligned_storage<
  SetArray::ComputeAllocBytes(SetArray::SmallScale),
  folly::constexpr_max(alignof(SetArray), size_t(16))
>::type s_theEmptySetArray;

ALWAYS_INLINE ArrayData* staticEmptyKeysetArray() {
  void* vp = &s_theEmptySetArray;
  return static_cast<ArrayData*>(vp);
}

//////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_SET_ARRAY_H_
