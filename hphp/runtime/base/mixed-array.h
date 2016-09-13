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

#ifndef incl_HPHP_HPHP_ARRAY_H_
#define incl_HPHP_HPHP_ARRAY_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"

#include <folly/portability/Constexpr.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct APCArray;
struct ArrayInit;
struct MemoryProfile;

//////////////////////////////////////////////////////////////////////

struct MixedArray final : private ArrayData,
                          type_scan::MarkCountable<MixedArray> {
  // Load factor scaler. If S is the # of elements, C is the
  // power-of-2 capacity, and L=LoadScale, we grow when S > C-C/L.
  // So 2 gives 0.5 load factor, 4 gives 0.75 load factor, 8 gives
  // 0.875 load factor. Use powers of 2 to enable shift-divide.
  static constexpr uint32_t LoadScale = 4;

  constexpr static uint32_t HashSize(uint32_t scale) { return 4 * scale; }
  constexpr static uint32_t Capacity(uint32_t scale) { return 3 * scale; }
  constexpr static uint32_t Mask(uint32_t scale) { return 4 * scale - 1; }

  using hash_t = strhash_t;

  /*
   * Iterator helper for kPackedKind and kMixedKind.  You can use this
   * to look at the values in the array, but not the keys unless you
   * know it is kMixedKind.
   *
   * This can be used as an optimization vs. ArrayIter, which uses
   * indirect calls in the loop.
   */
  struct ValIter;

  struct Elm {
    union {
      int64_t ikey;
      StringData* skey;
    };
    // We store values here, but also some information local to this array:
    // data.m_aux.u_hash contains either a negative number (for an int key) or
    // a string hashcode (31-bit and thus non-negative); the high bit is the
    // int/string key descriminator. data.m_type == kInvalidDataType if this is
    // an empty slot in the array (e.g. after a key is deleted).  It is
    // critical that when we return &data to clients, that they not read or
    // write the m_aux field!
    TypedValueAux data;

    bool hasStrKey() const {
      // Currently string hash is 31-bit, thus it saves us some instructions to
      // encode int keys as a negative hash, so that we don't have to care about
      // the MSB when working with strhash_t.
      return data.hash() >= 0;
    }

    bool hasIntKey() const {
      return data.hash() < 0;
    }

    hash_t hash() const {
      return data.hash();
    }

    hash_t probe() const {
      return hash();
    }

    void setStaticKey(StringData* k, strhash_t h) {
      assert(k->isStatic());
      skey = k;
      data.hash() = h;
    }

    void setStrKey(StringData* k, strhash_t h) {
      skey = k;
      data.hash() = h;
      k->incRefCount();
    }

    void setIntKey(int64_t k, inthash_t h) {
      ikey = k;
      data.hash() = h | STRHASH_MSB;
      assert(hasIntKey());
      static_assert(STRHASH_MSB < 0, "using strhash_t = int32_t");
    }

    bool isTombstone() const {
      return MixedArray::isTombstone(data.m_type);
    }

    template<class F> void scan(F& mark) const {
      if (!isTombstone()) {
        if (hasStrKey()) mark(skey);
        mark(data);
      }
    }

    static constexpr ptrdiff_t keyOff() {
      return offsetof(Elm, ikey);
    }
    static constexpr ptrdiff_t dataOff() {
      return offsetof(Elm, data);
    }
  };

  static constexpr ptrdiff_t dataOff() {
    return sizeof(MixedArray);
  }
  static constexpr ptrdiff_t usedOff() {
    return offsetof(MixedArray, m_used);
  }

  static constexpr ptrdiff_t elmOff(uint32_t pos) {
    return dataOff() + pos * sizeof(Elm);
  }

  struct ElmKey {
    ElmKey() {}
    ElmKey(strhash_t hash, StringData* key)
        : skey(key), hash(hash)
      {}
    union {
      StringData* skey;
      int64_t ikey;
    };
    hash_t hash;

    TYPE_SCAN_CUSTOM() {
      if (hash < 0) scanner.enqueue(skey);
      static_assert(STRHASH_MSB < 0, "using strhash_t = int32_t");
    }
  };

  /*
   * Initialize an empty small mixed array with given field. This should be
   * inlined.
   */
  static void InitSmall(MixedArray* a, RefCount count, uint32_t size,
                        int64_t nextIntKey);

  /*
   * Allocate a new, empty, request-local array in (mixed|dict) mode, with
   * enough space reserved for `capacity' members.
   *
   * The returned array is already incref'd.
   */
  static ArrayData* MakeReserveMixed(uint32_t size);
  static ArrayData* MakeReserveDict(uint32_t size);

  /*
   * Convert mixed-layout array to dict in-place. This function doesn't check
   * whether the input array contains references or not, so only use this when
   * you already know that they do not.
   */
  static MixedArray* ToDictInPlace(ArrayData*);

  /*
   * MakeReserveSame allocates a new, empty, request-local array with the same
   * mode as `other' and with enough space reserved for `capacity' members, or
   * if `capacity' is zero, with the same capacity as `other'.
   *
   * MakeReserveLike will return a PHP array with a memory representation
   * similar to the one used by `other'.
   *
   * The returned array is already incref'd.
   */
  static ArrayData* MakeReserveSame(const ArrayData* other, uint32_t capacity);
  static ArrayData* MakeReserveLike(const ArrayData* other, uint32_t capacity);

  /*
   * Like MakePacked, but given static strings, make a struct-like array.
   * Also requires size > 0.
   */
  static MixedArray* MakeStruct(uint32_t size, const StringData* const* keys,
                               const TypedValue* values);

  /*
   * Allocate an uncounted MixedArray and copy the values from the
   * input 'array' into the uncounted one. All values copied are made
   * uncounted as well.  An uncounted array can only contain uncounted
   * values (primitive values, uncounted or static strings and
   * uncounted or static arrays).  The Packed version does the same
   * when the array has a kPackedKind.
   *
   * 'extra' bytes may be allocated in front of the returned pointer,
   * must be a multiple of 16, and later be passed to ReleaseUncounted.
   * (This is used to co-allocate a TypedValue with its array data.)
   */
  static ArrayData* MakeUncounted(ArrayData* array, size_t extra = 0);

  static ArrayData* MakeDictFromAPC(const APCArray* apc);

  // This behaves the same as iter_begin except that it assumes
  // this array is not empty and its not virtual.
  ALWAYS_INLINE
  ssize_t getIterBegin() const {
    assert(!empty());
    if (LIKELY(!data()[0].isTombstone())) {
      return 0;
    }
    return nextElm(data(), 0);
  }

  static bool DictEqual(const ArrayData*, const ArrayData*);
  static bool DictNotEqual(const ArrayData*, const ArrayData*);
  static bool DictSame(const ArrayData*, const ArrayData*);
  static bool DictNotSame(const ArrayData*, const ArrayData*);

  using ArrayData::decRefCount;
  using ArrayData::hasMultipleRefs;
  using ArrayData::hasExactlyOneRef;
  using ArrayData::decWillRelease;
  using ArrayData::incRefCount;

  /*
   * MixedArray is convertible to ArrayData*, but not implicitly.
   * This is to avoid accidentally using virtual dispatch when you
   * already know something is Mixed.
   *
   * I.e., instead of doing things like mixed->nvGet(...) you want to
   * do MixedArray::NvGetInt(adYouKnowIsMixed, ...).  This means using
   * MixedArray*'s directly shouldn't really happen very often.
   */
  ArrayData* asArrayData() { return this; }
  const ArrayData* asArrayData() const { return this; }

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

  static const TypedValue* NvTryGetIntHackArr(const ArrayData*, int64_t);
  static const TypedValue* NvTryGetStrHackArr(const ArrayData*,
                                              const StringData*);

  static ArrayData* LvalIntRefHackArr(ArrayData*, int64_t, Variant*&, bool);
  static ArrayData* LvalStrRefHackArr(ArrayData*, StringData*, Variant*&, bool);
  static ArrayData* LvalNewRefHackArr(ArrayData*, Variant*&, bool);
  static ArrayData* SetRefIntHackArr(ArrayData*, int64_t, Variant&, bool);
  static ArrayData* SetRefStrHackArr(ArrayData*, StringData*, Variant&, bool);
  static ArrayData* AppendRefHackArr(ArrayData*, Variant&, bool);
  static ArrayData* AppendWithRefHackArr(ArrayData*, const Variant&, bool);

public:
  static Variant CreateVarForUncountedArray(const Variant& source);

  static size_t Vsize(const ArrayData*);
  static const Variant& GetValueRef(const ArrayData*, ssize_t pos);
  static bool IsVectorData(const ArrayData*);
  static const TypedValue* NvGetInt(const ArrayData*, int64_t ki);
  static constexpr auto NvTryGetInt = &NvGetInt;
  static const TypedValue* NvGetStr(const ArrayData*, const StringData* k);
  static constexpr auto NvTryGetStr = &NvGetStr;
  static void NvGetKey(const ArrayData*, TypedValue* out, ssize_t pos);
  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterLast(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t pos);
  static ssize_t IterRewind(const ArrayData*, ssize_t pos);
  static bool ExistsInt(const ArrayData*, int64_t k);
  static bool ExistsStr(const ArrayData*, const StringData* k);
  static ArrayData* LvalInt(ArrayData* ad, int64_t k, Variant*& ret,
                            bool copy);
  static constexpr auto LvalIntRef = &LvalInt;
  static ArrayData* LvalStr(ArrayData* ad, StringData* k, Variant*& ret,
                            bool copy);
  static constexpr auto LvalStrRef = &LvalStr;
  static ArrayData* LvalNew(ArrayData*, Variant*& ret, bool copy);
  static constexpr auto LvalNewRef = &LvalNew;
  static ArrayData* SetInt(ArrayData*, int64_t k, Cell v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, Cell v, bool copy);
  // TODO(t4466630) Do we want to raise warnings in zend compatibility mode?
  static ArrayData* ZSetInt(ArrayData*, int64_t k, RefData* v);
  static ArrayData* ZSetStr(ArrayData*, StringData* k, RefData* v);
  static ArrayData* ZAppend(ArrayData* ad, RefData* v, int64_t* key_ptr);
  static ArrayData* SetRefInt(ArrayData* ad, int64_t k, Variant& v, bool copy);
  static ArrayData* SetRefStr(ArrayData* ad, StringData* k, Variant& v,
                              bool copy);
  static ArrayData* AddInt(ArrayData*, int64_t k, Cell v, bool copy);
  static ArrayData* AddStr(ArrayData*, StringData* k, Cell v, bool copy);
  static ArrayData* RemoveInt(ArrayData*, int64_t k, bool copy);
  static ArrayData* RemoveStr(ArrayData*, const StringData* k, bool copy);
  static ArrayData* Copy(const ArrayData*);
  static ArrayData* CopyWithStrongIterators(const ArrayData*);
  static ArrayData* CopyStatic(const ArrayData*);
  static ArrayData* Append(ArrayData*, Cell v, bool copy);
  static ArrayData* AppendRef(ArrayData*, Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, const Variant& v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Pop(ArrayData*, Variant& value);
  static ArrayData* Dequeue(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, Cell v, bool copy);
  static ArrayData* ToPHPArray(ArrayData*, bool);
  static ArrayData* ToDict(ArrayData*, bool);
  static constexpr auto ToVec = &ArrayCommon::ToVec;
  static constexpr auto ToKeyset = &ArrayCommon::ToKeyset;

  static void Renumber(ArrayData*);
  static void OnSetEvalScalar(ArrayData*);
  static void Release(ArrayData*);
  static void ReleaseUncounted(ArrayData*, size_t extra = 0);
  static constexpr auto ValidMArrayIter = &ArrayCommon::ValidMArrayIter;
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter& fp);
  static ArrayData* Escalate(const ArrayData* ad) {
    return const_cast<ArrayData*>(ad);
  }

  static ArrayData* EscalateForSort(ArrayData* ad, SortFunction sf);
  static void Ksort(ArrayData*, int sort_flags, bool ascending);
  static void Sort(ArrayData*, int sort_flags, bool ascending);
  static void Asort(ArrayData*, int sort_flags, bool ascending);
  static bool Uksort(ArrayData*, const Variant& cmp_function);
  static bool Usort(ArrayData*, const Variant& cmp_function);
  static bool Uasort(ArrayData*, const Variant& cmp_function);

  static constexpr auto NvTryGetIntDict = &NvTryGetIntHackArr;
  static constexpr auto NvGetIntDict = &NvGetInt;
  static constexpr auto NvTryGetStrDict = &NvTryGetStrHackArr;
  static constexpr auto NvGetStrDict = &NvGetStr;
  static constexpr auto ReleaseDict = &Release;
  static constexpr auto NvGetKeyDict = &NvGetKey;
  static constexpr auto SetIntDict = &SetInt;
  static constexpr auto SetStrDict = &SetStr;
  static constexpr auto AddIntDict = &AddInt;
  static constexpr auto AddStrDict = &AddStr;
  static constexpr auto VsizeDict = &Vsize;
  static constexpr auto GetValueRefDict = &GetValueRef;
  static constexpr auto IsVectorDataDict = &IsVectorData;
  static constexpr auto ExistsIntDict = &ExistsInt;
  static constexpr auto ExistsStrDict = &ExistsStr;
  static constexpr auto LvalIntDict = &LvalInt;
  static constexpr auto LvalStrDict = &LvalStr;
  static constexpr auto LvalNewDict = &LvalNew;
  static constexpr auto RemoveIntDict = &RemoveInt;
  static constexpr auto RemoveStrDict = &RemoveStr;
  static constexpr auto IterBeginDict = &IterBegin;
  static constexpr auto IterLastDict = &IterLast;
  static constexpr auto IterEndDict = &IterEnd;
  static constexpr auto IterAdvanceDict = &IterAdvance;
  static constexpr auto IterRewindDict = &IterRewind;
  static constexpr auto ValidMArrayIterDict = ValidMArrayIter;
  static constexpr auto AdvanceMArrayIterDict = &AdvanceMArrayIter;
  static constexpr auto EscalateForSortDict = &EscalateForSort;
  static constexpr auto KsortDict = &Ksort;
  static constexpr auto SortDict = &Sort;
  static constexpr auto AsortDict = &Asort;
  static constexpr auto UksortDict = &Uksort;
  static constexpr auto UsortDict = &Usort;
  static constexpr auto UasortDict = &Uasort;
  static constexpr auto CopyDict = &Copy;
  static constexpr auto CopyWithStrongIteratorsDict = &CopyWithStrongIterators;
  static constexpr auto CopyStaticDict = &CopyStatic;
  static constexpr auto AppendDict = &Append;
  static constexpr auto LvalIntRefDict = &LvalIntRefHackArr;
  static constexpr auto LvalStrRefDict = &LvalStrRefHackArr;
  static constexpr auto LvalNewRefDict = &LvalNewRefHackArr;
  static constexpr auto SetRefIntDict = &SetRefIntHackArr;
  static constexpr auto SetRefStrDict = &SetRefStrHackArr;
  static constexpr auto AppendRefDict = &AppendRefHackArr;
  static constexpr auto AppendWithRefDict = &AppendWithRefHackArr;
  static constexpr auto PlusEqDict = &PlusEq;
  static constexpr auto MergeDict = &Merge;
  static constexpr auto PopDict = &Pop;
  static constexpr auto DequeueDict = &Dequeue;
  static constexpr auto PrependDict = &Prepend;
  static constexpr auto RenumberDict = &Renumber;
  static constexpr auto OnSetEvalScalarDict = &OnSetEvalScalar;
  static constexpr auto EscalateDict = &Escalate;
  static ArrayData* ToPHPArrayDict(ArrayData*, bool);
  static ArrayData* ToDictDict(ArrayData*, bool);
  static constexpr auto ToVecDict = &ArrayCommon::ToVec;
  static constexpr auto ToKeysetDict = &ArrayCommon::ToKeyset;

  //////////////////////////////////////////////////////////////////////

  // Like Lval[Int,Str], but silently does nothing if the element does not
  // exist. Not part of the ArrayData interface, but used for member operations.
  static ArrayData* LvalSilentInt(ArrayData*, int64_t, Variant*&, bool);
  static ArrayData* LvalSilentStr(ArrayData*, const StringData*,
                                  Variant*&, bool);

  static constexpr auto LvalSilentIntDict = &LvalSilentInt;
  static constexpr auto LvalSilentStrDict = &LvalSilentStr;

  //////////////////////////////////////////////////////////////////////

private:
  MixedArray* copyMixed() const;
  MixedArray* copyMixedAndResizeIfNeeded() const;
  MixedArray* copyMixedAndResizeIfNeededSlow() const;
  static ArrayData* MakeReserveImpl(uint32_t capacity, HeaderKind hk);

  static bool DictEqualHelper(const ArrayData*, const ArrayData*, bool);

public:
  // Elm's data.m_type == kInvalidDataType for deleted slots.
  static bool isTombstone(DataType t) {
    assert(isRealType(t) || t == kInvalidDataType);
    return t < KindOfUninit;
    static_assert(KindOfUninit == 0 && kInvalidDataType < 0, "");
  }

  // Element index, with special values < 0 used for hash tables.
  static constexpr int32_t Empty      = -1;
  static constexpr int32_t Tombstone  = -2;

  // Use a minimum of an 4-element hash table.  Valid range: [2..32]
  static constexpr uint32_t LgSmallScale = 0;
  static constexpr uint32_t SmallScale = 1 << LgSmallScale;
  static constexpr uint32_t SmallHashSize = SmallScale * 4;
  static constexpr uint32_t SmallMask = SmallHashSize - 1; // 3
  static constexpr uint32_t SmallSize = SmallScale * 3;

  static constexpr uint64_t MaxHashSize = uint64_t(1) << 32;
  static constexpr uint32_t MaxMask = MaxHashSize - 1;
  static constexpr uint32_t MaxSize = MaxMask - MaxMask / LoadScale;
  static constexpr uint32_t MaxMakeSize = 4 * SmallSize;
  static constexpr uint32_t MaxStructMakeSize = 64;

  uint32_t iterLimit() const { return m_used; }

  // Fetch a value and optional key (if keyPos != nullptr), given an
  // iterator pos.  If withref is true, copy the value with "withRef"
  // semantics, and decref the previous key before copying the key.
  // Otherwise get the value cell (unboxing), and initialize keyOut.
  void getArrayElm(ssize_t pos, TypedValue* out, TypedValue* keyOut) const;
  void getArrayElm(ssize_t pos, TypedValue* out) const;
  void dupArrayElmWithRef(ssize_t pos, TypedValue* valOut,
    TypedValue* keyOut) const;

  const TypedValue* getArrayElmPtr(ssize_t pos) const;
  TypedValue getArrayElmKey(ssize_t pos) const;

  bool isTombstone(ssize_t pos) const;

  size_t hashSize() const;
  size_t heapSize() const;
  static constexpr size_t computeMaxElms(uint32_t tableMask);
  static size_t computeAllocBytesFromMaxElms(uint32_t maxElms);

private:
  friend struct ArrayInit;
  friend struct MemoryProfile;
  friend struct EmptyArray;
  friend struct PackedArray;
  friend struct HashCollection;
  friend struct BaseMap;
  friend struct c_Map;
  friend struct c_ImmMap;
  friend struct BaseSet;
  friend struct c_Set;
  friend struct c_ImmSet;
  friend struct c_AwaitAllWaitHandle;
  enum class ClonePacked {};
  enum class CloneMixed {};

  friend size_t getMemSize(const ArrayData*);

public:
  // Safe downcast helpers
  static MixedArray* asMixed(ArrayData* ad) {
    assert(ad->hasMixedLayout());
    auto a = static_cast<MixedArray*>(ad);
    assert(a->checkInvariants());
    return a;
  }
  static const MixedArray* asMixed(const ArrayData* ad) {
    assert(ad->hasMixedLayout());
    auto a = static_cast<const MixedArray*>(ad);
    assert(a->checkInvariants());
    return a;
  }

  // Fast iteration
  template <class F, bool inc = true>
  static void IterateV(const MixedArray* arr, F fn) {
    assert(arr->hasMixedLayout());
    auto elm = arr->data();
    if (inc) arr->incRefCount();
    SCOPE_EXIT { if (inc) decRefArr(const_cast<MixedArray*>(arr)); };
    for (auto i = arr->m_used; i--; elm++) {
      if (LIKELY(!elm->isTombstone())) {
        if (ArrayData::call_helper(fn, &elm->data)) break;
      }
    }
  }
  template <class F, bool inc = true>
  static void IterateKV(const MixedArray* arr, F fn) {
    assert(arr->hasMixedLayout());
    auto elm = arr->data();
    if (inc) arr->incRefCount();
    SCOPE_EXIT { if (inc) decRefArr(const_cast<MixedArray*>(arr)); };
    for (auto i = arr->m_used; i--; elm++) {
      if (LIKELY(!elm->isTombstone())) {
        TypedValue key;
        key.m_data.num = elm->ikey;
        key.m_type = elm->hasIntKey() ? KindOfInt64 : KindOfString;
        if (ArrayData::call_helper(fn, &key, &elm->data)) break;
      }
    }
  }

private:
  static void getElmKey(const Elm& e, TypedValue* out);

private:
  enum class AllocMode : bool { Request, Static };

  static MixedArray* CopyMixed(const MixedArray& other, AllocMode, HeaderKind);
  static MixedArray* CopyReserve(const MixedArray* src, size_t expectedSize);

  MixedArray() = delete;
  MixedArray(const MixedArray&) = delete;
  MixedArray& operator=(const MixedArray&) = delete;
  ~MixedArray() = delete;

private:
  static void initHash(int32_t* table, uint32_t scale);
  static void copyHash(int32_t* to, const int32_t* from, uint32_t scale);
  // Copy elements as well as `m_nextKI' from one MixedArray to another.
  // Warning: it could copy up to 24 bytes beyond the array and thus overwrite
  // the hashtable, but it never reads/writes beyond the end of the hash
  // table.  If you use this function, make sure you copy/write the correct
  // data on the hash table afterwards.
  static void copyElmsNextUnsafe(MixedArray* to, const MixedArray* from,
                                 uint32_t nElems);

  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc, bool checkTypes);
  void postSort(bool resetKeys);

  static ArrayData* ArrayPlusEqGeneric(ArrayData*, MixedArray*,
                                       const ArrayData*, size_t);
  static ArrayData* ArrayMergeGeneric(MixedArray*, const ArrayData*);

  template <class AppendFunc>
  static ArrayData* AppendWithRefNoRef(ArrayData*, const Variant&,
                                       bool, AppendFunc);

  ssize_t nextElm(Elm* elms, ssize_t ei) const {
    assert(ei >= -1);
    while (size_t(++ei) < m_used) {
      if (!elms[ei].isTombstone()) {
        return ei;
      }
    }
    assert(ei == m_used);
    return ei;
  }

  ssize_t prevElm(Elm* elms, ssize_t ei) const;

  // Assert a bunch of invariants about this array then return true.
  // usage:  assert(checkInvariants());
  bool checkInvariants() const;

  template <class Hit>
  ssize_t findImpl(hash_t h0, Hit) const;

