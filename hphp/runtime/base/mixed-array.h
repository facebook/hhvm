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

#ifndef incl_HPHP_HPHP_ARRAY_H_
#define incl_HPHP_HPHP_ARRAY_H_

#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/data-walker.h"
#include "hphp/runtime/base/hash-table.h"
#include "hphp/runtime/base/mixed-array-keys.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"

#include <folly/portability/Constexpr.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct APCArray;
struct APCHandle;
struct MemoryProfile;

//////////////////////////////////////////////////////////////////////
struct MixedArrayElm {
  using hash_t = strhash_t;

  // We store values here, but also some information local to this array:
  // data.m_aux.u_hash contains either a negative number (for an int key) or
  // a string hashcode (31-bit and thus non-negative); the high bit is the
  // int/string key descriminator. data.m_type == kInvalidDataType if this is
  // an empty slot in the array (e.g. after a key is deleted).  It is
  // critical that when we return &data to clients, that they not read or
  // write the m_aux field!
  TypedValueAux data;

  // Putting the key second lets us cast MixedArrayElm* to TypedValue*, which
  // we can use to simplify the Lval sublattice in the JIT type system.
  union {
    int64_t ikey;
    StringData* skey;
  };

  void setStaticKey(StringData* k, strhash_t h) {
    assertx(k->isStatic());
    setStrKeyNoIncRef(k, h);
  }

  void setStrKeyNoIncRef(StringData* k, strhash_t h) {
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
    data.hash() = static_cast<int32_t>(h) | STRHASH_MSB;
    assertx(hasIntKey());
    static_assert(static_cast<int32_t>(STRHASH_MSB) < 0,
                  "high bit indicates int key");
  }

  void setTombstone() {
    // We don't explicitly check for tombstones in MixedArray::release(),
    // instead make the deleted element appear to contain uncounted values.
    data.m_type = kInvalidDataType;
    static_assert(!isRefcountedType(kInvalidDataType), "");
    data.hash() = -1;
    assertx(hasIntKey());
  }

  hash_t probe() const {
    return hash();
  }

  TYPE_SCAN_CUSTOM() {
    static_assert(!isRefcountedType(kInvalidDataType), "");
    // if data is a Tombstone, key should have been set to int type.
    if (hasStrKey()) scanner.scan(skey);
    if (isRefcountedType(data.m_type)) scanner.scan(data.m_data.pcnt);
  }

  // Members below here are required for HashTable implemenation.
  ALWAYS_INLINE const TypedValue* datatv() const {
    return &data;
  }

  ALWAYS_INLINE bool hasStrKey() const {
    // Currently string hash is 31-bit, thus it saves us some instructions to
    // encode int keys as a negative hash, so that we don't have to care about
    // the MSB when working with strhash_t.
    return data.hash() >= 0;
  }

  ALWAYS_INLINE StringData* strKey() const {
    assertx(hasStrKey());
    return skey;
  }

  ALWAYS_INLINE bool hasIntKey() const {
    return data.hash() < 0;
  }

  ALWAYS_INLINE int64_t intKey() const {
    return ikey;
  }

  ALWAYS_INLINE TypedValue getKey() const {
    if (hasIntKey()) {
      return make_tv<KindOfInt64>(ikey);
    }
    return skey->isRefCounted() ? make_tv<KindOfString>(skey)
                                : make_tv<KindOfPersistentString>(skey);
  }

  ALWAYS_INLINE hash_t hash() const {
    return data.hash();
  }

  ALWAYS_INLINE bool isInvalid() const {
    return isTombstone();
  }

  // Elm's data.m_type == kInvalidDataType for deleted slots.
  ALWAYS_INLINE bool isTombstone() const {
    assertx(isRealType(data.m_type) || data.m_type == kInvalidDataType);
    return data.m_type == kInvalidDataType;
  }

  static constexpr ptrdiff_t keyOff() {
    return offsetof(MixedArrayElm, ikey);
  }
  static constexpr ptrdiff_t dataOff() {
    return offsetof(MixedArrayElm, data) + offsetof(TypedValue, m_data);
  }
  static constexpr ptrdiff_t typeOff() {
    return offsetof(MixedArrayElm, data) + offsetof(TypedValue, m_type);
  }
  static constexpr ptrdiff_t hashOff() {
    return offsetof(MixedArrayElm, data) + offsetof(TypedValue, m_aux);
  }
};

