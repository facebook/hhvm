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
  static ArrayData* MakeReserveDArray(uint32_t size);
  static ArrayData* MakeReserveDict(uint32_t size);
  static ArrayData* MakeReserveShape(uint32_t size);
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
  static MixedArray* MakeStructDict(uint32_t size,
                                    const StringData* const* keys,
                                    const TypedValue* values);
  static MixedArray* MakeStructDArray(uint32_t size,
                                      const StringData* const* keys,
                                      const TypedValue* values);

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
  static ArrayData* MakeShapeFromAPC(const APCArray* apc);

  static bool DictEqual(const ArrayData*, const ArrayData*);
  static bool DictNotEqual(const ArrayData*, const ArrayData*);
  static bool DictSame(const ArrayData*, const ArrayData*);
  static bool DictNotSame(const ArrayData*, const ArrayData*);

  static bool ShapeEqual(const ArrayData*, const ArrayData*);
  static bool ShapeNotEqual(const ArrayData*, const ArrayData*);
  static bool ShapeSame(const ArrayData*, const ArrayData*);
  static bool ShapeNotSame(const ArrayData*, const ArrayData*);
  static bool ShapeGt(const ArrayData*, const ArrayData*);
  static bool ShapeGte(const ArrayData*, const ArrayData*);
  static bool ShapeLt(const ArrayData*, const ArrayData*);
  static bool ShapeLte(const ArrayData*, const ArrayData*);
  static bool ShapeCompare(const ArrayData*, const ArrayData*);

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
  using ArrayData::remove;
  using ArrayData::release;

