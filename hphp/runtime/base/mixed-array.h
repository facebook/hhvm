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
#include "hphp/runtime/base/hash-table.h"
#include "hphp/runtime/base/member-val.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"

#include <folly/portability/Constexpr.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct APCArray;
struct APCHandle;
struct ArrayInit;
struct MemoryProfile;

//////////////////////////////////////////////////////////////////////
struct MixedArrayElm {
  using hash_t = strhash_t;

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

  void setStaticKey(StringData* k, strhash_t h) {
    assert(k->isStatic());
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
    assert(hasIntKey());
    static_assert(static_cast<int32_t>(STRHASH_MSB) < 0,
                  "high bit indicates int key");
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
    assert(hasStrKey());
    return skey;
  }

  ALWAYS_INLINE bool hasIntKey() const {
    return data.hash() < 0;
  }

  ALWAYS_INLINE int64_t intKey() const {
    return ikey;
  }

  ALWAYS_INLINE Cell getKey() const {
    if (hasIntKey()) {
      return make_tv<KindOfInt64>(ikey);
    }
    auto str = skey;
    if (str->isRefCounted()) {
      str->rawIncRefCount();
      return make_tv<KindOfString>(str);
    }
    return make_tv<KindOfPersistentString>(str);
  }

  ALWAYS_INLINE hash_t hash() const {
    return data.hash();
  }

  ALWAYS_INLINE bool isInvalid() const {
    return isTombstone();
  }

  // Elm's data.m_type == kInvalidDataType for deleted slots.
  ALWAYS_INLINE bool isTombstone() const {
    assert(isRealType(data.m_type) || data.m_type == kInvalidDataType);
    return data.m_type < KindOfUninit;
    static_assert(KindOfUninit == 0 && kInvalidDataType < 0, "");
  }

  static constexpr ptrdiff_t keyOff() {
    return offsetof(MixedArrayElm, ikey);
  }
  static constexpr ptrdiff_t dataOff() {
    return offsetof(MixedArrayElm, data);
  }
  static constexpr ptrdiff_t hashOff() {
    return offsetof(MixedArrayElm, data) + offsetof(TypedValue, m_aux);
  }
};

