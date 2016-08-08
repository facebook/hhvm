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
#ifndef incl_HPHP_STRUCT_ARRAY_H_
#define incl_HPHP_STRUCT_ARRAY_H_

#include <cstddef>
#include <cstdint>
#include <sys/types.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/array-common.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Variant;
struct RefData;
struct ArrayData;
struct StringData;

//////////////////////////////////////////////////////////////////////

/*
 * Single element Array structure. 
 * Do not change member function names below to maintain compatibiality 
 * Mixed Array class usage. 
 * The hash value for the single element stored must be generated at the time of 
 * converting single array to mixed array.  
 * The layout is a sub-set of Mixed Array to keep the code path of conversion from 
 * SingleArray to SingleArray short.
 * Do not remove any of the public methods, assert them in the definition instead.
 * Keep the comments until the implementation is complete. 
 */
struct SingleArray final : public ArrayData,
                           type_scan::MarkCountable<StructArray> {
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
      return SingleArray::isTombstone(data.m_type);
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
    return sizeof(SingleArray);
  }
  static constexpr ptrdiff_t usedOff() {
    return offsetof(SingleArray, m_used);
  }

  struct ElmKey {
    ElmKey() {}
    ElmKey(StringData* key) : skey(key) {}
    ElmKey(int64_t key) : ikey(key) {}
    union {
      StringData* skey;
      int64_t ikey;
    };

    // There's no need to store a hash value for the key.
    // The data aux field has a hash value asscoiated with it: data.hash()
    // The array contains only a single integer/ String key. 
    //hash_t hash;
  };