struct MixedArray final : ArrayData,
                          array::HashTable<MixedArray, MixedArrayElm>,
                          type_scan::MarkCollectable<MixedArray> {
  struct ElmKey {
    ElmKey() {}
    ElmKey(strhash_t hash, StringData* key)
        : skey(key), hash(hash)
      {}
    union {
      StringData* skey;
      int64_t ikey;
    };
    int32_t hash;

    TYPE_SCAN_CUSTOM_FIELD(skey) {
      if (hash >= 0) scanner.scan(skey);
    }
  };

  /*
   * Initialize an empty small mixed array with given field. This should be
   * inlined.
   */
  static void InitSmall(MixedArray* a, uint32_t size, int64_t nextIntKey);

  /*
   * Allocate a new, empty, request-local array in (mixed|dict) mode, with
   * enough space reserved for `capacity' members.
   *
   * The returned array is already incref'd.
   */
  static ArrayData* MakeReserveMixed(uint32_t size);
  static ArrayData* MakeReserveDArray(uint32_t size);
  static ArrayData* MakeReserveDict(uint32_t size);
  static constexpr auto MakeReserve = &MakeReserveMixed;

  /*
   * Convert mixed-layout array to dict in-place. This function doesn't check
   * whether the input array contains references or not, so only use this when
   * you already know that they do not.
   */
  static MixedArray* ToDictInPlace(ArrayData*);

  /*
   * MakeReserveLike will return a PHP array with a memory representation
   * similar to the one used by `other'.
   *
   * The returned array is already incref'd.
   */
  static ArrayData* MakeReserveLike(const ArrayData* other, uint32_t capacity);

  /*
   * Allocates a new request-local array with given key,value,key,value,... in
   * natural order. Returns nullptr if there are duplicate keys. Does not check
   * for integer-like keys. Takes ownership of keys and values iff successful.
   */
  static MixedArray* MakeMixed(uint32_t size, const TypedValue* kvs);
  static MixedArray* MakeDArray(uint32_t size, const TypedValue* kvs);
  static MixedArray* MakeDict(uint32_t size, const TypedValue* kvs);
private:
  template<HeaderKind hdr, ArrayData::DVArray dv>
  static MixedArray* MakeMixedImpl(uint32_t size, const TypedValue* kvs);

public:
  static constexpr size_t kKeyTypesOffset = 7;

  const MixedArrayKeys& keyTypes() const {
    auto const pointer = uintptr_t(this) + kKeyTypesOffset;
    return *reinterpret_cast<MixedArrayKeys*>(pointer);
  }

  MixedArrayKeys* mutableKeyTypes() {
    auto const pointer = uintptr_t(this) + kKeyTypesOffset;
    return reinterpret_cast<MixedArrayKeys*>(pointer);
  }

public:
  /*
   * Same semantics as PackedArray::MakeNatural().
   */
  static MixedArray* MakeDArrayNatural(uint32_t size, const TypedValue* vals);

  /*
   * Like MakePacked, but given static strings, make a struct-like array.
   * Also requires size > 0.
   */
  static MixedArray* MakeStruct(uint32_t size, const StringData* const* keys,
                               const TypedValue* values);
  static MixedArray* MakeStructDict(uint32_t size,
                                    const StringData* const* keys,
                                    const TypedValue* values);
  static MixedArray* MakeStructDArray(uint32_t size,
                                      const StringData* const* keys,
                                      const TypedValue* values);

  /*
   * Allocate a struct-like array (with string literal keys), but only init
   * the hash table and the header, leaving elms uninit. Requires size > 0.
   */
  static MixedArray* AllocStruct(uint32_t size, const int32_t* hash);
  static MixedArray* AllocStructDict(uint32_t size, const int32_t* hash);
  static MixedArray* AllocStructDArray(uint32_t size, const int32_t* hash);

  /*
   * Allocate an uncounted MixedArray and copy the values from the
   * input 'array' into the uncounted one. All values copied are made
   * uncounted as well.  An uncounted array can only contain uncounted
   * values (primitive values, uncounted or static strings and
   * uncounted or static arrays).  The Packed version does the same
   * when the array has a kPackedKind.
   *
   * If withApcTypedValue is true, space for an APCTypedValue will be
   * allocated in front of the returned pointer.
   */
  static ArrayData* MakeUncounted(
      ArrayData* array, bool withApcTypedValue,
      DataWalker::PointerMap* seen = nullptr
  );
  static ArrayData* MakeUncounted(
      ArrayData* array, int, DataWalker::PointerMap* seen = nullptr
  ) = delete;
  static ArrayData* MakeUncounted(
      ArrayData* array, size_t extra, DataWalker::PointerMap* seen = nullptr
  ) = delete;

  static ArrayData* MakeDictFromAPC(const APCArray* apc);
  static ArrayData* MakeDArrayFromAPC(const APCArray* apc);

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
  using ArrayData::at;
  using ArrayData::rval;
  using ArrayData::lval;
  using ArrayData::set;
  using ArrayData::remove;
  using ArrayData::release;

public:
  static size_t Vsize(const ArrayData*);
  static TypedValue GetPosVal(const ArrayData*, ssize_t pos);
  static bool IsVectorData(const ArrayData*);
  static bool IsStrictVector(const ArrayData* ad) {
    return ad->m_size == asMixed(ad)->m_nextKI && IsVectorData(ad);
  }
  static bool ExistsInt(const ArrayData*, int64_t k);
  static bool ExistsStr(const ArrayData*, const StringData* k);
  static arr_lval LvalInt(ArrayData* ad, int64_t k, bool copy);
  static arr_lval LvalStr(ArrayData* ad, StringData* k, bool copy);
  static ArrayData* SetInt(ArrayData*, int64_t k, TypedValue v);
  static ArrayData* SetIntMove(ArrayData*, int64_t k, TypedValue v);
  static ArrayData* SetStr(ArrayData*, StringData* k, TypedValue v);
  static ArrayData* SetStrMove(ArrayData*, StringData* k, TypedValue v);
  static ArrayData* AddInt(ArrayData*, int64_t k, TypedValue v, bool copy);
  static ArrayData* AddStr(ArrayData*, StringData* k, TypedValue v, bool copy);
  static ArrayData* RemoveInt(ArrayData*, int64_t k);
  static ArrayData* RemoveStr(ArrayData*, const StringData* k);
  static ArrayData* Copy(const ArrayData*);
  static ArrayData* CopyStatic(const ArrayData*);
  static ArrayData* Append(ArrayData*, TypedValue v);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Pop(ArrayData*, Variant& value);
  static ArrayData* Dequeue(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, TypedValue v);
  static ArrayData* ToPHPArray(ArrayData*, bool);
  static ArrayData* ToPHPArrayIntishCast(ArrayData*, bool);
  static ArrayData* ToDict(ArrayData*, bool);
  static constexpr auto ToVec = &ArrayCommon::ToVec;
  static constexpr auto ToKeyset = &ArrayCommon::ToKeyset;
  static constexpr auto ToVArray = &ArrayCommon::ToVArray;
  static ArrayData* ToDArray(ArrayData*, bool);

  static void Renumber(ArrayData*);
  static void OnSetEvalScalar(ArrayData*);
  static void Release(ArrayData*);
  // Recursively register {allocation, rootAPCHandle} with APCGCManager
  static void RegisterUncountedAllocations(ArrayData* ad,
                                           APCHandle* rootAPCHandle);
  static void ReleaseUncounted(ArrayData*);

  static ArrayData* EscalateForSort(ArrayData* ad, SortFunction sf);
  static void Ksort(ArrayData*, int sort_flags, bool ascending);
  static void Sort(ArrayData*, int sort_flags, bool ascending);
  static void Asort(ArrayData*, int sort_flags, bool ascending);
  static bool Uksort(ArrayData*, const Variant& cmp_function);
  static bool Usort(ArrayData*, const Variant& cmp_function);
  static bool Uasort(ArrayData*, const Variant& cmp_function);

  static constexpr auto NvGetIntDict = &NvGetInt;
  static constexpr auto NvGetStrDict = &NvGetStr;
  static constexpr auto NvGetIntPosDict = &NvGetIntPos;
  static constexpr auto NvGetStrPosDict = &NvGetStrPos;
  static constexpr auto ReleaseDict = &Release;
  static constexpr auto GetPosKeyDict = &GetPosKey;
  static constexpr auto GetPosValDict = &GetPosVal;
  static constexpr auto SetIntDict = &SetInt;
  static constexpr auto SetIntMoveDict = &SetIntMove;
  static constexpr auto SetStrDict = &SetStr;
  static constexpr auto SetStrMoveDict = &SetStrMove;
  static constexpr auto AddIntDict = &AddInt;
  static constexpr auto AddStrDict = &AddStr;
  static constexpr auto VsizeDict = &Vsize;
  static constexpr auto IsVectorDataDict = &IsVectorData;
  static constexpr auto ExistsIntDict = &ExistsInt;
  static constexpr auto ExistsStrDict = &ExistsStr;
  static constexpr auto LvalIntDict = &LvalInt;
  static constexpr auto LvalStrDict = &LvalStr;
  static constexpr auto RemoveIntDict = &RemoveInt;
  static constexpr auto RemoveStrDict = &RemoveStr;
  static constexpr auto IterBeginDict = &IterBegin;
  static constexpr auto IterLastDict = &IterLast;
  static constexpr auto IterEndDict = &IterEnd;
  static constexpr auto IterAdvanceDict = &IterAdvance;
  static constexpr auto IterRewindDict = &IterRewind;
  static constexpr auto EscalateForSortDict = &EscalateForSort;
  static constexpr auto KsortDict = &Ksort;
  static constexpr auto SortDict = &Sort;
  static constexpr auto AsortDict = &Asort;
  static constexpr auto UksortDict = &Uksort;
  static constexpr auto UsortDict = &Usort;
  static constexpr auto UasortDict = &Uasort;
  static constexpr auto CopyDict = &Copy;
  static constexpr auto CopyStaticDict = &CopyStatic;
  static constexpr auto AppendDict = &Append;
  static constexpr auto PlusEqDict = &PlusEq;
  static constexpr auto MergeDict = &Merge;
  static constexpr auto PopDict = &Pop;
  static constexpr auto DequeueDict = &Dequeue;
  static constexpr auto PrependDict = &Prepend;
  static constexpr auto RenumberDict = &Renumber;
  static constexpr auto OnSetEvalScalarDict = &OnSetEvalScalar;
  static ArrayData* ToPHPArrayDict(ArrayData*, bool);
  static ArrayData* ToPHPArrayIntishCastDict(ArrayData*, bool);
  static ArrayData* ToDictDict(ArrayData*, bool);
  static constexpr auto ToVecDict = &ArrayCommon::ToVec;
  static constexpr auto ToKeysetDict = &ArrayCommon::ToKeyset;
  static constexpr auto ToVArrayDict = &ArrayCommon::ToVArray;
  static ArrayData* ToDArrayDict(ArrayData*, bool);

  //////////////////////////////////////////////////////////////////////

  // Helpers used by ArrayInit to update MixedArrays without refcount ops.
  // These helpers work for both mixed PHP arrays and dicts.
  static ArrayData* SetIntInPlace(ArrayData*, int64_t k, TypedValue v);
  static ArrayData* SetStrInPlace(ArrayData*, StringData* k, TypedValue v);

  // Return an lval to the value at the given key. If `k` is not present,
  // this method will insert a null value for it and return it.
  //  @precondition: !isFull
  //  @precondition: !cowCheck
  static tv_lval LvalInPlace(ArrayData* ad, const Variant& k);

  // If the key is not present in the array, return a null lval; else,
  // COW the array if necessary and return an lval into the new array.
  static arr_lval LvalSilentInt(ArrayData* ad, int64_t k);
  static arr_lval LvalSilentStr(ArrayData* ad, StringData* k);

  //////////////////////////////////////////////////////////////////////

private:
  MixedArray* copyMixed() const;
  static ArrayData* MakeReserveImpl(uint32_t capacity, HeaderKind hk,
                                    ArrayData::DVArray);
  static MixedArray* MakeStructImpl(uint32_t, const StringData* const*,
                                    const TypedValue*, HeaderKind,
                                    ArrayData::DVArray);
  static MixedArray* AllocStructImpl(uint32_t, const int32_t*,
                                    HeaderKind, ArrayData::DVArray);

  template <IntishCast IC>
  static ArrayData* FromDictImpl(ArrayData*, bool, bool);

  static bool DictEqualHelper(const ArrayData*, const ArrayData*, bool);

public:

  uint32_t iterLimit() const { return m_used; }

  // Fetch a value and optional key (if keyPos != nullptr), given an
  // iterator pos.  Get the value cell, and initialize keyOut.
  void getArrayElm(ssize_t pos, TypedValue* out, TypedValue* keyOut) const;
  void getArrayElm(ssize_t pos, TypedValue* out) const;

  const TypedValue* getArrayElmPtr(ssize_t pos) const;
  TypedValue getArrayElmKey(ssize_t pos) const;

  bool isTombstone(ssize_t pos) const;
  // Elm's data.m_type == kInvalidDataType for deleted slots.
  static bool isTombstone(DataType t) {
    return t == kInvalidDataType;
  }

private:
  friend struct array::HashTable<MixedArray, MixedArrayElm>;
  friend struct MemoryProfile;
  friend struct EmptyArray;
  friend struct PackedArray;
  friend struct RecordArray;
  friend struct HashCollection;
  friend struct BaseMap;
  friend struct c_Map;
  friend struct c_ImmMap;
  friend struct BaseSet;
  friend struct c_Set;
  friend struct c_ImmSet;
  friend struct c_AwaitAllWaitHandle;

  friend size_t getMemSize(const ArrayData*, bool);
  template <typename AccessorT, class ArrayT>
  friend SortFlavor genericPreSort(ArrayT&, const AccessorT&, bool);

public:
  // Safe downcast helpers
  static MixedArray* asMixed(ArrayData* ad) {
    assertx(ad->hasVanillaMixedLayout());
    auto a = static_cast<MixedArray*>(ad);
    assertx(a->checkInvariants());
    return a;
  }
  static const MixedArray* asMixed(const ArrayData* ad) {
    assertx(ad->hasVanillaMixedLayout());
    auto a = static_cast<const MixedArray*>(ad);
    assertx(a->checkInvariants());
    return a;
  }

  // Fast iteration
  template <class F, bool inc = true>
  static void IterateV(const MixedArray* arr, F fn) {
    assertx(arr->hasVanillaMixedLayout());
    auto elm = arr->data();
    if (inc) arr->incRefCount();
    SCOPE_EXIT { if (inc) decRefArr(const_cast<MixedArray*>(arr)); };
    for (auto i = arr->m_used; i--; elm++) {
      if (LIKELY(!elm->isTombstone())) {
        if (ArrayData::call_helper(fn, elm->data)) break;
      }
    }
  }
  template <class F, bool inc = true>
  static void IterateKV(const MixedArray* arr, F fn) {
    assertx(arr->hasVanillaMixedLayout());
    auto elm = arr->data();
    if (inc) arr->incRefCount();
    SCOPE_EXIT { if (inc) decRefArr(const_cast<MixedArray*>(arr)); };
    for (auto i = arr->m_used; i--; elm++) {
      if (LIKELY(!elm->isTombstone())) {
        TypedValue key;
        key.m_data.num = elm->ikey;
        key.m_type = elm->hasIntKey() ? KindOfInt64 : KindOfString;
        if (ArrayData::call_helper(fn, key, elm->data)) break;
      }
    }
  }

private:
  static TypedValue getElmKey(const Elm& e);

private:
  enum class AllocMode : bool { Request, Static };

  static MixedArray* CopyMixed(const MixedArray& other, AllocMode,
                               HeaderKind, ArrayData::DVArray);
  static MixedArray* CopyReserve(const MixedArray* src, size_t expectedSize);

  // Slow paths used for MixedArrays with references or counted string keys.
  // We fall back to SlowCopy and SlowGrow in the middle of iteration when we
  // encounter a reference, so we take an (elm, end) pair as arguments.
  static MixedArray* SlowCopy(MixedArray*, const ArrayData& old,
                              MixedArrayElm* elm, MixedArrayElm* end);
  static MixedArray* SlowGrow(MixedArray*, const ArrayData& old,
                              MixedArrayElm* elm, MixedArrayElm* end);
  static void SlowRelease(MixedArray*);

  MixedArray() = delete;
  MixedArray(const MixedArray&) = delete;
  MixedArray& operator=(const MixedArray&) = delete;
  ~MixedArray() = delete;

private:
  // Copy elements as well as `m_nextKI' from one MixedArray to another.
  // Warning: it could copy up to 24 bytes beyond the array and thus overwrite
  // the hashtable, but it never reads/writes beyond the end of the hash
  // table.  If you use this function, make sure you copy/write the correct
  // data on the hash table afterwards.
  static void copyElmsNextUnsafe(MixedArray* to, const MixedArray* from,
                                 uint32_t nElems);

  /*
   * Copy this from adIn, intish casting all the intish string keys in
   * accordance with the value of the intishCast template parameter
   */
  template <IntishCast IC>
  static ArrayData* copyWithIntishCast(MixedArray* adIn, bool asDArray = false);

  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc, bool checkTypes);
  void postSort(bool resetKeys);

  static ArrayData* ArrayPlusEqGeneric(ArrayData*, MixedArray*,
                                       const ArrayData*, size_t);
  static ArrayData* ArrayMergeGeneric(MixedArray*, const ArrayData*);

  // Assert a bunch of invariants about this array then return true.
  // usage:  assertx(checkInvariants());
  bool checkInvariants() const;