struct MixedArray final : ArrayData,
                          array::HashTable<MixedArray, MixedArrayElm>,
                          type_scan::MarkCountable<MixedArray> {
  /*
   * Iterator helper for kPackedKind and kMixedKind.  You can use this
   * to look at the values in the array, but not the keys unless you
   * know it is kMixedKind.
   *
   * This can be used as an optimization vs. ArrayIter, which uses
   * indirect calls in the loop.
   */
  struct ValIter;


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
  static ArrayData* MakeReserveDict(uint32_t size);
  static constexpr auto MakeReserve = &MakeReserveMixed;

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
   * Allocates a new request-local array with given key,value,key,value,... in
   * natural order. Returns nullptr if there are duplicate keys. Does not check
   * for integer-like keys. Takes ownership of keys and values iff successful.
   */
  static MixedArray* MakeMixed(uint32_t size, const TypedValue* keysAndValues);

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

  static bool DictEqual(const ArrayData*, const ArrayData*);
  static bool DictNotEqual(const ArrayData*, const ArrayData*);
  static bool DictSame(const ArrayData*, const ArrayData*);
  static bool DictNotSame(const ArrayData*, const ArrayData*);

  /*
   * Memoization interface.
   *
   * Both functions take a current base (which should either be a memoization
   * cache, or a RefData pointing to one), a pointer to a contiguous range of
   * keys to use for the lookup, and the length of the range. The length should
   * always be at least one.
   *
   * MemoGet will return the stored value corresponding to the keys, or
   * KindOfUninit if not found.
   *
   * MemoSet will store the given Cell at the location corresponding to the
   * keys, updating the base if the underlying dicts are mutated.
   */
  static Cell MemoGet(const TypedValue*, const Cell*, uint32_t);
  static void MemoSet(TypedValue*, const Cell*, uint32_t, Cell);

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
  using ArrayData::lvalNew;
  using ArrayData::set;
  using ArrayData::setRef;
  using ArrayData::add;
  using ArrayData::remove;
  using ArrayData::release;

public:
  static size_t Vsize(const ArrayData*);
  static member_rval::ptr_u GetValueRef(const ArrayData*, ssize_t pos);
  static bool IsVectorData(const ArrayData*);
  static bool IsStrictVector(const ArrayData* ad) {
    return ad->m_size == asMixed(ad)->m_nextKI && IsVectorData(ad);
  }
  static constexpr auto NvTryGetInt = &NvGetInt;
  static constexpr auto NvTryGetStr = &NvGetStr;
  static member_rval RvalIntStrict(const ArrayData* ad, int64_t k) {
    return member_rval { ad, NvTryGetInt(ad, k) };
  }
  static member_rval RvalStrStrict(const ArrayData* ad, const StringData* k) {
    return member_rval { ad, NvTryGetStr(ad, k) };
  }
  static member_rval RvalAtPos(const ArrayData* ad, ssize_t pos) {
    return member_rval { ad, GetValueRef(ad, pos) };
  }
  static bool ExistsInt(const ArrayData*, int64_t k);
  static bool ExistsStr(const ArrayData*, const StringData* k);
  static member_lval LvalInt(ArrayData* ad, int64_t k, bool copy);
  static member_lval LvalIntRef(ArrayData* ad, int64_t k, bool copy);
  static member_lval LvalStr(ArrayData* ad, StringData* k, bool copy);
  static member_lval LvalStrRef(ArrayData* ad, StringData* k, bool copy);
  static member_lval LvalNew(ArrayData*, bool copy);
  static member_lval LvalNewRef(ArrayData*, bool copy);
  static ArrayData* SetInt(ArrayData*, int64_t k, Cell v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, Cell v, bool copy);
  static ArrayData* SetWithRefInt(ArrayData*, int64_t k,
                                  TypedValue v, bool copy);
  static ArrayData* SetWithRefStr(ArrayData*, StringData* k,
                                  TypedValue v, bool copy);
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
  static ArrayData* CopyStatic(const ArrayData*);
  static ArrayData* Append(ArrayData*, Cell v, bool copy);
  static ArrayData* AppendRef(ArrayData*, Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, TypedValue v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Pop(ArrayData*, Variant& value);
  static ArrayData* Dequeue(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, Cell v, bool copy);
  static ArrayData* ToPHPArray(ArrayData*, bool);
  static ArrayData* ToDict(ArrayData*, bool);
  static constexpr auto ToVec = &ArrayCommon::ToVec;
  static constexpr auto ToKeyset = &ArrayCommon::ToKeyset;
  static constexpr auto ToVArray = &ArrayCommon::ToVArray;

  static void Renumber(ArrayData*);
  static void OnSetEvalScalar(ArrayData*);
  static void Release(ArrayData*);
  // Recursively register {allocation, rootAPCHandle} with APCGCManager
  static void RegisterUncountedAllocations(ArrayData* ad,
                                                APCHandle* rootAPCHandle);
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

  static member_rval::ptr_u NvTryGetIntDict(const ArrayData*, int64_t);
  static constexpr auto NvGetIntDict = &NvGetInt;
  static member_rval::ptr_u NvTryGetStrDict(const ArrayData*,
                                            const StringData*);
  static constexpr auto NvGetStrDict = &NvGetStr;
  static member_rval RvalIntDict(const ArrayData* ad, int64_t k) {
    return member_rval { ad, NvGetIntDict(ad, k) };
  }
  static member_rval RvalIntStrictDict(const ArrayData* ad, int64_t k) {
    return member_rval { ad, NvTryGetIntDict(ad, k) };
  }
  static member_rval RvalStrDict(const ArrayData* ad, const StringData* k) {
    return member_rval { ad, NvGetStrDict(ad, k) };
  }
  static member_rval RvalStrStrictDict(const ArrayData* ad,
                                       const StringData* k) {
    return member_rval { ad, NvTryGetStrDict(ad, k) };
  }
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
  static constexpr auto CopyStaticDict = &CopyStatic;
  static constexpr auto AppendDict = &Append;
  static member_lval LvalIntRefDict(ArrayData*, int64_t, bool);
  static member_lval LvalStrRefDict(ArrayData*, StringData*, bool);
  static member_lval LvalNewRefDict(ArrayData*, bool);
  static ArrayData* SetWithRefIntDict(ArrayData*, int64_t k,
                                      TypedValue v, bool copy);
  static ArrayData* SetWithRefStrDict(ArrayData*, StringData* k,
                                      TypedValue v, bool copy);
  static ArrayData* SetRefIntDict(ArrayData*, int64_t, Variant&, bool);
  static ArrayData* SetRefStrDict(ArrayData*, StringData*, Variant&, bool);
  static ArrayData* AppendRefDict(ArrayData*, Variant&, bool);
  static ArrayData* AppendWithRefDict(ArrayData*, TypedValue, bool);
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
  static constexpr auto ToVArrayDict = &ArrayCommon::ToVArray;

  //////////////////////////////////////////////////////////////////////

  // Like Lval[Int,Str], but silently does nothing if the element does not
  // exist. Not part of the ArrayData interface, but used for member operations.
  static member_lval LvalSilentInt(ArrayData*, int64_t, bool);
  static member_lval LvalSilentStr(ArrayData*, const StringData*, bool);

  static constexpr auto LvalSilentIntDict = &LvalSilentInt;
  static constexpr auto LvalSilentStrDict = &LvalSilentStr;

  //////////////////////////////////////////////////////////////////////

private:
  MixedArray* copyMixed() const;
  static ArrayData* MakeReserveImpl(uint32_t capacity, HeaderKind hk);

  static bool DictEqualHelper(const ArrayData*, const ArrayData*, bool);

public:

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
  // Elm's data.m_type == kInvalidDataType for deleted slots.
  static bool isTombstone(DataType t) {
    assert(isRealType(t) || t == kInvalidDataType);
    return t < KindOfUninit;
    static_assert(KindOfUninit == 0 && kInvalidDataType < 0, "");
  }

private:
  friend struct array::HashTable<MixedArray, MixedArrayElm>;
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
  template <typename AccessorT, class ArrayT>
  friend SortFlavor genericPreSort(ArrayT&, const AccessorT&, bool);

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
        if (ArrayData::call_helper(fn, elm->data)) break;
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
        if (ArrayData::call_helper(fn, key, elm->data)) break;
      }
    }
  }