/*
   * Initialize an empty small mixed array with given field. This should be
   * inlined.
   */
  static void InitSmall(SingleArray* a, RefCount count, uint32_t size,
                        int64_t nextIntKey);

  /*
   * Allocate a new, empty, request-local array with the same mode as
   * `other' and with enough space reserved for `capacity' members, or
   * if `capacity' is zero, with the same capacity as `other'.
   *
   * The returned array is already incref'd.
   */
  static ArrayData* MakeReserveLike(const ArrayData* other, uint32_t capacity);

  /*
   * Like MakePacked, but given static strings, make a struct-like array.
   * Also requires size > 0.
   */
  static SingleArray* MakeStruct(uint32_t size, const StringData* const* keys,
                               const TypedValue* values);
  static StructArray* MakeStructArray(uint32_t size, const TypedValue* values,
                                      Shape*);

  /*
   * Allocate an uncounted SingleArray and copy the values from the
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

  static ArrayData* MakeFromDict(ArrayData* adIn, bool copy);

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

  using ArrayData::decRefCount;
  using ArrayData::hasMultipleRefs;
  using ArrayData::hasExactlyOneRef;
  using ArrayData::decWillRelease;
  using ArrayData::incRefCount;

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

private:
  SingleArray* copySingle() const;
  SingleArray* copySingleAndResizeIfNeeded() const;
  SingleArray* copySingleAndResizeIfNeededSlow() const;
  static ArrayData* MakeReserveImpl(uint32_t capacity, HeaderKind hk);

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

  uint32_t iterLimit() const { return m_used; }

  // Fetch a value and optional key (if keyPos != nullptr), given an
  // iterator pos.  If withref is true, copy the value with "withRef"
  // semantics, and decref the previous key before copying the key.
  // Otherwise get the value cell (unboxing), and initialize keyOut.
  void getArrayElm(ssize_t pos, TypedValue* out, TypedValue* keyOut) const;
  void getArrayElm(ssize_t pos, TypedValue* out) const;
  void dupArrayElmWithRef(ssize_t pos, TypedValue* valOut,
    TypedValue* keyOut) const;

  bool isTombstone(ssize_t pos) const;

  // public functions, do not remove them
  // hashSize() and heapSize() will return zero. 
  size_t hashSize() const;
  size_t heapSize() const;
  static constexpr size_t computeMaxElms(uint32_t tableMask);
  static size_t computeAllocBytesFromMaxElms(uint32_t maxElms);

private:
  friend struct ArrayInit;
  friend struct MemoryProfile;
  friend struct EmptyArray;
  friend struct PackedArray;
  friend struct StructArray;
  friend struct HashCollection;
  friend struct BaseMap;
  friend struct c_Map;
  friend struct c_ImmMap;
  friend struct BaseSet;
  friend struct c_Set;
  friend struct c_ImmSet;
  friend struct c_AwaitAllWaitHandle;

  friend size_t getMemSize(const ArrayData*);

public:
  // Safe downcast helpers
  static SingleArray* asSingle(ArrayData* ad);
  static const SingleArray* asSingle(const ArrayData* ad);
  // Fast iteration
  template <class F> static void IterateV(SingleArray* arr, F fn) {
    auto elm = arr->data();
    arr->incRefCount();
    SCOPE_EXIT { decRefArr(arr); };
    for (auto i = arr->m_used; i--; elm++) {
      if (LIKELY(!elm->isTombstone())) {
        if (ArrayData::call_helper(fn, &elm->data)) break;
      }
    }
  }

  template <class F> static void IterateKV(SingleArray* arr, F fn) {
    auto elm = arr->data();
    arr->incRefCount();
    SCOPE_EXIT { decRefArr(arr); };
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

  static SingleArray* CopySingle(const SingleArray& other, AllocMode, HeaderKind);
  // Reserve copy functionality may not be needed.  
  static SingleArray* CopyReserve(const SingleArray* src, size_t expectedSize);

  SingleArray() = delete;
  SingleArray(const SingleArray&) = delete;
  SingleArray& operator=(const SingleArray&) = delete;
  ~SingleArray() = delete;

private:
  static void initHash(int32_t* table, uint32_t scale);
  static void copyHash(int32_t* to, const int32_t* from, uint32_t scale);
  // Copy elements as well as `m_nextKI' from one SingleArray to another.
  // Warning: it could copy up to 24 bytes beyond the array and thus overwrite
  // the hashtable, but it never reads/writes beyond the end of the hash
  // table.  If you use this function, make sure you copy/write the correct
  // data on the hash table afterwards.

  //nextKI and hashtable are not allocated for SingleArray's
  static void copyElmsNextUnsafe(SingleArray* to, const SingleArray* from,
                                 uint32_t nElems);

  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc, bool checkTypes);
  void postSort(bool resetKeys);
  static ArrayData* ArrayPlusEqGeneric(ArrayData*,
    SingleArray*, const ArrayData*, size_t);
  static ArrayData* ArrayMergeGeneric(SingleArray*, const ArrayData*);

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

//------------------------------Remove all methods below-------------------------------------------------------

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

  /*
   * Helper routine for inserting elements into a new array
   * when Grow()ing the array, that also checks for potentially
   * unbalanced entries because of hash collision.
   */
  static SingleArray* InsertCheckUnbalanced(SingleArray* ad, int32_t* table,
                                           uint32_t mask,
                                           Elm* iter, Elm* stop);
  /*
   * grow() increases the hash table size and the number of slots for
   * elements by a factor of 2. grow() rebuilds the hash table, but it
   * does not compact the elements.
   */
  static SingleArray* Grow(SingleArray* old, uint32_t newScale);

  /**
   * compact() does not change the hash table size or the number of slots
   * for elements. compact() rebuilds the hash table and compacts the
   * elements into the slots with lower addresses.
   */
  void compact(bool renumber);

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

  SingleArray* copyImpl(SingleArray* target) const;

  bool isFull() const;
  bool isFullPacked() const;

  // Pre: !isFullPacked()
  TypedValue& allocNextElm(uint32_t i);

  Elm& allocElm(int32_t* ei);

  SingleArray* getLval(TypedValue& tv, Variant*& ret);
  SingleArray* initRef(TypedValue& tv, Variant& v);
  SingleArray* initLval(TypedValue& tv, Variant*& ret);
  SingleArray* initWithRef(TypedValue& tv, const Variant& v);
  SingleArray* moveVal(TypedValue& tv, TypedValue v);

  ArrayData* zInitVal(TypedValue& tv, RefData* v);
  ArrayData* zSetVal(TypedValue& tv, RefData* v);

  /*
   * resize() and resizeIfNeeded() will convert the SingleArray into a 
   * MixedArray. 
   */
  SingleArray* resize();
  // May not be necessary
  SingleArray* resizeIfNeeded(); 

  Elm* data() const {
    return const_cast<Elm*>(reinterpret_cast<Elm const*>(this + 1));
  }

  bool isZombie() const { return m_used + 1 == 0; }
  void setZombie() { m_used = -uint32_t{1}; }

public:
  template<class F> void scan(F&) const; // in mixed-array-defs.h
private:
  uint64_t m_used;   // Number of used elements (values or tombstones)
};  // struct SingleArray


inline constexpr size_t SingleArray::computeMaxElms(uint32_t ) {
  return 1;
}

ALWAYS_INLINE constexpr size_t computeAllocBytes(uint32_t) {
  return sizeof(SingleArray) + sizeof(SingleArray::Elm);
}

//////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_HPHP_ARRAY_H_