private:
  // The array should already be sized for the new insertion before
  // calling these methods.
  struct InsertPos {
    InsertPos(bool found, TypedValue& tv) : found(found), tv(tv) {}
    bool found;
    TypedValue& tv;
  };
  InsertPos insert(int64_t k);
  InsertPos insert(StringData* k);

  using HashTable<MixedArray, MixedArrayElm>::findForRemove;
  int32_t findForRemove(int64_t ki, inthash_t h, bool updateNext);
  int32_t findForRemove(const StringData* s, strhash_t h);

  static ArrayData* RemoveIntImpl(ArrayData*, int64_t, bool);
  static ArrayData* RemoveStrImpl(ArrayData*, const StringData*, bool);
  static ArrayData* AppendImpl(ArrayData*, TypedValue v, bool copy);

  void nextInsert(TypedValue);
  ArrayData* addVal(int64_t ki, TypedValue data);
  ArrayData* addVal(StringData* key, TypedValue data);
  ArrayData* addValNoAsserts(StringData* key, TypedValue data);

  template <bool warn, class K> arr_lval addLvalImpl(K k);
  // If "move" is false, this method will inc-ref data.
  template <class K, bool move = false> ArrayData* update(K k, TypedValue data);

  void eraseNoCompact(ssize_t pos);
  void erase(ssize_t pos) {
    eraseNoCompact(pos);
    if (m_size <= m_used / 2) {
      // Compact in order to keep elms from being overly sparse.
      compact(false);
    }
  }

  MixedArray* copyImpl(MixedArray* target) const;

  bool hasIntishKeys() const;

  MixedArray* moveVal(TypedValue& tv, TypedValue v);

  /*
   * Helper routine for inserting elements into a new array
   * when Grow()ing the array, that also checks for potentially
   * unbalanced entries because of hash collision.
   */
  static MixedArray* InsertCheckUnbalanced(MixedArray* ad, int32_t* table,
                                           uint32_t mask,
                                           Elm* iter, Elm* stop);
  /*
   * Grow makes a copy of the array with scale = newScale. Grow rebuilds the
   * hash table, but it does not compact the elements. If copy is true, it
   * will copy elements instead of taking ownership of them.
   */
  static MixedArray* Grow(MixedArray* old, uint32_t newScale, bool copy);

  /*
   * prepareForInsert ensures that the array has room to insert an element and
   * has a refcount of 1, copying if requested and growing if needed.
   */
  MixedArray* prepareForInsert(bool copy);

  /**
   * compact() does not change the hash table size or the number of slots
   * for elements. compact() rebuilds the hash table and compacts the
   * elements into the slots with lower addresses.
   */
  void compact(bool renumber);

  bool isZombie() const { return m_used + 1 == 0; }
  void setZombie() { m_used = -uint32_t{1}; }

public:
  void scan(type_scan::Scanner&) const; // in mixed-array-defs.h

private:
  struct Initializer;
  static Initializer s_initializer;

  struct DArrayInitializer;
  static DArrayInitializer s_darr_initializer;

  int64_t  m_nextKI;        // Next integer key to use for append.
};

HASH_TABLE_CHECK_OFFSETS(MixedArray, MixedArrayElm)
//////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_HPHP_ARRAY_H_