private:
  static Cell getElmKey(const Elm& e);

private:
  enum class AllocMode : bool { Request, Static };

  static MixedArray* CopyMixed(const MixedArray& other, AllocMode, HeaderKind);
  static MixedArray* CopyReserve(const MixedArray* src, size_t expectedSize);

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

  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc, bool checkTypes);
  void postSort(bool resetKeys);

  static ArrayData* ArrayPlusEqGeneric(ArrayData*, MixedArray*,
                                       const ArrayData*, size_t);
  static ArrayData* ArrayMergeGeneric(MixedArray*, const ArrayData*);

  // Assert a bunch of invariants about this array then return true.
  // usage:  assert(checkInvariants());
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

  bool nextInsert(Cell);
  ArrayData* nextInsertRef(Variant& data);
  ArrayData* nextInsertWithRef(TypedValue data);
  ArrayData* nextInsertWithRef(const Variant& data);
  ArrayData* addVal(int64_t ki, Cell data);
  ArrayData* addVal(StringData* key, Cell data);
  ArrayData* addValNoAsserts(StringData* key, Cell data);

  Elm& addKeyAndGetElem(StringData* key);

  template <bool warn, class K> member_lval addLvalImpl(K k);
  template <class K> ArrayData* update(K k, Cell data);
  template <class K> ArrayData* updateWithRef(K k, TypedValue data);
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

  MixedArray* initRef(TypedValue& tv, Variant& v);
  MixedArray* initWithRef(TypedValue& tv, TypedValue v);
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

  int64_t  m_nextKI;        // Next integer key to use for append.
};

ALWAYS_INLINE Array empty_dict_array() {
  return Array::attach(staticEmptyDictArray());
}

HASH_TABLE_CHECK_OFFSETS(MixedArray, MixedArrayElm)
//////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_HPHP_ARRAY_H_