public:
  ssize_t find(int64_t ki, inthash_t h) const;
  ssize_t find(const StringData* s, strhash_t h) const;

private:
  // The array should already be sized for the new insertion before
  // calling these methods.
  template <class Hit>
  int32_t* findForInsertImpl(hash_t h0, Hit) const;
  int32_t* findForInsert(int64_t ki, inthash_t h) const;
  int32_t* findForInsert(const StringData* k, strhash_t h) const;

  struct InsertPos {
    InsertPos(bool found, TypedValue& tv) : found(found), tv(tv) {}
    bool found;
    TypedValue& tv;
  };
  InsertPos insert(int64_t k);
  InsertPos insert(StringData* k);

  template <class Hit, class Remove>
  ssize_t findForRemoveImpl(hash_t h0, Hit, Remove) const;
  ssize_t findForRemove(int64_t ki, inthash_t h, bool updateNext);
  ssize_t findForRemove(const StringData* k, strhash_t h);

  ssize_t iter_advance_helper(ssize_t prev) const;

  /**
   * findForNewInsert() CANNOT be used unless the caller can guarantee that
   * the relevant key is not already present in the array. Otherwise this can
   * put the array into a bad state; use with caution. The *CheckUnbalanced
   * version checks for the array becoming too unbalanced because of hash
   * collisions, and is only called when an array Grow()s.
   */
  int32_t* findForNewInsert(hash_t h0) const;
  int32_t* findForNewInsert(int32_t* table, size_t mask, hash_t h0) const;
  int32_t* findForNewInsertCheckUnbalanced(int32_t* table,
                                           size_t mask, hash_t h0);

  bool nextInsert(Cell);
  ArrayData* nextInsertRef(Variant& data);
  ArrayData* nextInsertWithRef(const Variant& data);
  ArrayData* addVal(int64_t ki, Cell data);
  ArrayData* addVal(StringData* key, Cell data);
  ArrayData* addValNoAsserts(StringData* key, Cell data);

  Elm& addKeyAndGetElem(StringData* key);

  template <class K> ArrayData* addLvalImpl(K k, Variant*& ret);
  template <class K> ArrayData* update(K k, Cell data);
  template <class K> ArrayData* updateRef(K k, Variant& data);

  template <class K> ArrayData* zSetImpl(K k, RefData* data);
  ArrayData* zAppendImpl(RefData* data, int64_t* key_ptr);

  void adjustMArrayIter(ssize_t pos);
  void eraseNoCompact(ssize_t pos);
  void erase(ssize_t pos) {
    eraseNoCompact(pos);
    if (m_size <= m_used / 2) {
      // Compact in order to keep elms from being overly sparse.
      compact(false);
    }
  }

  MixedArray* copyImpl(MixedArray* target) const;

  bool isFull() const;
  Elm& allocElm(int32_t* ei);

  MixedArray* getLval(TypedValue& tv, Variant*& ret);
  MixedArray* initRef(TypedValue& tv, Variant& v);
  MixedArray* initLval(TypedValue& tv, Variant*& ret);
  MixedArray* initWithRef(TypedValue& tv, const Variant& v);
  MixedArray* moveVal(TypedValue& tv, TypedValue v);

  ArrayData* zInitVal(TypedValue& tv, RefData* v);
  ArrayData* zSetVal(TypedValue& tv, RefData* v);

  /*
   * Helper routine for inserting elements into a new array
   * when Grow()ing the array, that also checks for potentially
   * unbalanced entries because of hash collision.
   */
  static MixedArray* InsertCheckUnbalanced(MixedArray* ad, int32_t* table,
                                           uint32_t mask,
                                           Elm* iter, Elm* stop);
  /*
   * grow() increases the hash table size and the number of slots for
   * elements by a factor of 2. grow() rebuilds the hash table, but it
   * does not compact the elements.
   */
  static MixedArray* Grow(MixedArray* old, uint32_t newScale);

  /**
   * compact() does not change the hash table size or the number of slots
   * for elements. compact() rebuilds the hash table and compacts the
   * elements into the slots with lower addresses.
   */
  void compact(bool renumber);

  /*
   * resize() and resizeIfNeeded() will grow or compact the array as
   * necessary to ensure that there is room for a new element and a
   * new hash entry.
   *
   * resize() assumes isFull().  resizeIfNeeded() will first check if
   * there is room for a new element and hash entry before growing or
   * compacting the array.
   *
   * Both functions return the new MixedArray* to use (or the old one
   * if they didn't need to grow).  The old MixedArray is left in a
   * zombie state where the only legal action is to decref and then
   * throw it away.
   */
  MixedArray* resize();
  MixedArray* resizeIfNeeded();

  Elm* data() const {
    return const_cast<Elm*>(reinterpret_cast<Elm const*>(this + 1));
  }

  int32_t* hashTab() const {
    return const_cast<int32_t*>(
      reinterpret_cast<int32_t const*>(
        data() + static_cast<size_t>(m_scale) * 3
      )
    );
  }

  uint32_t capacity() const { return Capacity(m_scale); }
  uint32_t mask() const { return Mask(m_scale); }
  uint32_t scale() const { return m_scale; }

  bool isZombie() const { return m_used + 1 == 0; }
  void setZombie() { m_used = -uint32_t{1}; }