public:
  static size_t Vsize(const ArrayData*);
  static tv_rval GetValueRef(const ArrayData*, ssize_t pos);
  static bool IsVectorData(const ArrayData*);
  static bool IsStrictVector(const ArrayData* ad) {
    return ad->m_size == asMixed(ad)->m_nextKI && IsVectorData(ad);
  }
  static constexpr auto NvTryGetInt = &NvGetInt;
  static constexpr auto NvTryGetStr = &NvGetStr;
  static tv_rval RvalIntStrict(const ArrayData* ad, int64_t k) {
    assertx(ad->isMixed());
    return NvTryGetInt(ad, k);
  }
  static tv_rval RvalStrStrict(const ArrayData* ad, const StringData* k) {
    assertx(ad->isMixed());
    return NvTryGetStr(ad, k);
  }
  static tv_rval RvalAtPos(const ArrayData* ad, ssize_t pos) {
    assertx(ad->isMixed());
    return GetValueRef(ad, pos);
  }
  static bool ExistsInt(const ArrayData*, int64_t k);
  static bool ExistsStr(const ArrayData*, const StringData* k);
  static arr_lval LvalInt(ArrayData* ad, int64_t k, bool copy);
  static arr_lval LvalIntRef(ArrayData* ad, int64_t k, bool copy);
  static arr_lval LvalStr(ArrayData* ad, StringData* k, bool copy);
  static arr_lval LvalStrRef(ArrayData* ad, StringData* k, bool copy);
  static arr_lval LvalNew(ArrayData*, bool copy);
  static arr_lval LvalNewRef(ArrayData*, bool copy);
  static ArrayData* SetInt(ArrayData*, int64_t k, Cell v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, Cell v, bool copy);
  static ArrayData* SetWithRefInt(ArrayData*, int64_t k,
                                  TypedValue v, bool copy);
  static ArrayData* SetWithRefStr(ArrayData*, StringData* k,
                                  TypedValue v, bool copy);
  static ArrayData* SetRefInt(ArrayData* ad, int64_t k,
                              tv_lval v, bool copy);
  static ArrayData* SetRefStr(ArrayData* ad, StringData* k,
                              tv_lval v, bool copy);
  static ArrayData* AddInt(ArrayData*, int64_t k, Cell v, bool copy);
  static ArrayData* AddStr(ArrayData*, StringData* k, Cell v, bool copy);
  static ArrayData* RemoveInt(ArrayData*, int64_t k, bool copy);
  static ArrayData* RemoveStr(ArrayData*, const StringData* k, bool copy);
  static ArrayData* Copy(const ArrayData*);
  static ArrayData* CopyStatic(const ArrayData*);
  static ArrayData* Append(ArrayData*, Cell v, bool copy);
  static ArrayData* AppendRef(ArrayData*, tv_lval v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, TypedValue v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Pop(ArrayData*, Variant& value);
  static ArrayData* Dequeue(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, Cell v);
  static ArrayData* ToPHPArray(ArrayData*, bool);
  static ArrayData* ToDict(ArrayData*, bool);
  static ArrayData* ToShape(ArrayData*, bool);
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

  static tv_rval NvTryGetIntDict(const ArrayData*, int64_t);
  static constexpr auto NvGetIntDict = &NvGetInt;
  static tv_rval NvTryGetStrDict(const ArrayData*,
                                            const StringData*);
  static constexpr auto NvGetStrDict = &NvGetStr;
  static tv_rval RvalIntDict(const ArrayData* ad, int64_t k) {
    assertx(ad->isDictOrShape());
    return NvGetIntDict(ad, k);
  }
  static tv_rval RvalIntStrictDict(const ArrayData* ad, int64_t k) {
    assertx(ad->isDictOrShape());
    return NvTryGetIntDict(ad, k);
  }
  static tv_rval RvalStrDict(const ArrayData* ad, const StringData* k) {
    assertx(ad->isDictOrShape());
    return NvGetStrDict(ad, k);
  }
  static tv_rval RvalStrStrictDict(const ArrayData* ad,
                                       const StringData* k) {
    assertx(ad->isDictOrShape());
    return NvTryGetStrDict(ad, k);
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
  static arr_lval LvalIntRefDict(ArrayData*, int64_t, bool);
  static arr_lval LvalStrRefDict(ArrayData*, StringData*, bool);
  static arr_lval LvalNewRefDict(ArrayData*, bool);
  static ArrayData* SetWithRefIntDict(ArrayData*, int64_t k,
                                      TypedValue v, bool copy);
  static ArrayData* SetWithRefStrDict(ArrayData*, StringData* k,
                                      TypedValue v, bool copy);
  static ArrayData* SetRefIntDict(ArrayData*, int64_t, tv_lval, bool);
  static ArrayData* SetRefStrDict(ArrayData*, StringData*, tv_lval, bool);
  static ArrayData* AppendRefDict(ArrayData*, tv_lval, bool);
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
  static constexpr auto ToShapeDict = &ArrayCommon::ToShape;
  static ArrayData* ToDArrayDict(ArrayData*, bool);

  //////////////////////////////////////////////////////////////////////

  // Like Lval[Int,Str], but silently does nothing if the element does not
  // exist. Not part of the ArrayData interface, but used for member operations.
  static arr_lval LvalSilentInt(ArrayData*, int64_t, bool);
  static arr_lval LvalSilentStr(ArrayData*, const StringData*, bool);

  static constexpr auto LvalSilentIntDict = &LvalSilentInt;
  static constexpr auto LvalSilentStrDict = &LvalSilentStr;

  //////////////////////////////////////////////////////////////////////

  static tv_rval NvTryGetIntShape(const ArrayData*, int64_t);
  static constexpr auto NvGetIntShape = &NvGetInt;
  static tv_rval NvTryGetStrShape(const ArrayData*,
                                  const StringData*);
  static constexpr auto NvGetStrShape = &NvGetStr;
  static tv_rval RvalIntShape(const ArrayData* ad, int64_t k) {
    assertx(ad->isShape());
    return NvGetIntShape(ad, k);
  }
  static tv_rval RvalIntStrictShape(const ArrayData* ad, int64_t k) {
    assertx(ad->isShape());
    return NvTryGetIntShape(ad, k);
  }
  static tv_rval RvalStrShape(const ArrayData* ad, const StringData* k) {
    assertx(ad->isShape());
    return NvGetStrShape(ad, k);
  }
  static tv_rval RvalStrStrictShape(const ArrayData* ad,
                                       const StringData* k) {
    assertx(ad->isShape());
    return NvTryGetStrShape(ad, k);
  }
  static constexpr auto ReleaseShape = &Release;
  static constexpr auto NvGetKeyShape = &NvGetKey;
  static constexpr auto SetIntShape = &SetInt;
  static constexpr auto SetStrShape = &SetStr;
  static constexpr auto AddIntShape = &AddInt;
  static constexpr auto AddStrShape = &AddStr;
  static constexpr auto VsizeShape = &Vsize;
  static constexpr auto GetValueRefShape = &GetValueRef;
  static constexpr auto IsVectorDataShape = &IsVectorData;
  static constexpr auto ExistsIntShape = &ExistsInt;
  static constexpr auto ExistsStrShape = &ExistsStr;
  static constexpr auto LvalIntShape = &LvalInt;
  static constexpr auto LvalStrShape = &LvalStr;
  static constexpr auto LvalNewShape = &LvalNew;
  static constexpr auto RemoveIntShape = &RemoveInt;
  static constexpr auto RemoveStrShape = &RemoveStr;
  static constexpr auto IterBeginShape = &IterBegin;
  static constexpr auto IterLastShape = &IterLast;
  static constexpr auto IterEndShape = &IterEnd;
  static constexpr auto IterAdvanceShape = &IterAdvance;
  static constexpr auto IterRewindShape = &IterRewind;
  static constexpr auto ValidMArrayIterShape = ValidMArrayIter;
  static constexpr auto AdvanceMArrayIterShape = &AdvanceMArrayIter;
  static constexpr auto EscalateForSortShape = &EscalateForSort;
  static constexpr auto KsortShape = &Ksort;
  static constexpr auto SortShape = &Sort;
  static constexpr auto AsortShape = &Asort;
  static constexpr auto UksortShape = &Uksort;
  static constexpr auto UsortShape = &Usort;
  static constexpr auto UasortShape = &Uasort;
  static constexpr auto CopyShape = &Copy;
  static constexpr auto CopyStaticShape = &CopyStatic;
  static constexpr auto AppendShape = &Append;
  static arr_lval LvalIntRefShape(ArrayData*, int64_t, bool);
  static arr_lval LvalStrRefShape(ArrayData*, StringData*, bool);
  static arr_lval LvalNewRefShape(ArrayData*, bool);
  static ArrayData* SetWithRefIntShape(ArrayData*, int64_t k,
                                      TypedValue v, bool copy);
  static ArrayData* SetWithRefStrShape(ArrayData*, StringData* k,
                                      TypedValue v, bool copy);
  static ArrayData* SetRefIntShape(ArrayData*, int64_t, tv_lval, bool);
  static ArrayData* SetRefStrShape(ArrayData*, StringData*, tv_lval, bool);
  static ArrayData* AppendRefShape(ArrayData*, tv_lval, bool);
  static ArrayData* AppendWithRefShape(ArrayData*, TypedValue, bool);
  static constexpr auto PlusEqShape = &PlusEq;
  static constexpr auto MergeShape = &Merge;
  static constexpr auto PopShape = &Pop;
  static constexpr auto DequeueShape = &Dequeue;
  static constexpr auto PrependShape = &Prepend;
  static constexpr auto RenumberShape = &Renumber;
  static constexpr auto OnSetEvalScalarShape = &OnSetEvalScalar;
  static constexpr auto EscalateShape = &Escalate;
  static ArrayData* ToPHPArrayShape(ArrayData*, bool);
  static ArrayData* ToShapeShape(ArrayData*, bool);
  static constexpr auto ToVecShape = &ArrayCommon::ToVec;
  static constexpr auto ToKeysetShape = &ArrayCommon::ToKeyset;
  static constexpr auto ToVArrayShape = &ArrayCommon::ToVArray;
  static constexpr auto ToDictShape = &ArrayCommon::ToDict;
  static ArrayData* ToDArrayShape(ArrayData*, bool);

  static constexpr auto LvalSilentIntShape = &LvalSilentInt;
  static constexpr auto LvalSilentStrShape = &LvalSilentStr;

  //////////////////////////////////////////////////////////////////////

private:
  MixedArray* copyMixed() const;
  static ArrayData* MakeReserveImpl(uint32_t capacity, HeaderKind hk,
                                    ArrayData::DVArray);
  static MixedArray* MakeStructImpl(uint32_t, const StringData* const*,
                                    const TypedValue*, HeaderKind,
                                    ArrayData::DVArray);

  static ArrayData* FromDictImpl(ArrayData*, bool, bool);

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
    return t == kInvalidDataType;
  }

private:
  friend struct array::HashTable<MixedArray, MixedArrayElm>;
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

  friend size_t getMemSize(const ArrayData*, bool);
  template <typename AccessorT, class ArrayT>
  friend SortFlavor genericPreSort(ArrayT&, const AccessorT&, bool);

public:
  // Safe downcast helpers
  static MixedArray* asMixed(ArrayData* ad) {
    assertx(ad->hasMixedLayout());
    auto a = static_cast<MixedArray*>(ad);
    assertx(a->checkInvariants());
    return a;
  }
  static const MixedArray* asMixed(const ArrayData* ad) {
    assertx(ad->hasMixedLayout());
    auto a = static_cast<const MixedArray*>(ad);
    assertx(a->checkInvariants());
    return a;
  }

  // Fast iteration
  template <class F, bool inc = true>
  static void IterateV(const MixedArray* arr, F fn) {
    assertx(arr->hasMixedLayout());
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
    assertx(arr->hasMixedLayout());
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

  static MixedArray* CopyMixed(const MixedArray& other, AllocMode,
                               HeaderKind, ArrayData::DVArray);
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

  bool nextInsert(Cell);
  ArrayData* nextInsertRef(tv_lval data);
  ArrayData* nextInsertWithRef(TypedValue data);
  ArrayData* nextInsertWithRef(const Variant& data);
  ArrayData* addVal(int64_t ki, Cell data);
  ArrayData* addVal(StringData* key, Cell data);
  ArrayData* addValNoAsserts(StringData* key, Cell data);

  Elm& addKeyAndGetElem(StringData* key);

  template <bool warn, class K> arr_lval addLvalImpl(K k);
  template <class K> ArrayData* update(K k, Cell data);
  template <class K> ArrayData* updateWithRef(K k, TypedValue data);
  template <class K> ArrayData* updateRef(K k, tv_lval data);

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

  MixedArray* initRef(TypedValue& tv, tv_lval v);
  MixedArray* initWithRef(TypedValue& tv, TypedValue v);
  MixedArray* initWithRef(TypedValue& tv, const Variant& v);
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

  struct ShapeInitializer;
  static ShapeInitializer s_shape_initializer;

  int64_t  m_nextKI;        // Next integer key to use for append.
};

ALWAYS_INLINE Array empty_dict_array() {
  return Array::attach(staticEmptyDictArray());
}

HASH_TABLE_CHECK_OFFSETS(MixedArray, MixedArrayElm)
//////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_HPHP_ARRAY_H_