public:
  template<class F> void scan(F&) const; // in mixed-array-defs.h

private:
  struct Initializer;
  static Initializer s_initializer;

  // Some of these are packed into qword-sized unions so we can
  // combine stores during initialization. (gcc won't do it on its own.)
  union {
    struct {
      uint32_t m_scale;     // size-class equal to 1/4 table size
      uint32_t m_used;      // Number of used elements (values or tombstones)
    };
    uint64_t m_scale_used;
  };
  int64_t  m_nextKI;        // Next integer key to use for append.
};

inline constexpr size_t MixedArray::computeMaxElms(uint32_t mask) {
  return size_t(mask) - size_t(mask) / LoadScale;
}

ALWAYS_INLINE constexpr size_t computeAllocBytes(uint32_t scale) {
  return sizeof(MixedArray) +
         MixedArray::HashSize(scale) * sizeof(int32_t) +
         MixedArray::Capacity(scale) * sizeof(MixedArray::Elm);
}

extern std::aligned_storage<
  computeAllocBytes(1),
  folly::constexpr_max(alignof(MixedArray), size_t(16))
>::type s_theEmptyDictArray;

ALWAYS_INLINE ArrayData* staticEmptyDictArray() {
  void* vp = &s_theEmptyDictArray;
  return static_cast<ArrayData*>(vp);
}

//////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_HPHP_ARRAY_H_
