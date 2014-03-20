/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArrayInit;
struct MemoryProfile;

class HphpArray : public ArrayData {
  // Load factor scaler. If S is the # of elements, C is the
  // power-of-2 capacity, and L=LoadScale, we grow when S > C-C/L.
  // So 2 gives 0.5 load factor, 4 gives 0.75 load factor, 8 gives
  // 0.875 load factor. Use powers of 2 to enable shift-divide.
  static const uint32_t LoadScale = 4;

public:
  /*
   * Iterator helper for kPackedKind and kMixedKind.  You can use this
   * to look at the values in the array, but not the keys unless you
   * know it is kMixedKind.
   *
   * This can be used as an optimization vs. ArrayIter, which uses
   * indirect calls in the loop.
   *
   * Note that this iterator class doesn't skip tombstones.  You have
   * to do that yourself if you want it.
   */
  struct ValIter;

  struct Elm {
    /* The key is either a string pointer or an int value, and the _count
     * field in data is used to discriminate the key type. _count = 0 means
     * int, nonzero values contain 32 bits of a string's hashcode.
     * It is critical that when we return &data to clients, that they not
     * read or write the _count field! */
    union {
      int64_t ikey;
      StringData* key;
    };
    // We store values here, but also some information local to this array:
    // data.m_aux.u_hash contains either 0 (for an int key) or a string
    // hashcode; the high bit is the int/string key descriminator.
    // data.m_type == KindOfInvalid if this is an empty slot in the
    // array (e.g. after a key is deleted).
    TypedValueAux data;

    bool hasStrKey() const {
      return data.hash() != 0;
    }

    bool hasIntKey() const {
      return data.hash() == 0;
    }

    int32_t hash() const {
      return data.hash();
    }

    void setStaticKey(StringData* k, strhash_t h) {
      assert(k->isStatic());
      key = k;
      data.hash() = h | STRHASH_MSB;
    }

    void setStrKey(StringData* k, strhash_t h) {
      key = k;
      data.hash() = h | STRHASH_MSB;
      k->incRefCount();
    }

    void setIntKey(int64_t k) {
      ikey = k;
      data.hash() = 0;
    }

    static constexpr size_t dataOff() {
      return offsetof(Elm, data);
    }
  };

  static constexpr size_t dataOff() {
    return sizeof(HphpArray);
  }

  /*
   * Allocate a new, empty, request-local HphpArray in packed mode,
   * with enough space reserved for `capacity' members.
   *
   * The returned array is already incref'd.
   */
  static HphpArray* MakeReserve(uint32_t capacity);

  /*
   * Allocate a packed HphpArray.  This is an array in packed
   * mode, containing `size' values, in the reverse order of the
   * `values' array.
   *
   * This function takes ownership of the TypedValues in `values'.
   *
   * The returned array is already incref'd.
   *
   * Pre: size > 0
   */
  static HphpArray* MakePacked(uint32_t size, const TypedValue* values);

  /*
   * Like MakePacked, but given static strings, make a struct-like array.
   * Also requires size > 0.
   */
  static HphpArray* MakeStruct(uint32_t size, StringData** keys,
                               const TypedValue* values);

  /*
   * Allocate an uncounted HphpArray and copy the values from the input 'array'
   * into the uncounted one. All values copied are made uncounted as well.
   * An uncounted array can only contain uncounted values (primitive values,
   * uncounted or static strings and uncounted or static arrays).
   */
  static HphpArray* MakeUncounted(ArrayData* array);

  /*
   * Return a pointer to the singleton static empty array.  This is
   * used for initial empty arrays (COW will cause it to escalate to a
   * request-local array if it is modified).
   */
  static ArrayData* GetStaticEmptyArray();

  // This behaves the same as iter_begin except that it assumes
  // this array is not empty and its not virtual.
  ssize_t getIterBegin() const {
    assert(!empty());
    if (LIKELY(!isTombstone(data()[0].data.m_type))) {
      return 0;
    }
    return nextElm(data(), 0);
  }

  // These using directives ensure the full set of overloaded functions
  // are visible in this class, to avoid triggering implicit conversions
  // from a const Variant& key to int64.
  using ArrayData::exists;
  using ArrayData::lval;
  using ArrayData::lvalNew;
  using ArrayData::set;
  using ArrayData::setRef;
  using ArrayData::add;
  using ArrayData::remove;
  using ArrayData::nvGet;
  using ArrayData::release;

  // implements ArrayData
  static const Variant& GetValueRef(const ArrayData*, ssize_t pos);

  // overrides ArrayData
  static bool IsVectorData(const ArrayData*);
  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t pos);
  static ssize_t IterRewind(const ArrayData*, ssize_t pos);

  // implements ArrayData
  static bool ExistsInt(const ArrayData*, int64_t k);
  static bool ExistsStr(const ArrayData*, const StringData* k);
  static bool ExistsIntPacked(const ArrayData*, int64_t k);

  // implements ArrayData
  static ArrayData* LvalInt(ArrayData* ad, int64_t k, Variant*& ret,
                            bool copy);
  static ArrayData* LvalStr(ArrayData* ad, StringData* k, Variant*& ret,
                            bool copy);
  static ArrayData* LvalIntPacked(ArrayData* ad, int64_t k, Variant*& ret,
                                  bool copy);
  static ArrayData* LvalStrPacked(ArrayData* ad, StringData* k, Variant*& ret,
                                  bool copy);
  static ArrayData* LvalNew(ArrayData*, Variant*& ret, bool copy);
  static ArrayData* LvalNewPacked(ArrayData*, Variant*& ret, bool copy);

  // implements ArrayData
  static ArrayData* SetIntPacked(ArrayData*, int64_t k, const Variant& v, bool copy);
  static ArrayData* SetStrPacked(ArrayData*, StringData* k, const Variant& v,
                                 bool copy);
  static ArrayData* SetInt(ArrayData*, int64_t k, const Variant& v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, const Variant& v, bool copy);

  static ArrayData* ZSetInt(ArrayData*, int64_t k, RefData* v);
  static ArrayData* ZSetStr(ArrayData*, StringData* k, RefData* v);
  static ArrayData* ZAppend(ArrayData* ad, RefData* v);

  // implements ArrayData
  static ArrayData* SetRefInt(ArrayData* ad, int64_t k, const Variant& v,
                              bool copy);
  static ArrayData* SetRefStr(ArrayData* ad, StringData* k, const Variant& v,
                              bool copy);
  static ArrayData* SetRefIntPacked(ArrayData* ad, int64_t k, const Variant& v,
                                    bool copy);
  static ArrayData* SetRefStrPacked(ArrayData* ad, StringData* k, const Variant& v,
                                    bool copy);

  // overrides ArrayData
  static ArrayData* AddInt(ArrayData*, int64_t k, const Variant& v, bool copy);
  static ArrayData* AddStr(ArrayData*, StringData* k, const Variant& v, bool copy);
  static ArrayData* AddIntPacked(ArrayData*, int64_t k, const Variant& v, bool copy);

  // implements ArrayData
  static ArrayData* RemoveInt(ArrayData*, int64_t k, bool copy);
  static ArrayData* RemoveStr(ArrayData*, const StringData* k, bool copy);
  static ArrayData* RemoveIntPacked(ArrayData*, int64_t k, bool copy);
  static ArrayData* RemoveStrPacked(ArrayData*, const StringData* k, bool copy);

  // overrides ArrayData
  static ArrayData* Copy(const ArrayData*);
  static ArrayData* CopyPacked(const ArrayData*);
  static ArrayData* CopyWithStrongIterators(const ArrayData*);
  static ArrayData* NonSmartCopy(const ArrayData*);

  static ArrayData* AppendPacked(ArrayData*, const Variant& v, bool copy);
  static ArrayData* Append(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendRef(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendRefPacked(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendWithRefPacked(ArrayData*, const Variant& v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Pop(ArrayData*, Variant& value);
  static ArrayData* PopPacked(ArrayData*, Variant& value);
  static ArrayData* Dequeue(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, const Variant& v, bool copy);
  static ArrayData* DequeuePacked(ArrayData*, Variant& value);
  static ArrayData* PrependPacked(ArrayData*, const Variant& v, bool copy);
  static void Renumber(ArrayData*);
  static void RenumberPacked(ArrayData*);
  static void OnSetEvalScalar(ArrayData*);
  static void OnSetEvalScalarPacked(ArrayData*);
  static void ReleasePacked(ArrayData*);
  static void Release(ArrayData*);
  static void ReleaseUncounted(ArrayData*);

  // overrides ArrayData
  static bool ValidMArrayIter(const ArrayData*, const MArrayIter &fp);
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter& fp);

  HphpArray* copyImpl() const;
  HphpArray* copyPacked() const;
  HphpArray* copyMixed() const;
  HphpArray* copyPackedAndResizeIfNeeded() const;
  HphpArray* copyMixedAndResizeIfNeeded() const;
  HphpArray* copyPackedAndResizeIfNeededSlow() const;
  HphpArray* copyMixedAndResizeIfNeededSlow() const;

  // nvGet and friends.
  // "nv" stands for non-variant. If we know the types of keys and values
  // through runtime and compile-time chicanery, we can directly call these
  // methods.

  // nvGet returns a pointer to the value if the specified key is in the
  // array, NULL otherwise.
  static TypedValue* NvGetIntPacked(const ArrayData*, int64_t ki);
  static TypedValue* NvGetStrPacked(const ArrayData*, const StringData* k);
  static TypedValue* NvGetInt(const ArrayData*, int64_t ki);
  static TypedValue* NvGetStr(const ArrayData*, const StringData* k);
  static void NvGetKeyPacked(const ArrayData*, TypedValue* out, ssize_t pos);
  static void NvGetKey(const ArrayData*, TypedValue* out, ssize_t pos);

  /**
   * Main helper for AddNewElemC.  The semantics are slightly different from
   * other helpers, but tuned for the opcode.  The value to set is passed by
   * value; the caller has incref'd it if necessary, and this call *moves* it
   * to its location in the array (caller must not decref).  If the value cannot
   * be stored in the array, this helper decref's it.
   */
  static ArrayData* AddNewElemC(ArrayData* a, TypedValue value);

  /*
   * Inline helpers to be called directly from the TC
   */
  static TypedValue GetCellIntPacked(const ArrayData* ad, int64_t ki);
  static uint64_t IssetIntPacked(const ArrayData* ad, int64_t ki);

  /*
   * Sorting routines.
   */
  static ArrayData* EscalateForSort(ArrayData* ad);
  static void Ksort(ArrayData*, int sort_flags, bool ascending);
  static void Sort(ArrayData*, int sort_flags, bool ascending);
  static void Asort(ArrayData*, int sort_flags, bool ascending);
  static bool Uksort(ArrayData*, const Variant& cmp_function);
  static bool Usort(ArrayData*, const Variant& cmp_function);
  static bool Uasort(ArrayData*, const Variant& cmp_function);

  // Elm's data.m_type == KindOfInvalid for deleted slots.
  static bool isTombstone(DataType t) {
    assert(IS_REAL_TYPE(t) || t == KindOfInvalid);
    return t < KindOfUninit;
    static_assert(KindOfUninit == 0 && KindOfInvalid < 0, "");
  }

  // Element index, with special values < 0 used for hash tables.
  // NOTE: Unfortunately, g++ on x64 tends to generate worse machine code for
  // 32-bit ints than it does for 64-bit ints. As such, we have deliberately
  // chosen to use ssize_t in some places where ideally we *should* have used
  // int32_t.
  static const int32_t Empty      = -1; // == ArrayData::invalid_index
  static const int32_t Tombstone  = -2;

  // Use a minimum of an 4-element hash table.  Valid range: [2..32]
  static const uint32_t MinLgTableSize = 2;
  static const uint32_t SmallHashSize = 1 << MinLgTableSize;
  static const uint32_t SmallMask = SmallHashSize - 1;
  static const uint32_t SmallSize = SmallHashSize - SmallHashSize / LoadScale;
  static const uint32_t MaxMakeSize = 4 * SmallSize;

  uint32_t iterLimit() const { return m_used; }

  // Fetch a value and optional key (if keyPos != nullptr), given an
  // iterator pos.  If withref is true, copy the value with "withRef"
  // semantics, and decref the previous key before copying the key.
  // Otherwise get the value cell (unboxing), and initialize keyOut.
  template <bool withRef>
  void getArrayElm(ssize_t pos, TypedValue* out, TypedValue* keyOut) const;
  void getArrayElm(ssize_t pos, TypedValue* out) const;
  bool isTombstone(ssize_t pos) const;

  static bool validPos(ssize_t pos);
  static bool validPos(int32_t pos);
  size_t hashSize() const;
  static size_t computeMaxElms(uint32_t tableMask);
  static size_t computeDataSize(uint32_t tableMask);

private:
  friend struct ArrayInit;
  friend struct MemoryProfile;
  friend struct EmptyArray;
  enum class ClonePacked {};
  enum class CloneMixed {};
  enum SortFlavor { IntegerSort, StringSort, GenericSort };

private:
  // Safe downcast helpers
  static HphpArray* asPacked(ArrayData* ad);
  static const HphpArray* asPacked(const ArrayData* ad);
  static HphpArray* asMixed(ArrayData* ad);
  static const HphpArray* asMixed(const ArrayData* ad);
  static HphpArray* asHphpArray(ArrayData* ad);
  static const HphpArray* asHphpArray(const ArrayData* ad);

  static void getElmKey(const Elm& e, TypedValue* out);

private:
  enum class AllocMode : bool { Smart, NonSmart };

  template<class CopyElem>
  static HphpArray* CopyPacked(const HphpArray& other,
                               AllocMode,
                               CopyElem);
  template<class CopyKeyValue>
  static HphpArray* CopyMixed(const HphpArray& other,
                              AllocMode,
                              CopyKeyValue);
  static HphpArray* CopyReserve(const HphpArray* src, size_t expectedSize);

  HphpArray() = delete;
  HphpArray(const HphpArray&) = delete;
  HphpArray& operator=(const HphpArray&) = delete;
  ~HphpArray() = delete;

private:
  static void initHash(int32_t* table, size_t tableSize);
  static int32_t* copyHash(int32_t* to, const int32_t* from, size_t tableSize);
  static Elm* copyElms(Elm* to, const Elm* from, size_t count);

  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc, bool checkTypes);
  void postSort(bool resetKeys);
  static ArrayData* ArrayPlusEqGeneric(ArrayData*,
    HphpArray*, const ArrayData*, size_t);
  static ArrayData* ArrayMergeGeneric(HphpArray*, const ArrayData*);

  // convert in-place from kPackedKind to kMixedKind: fill in keys & hashtable
  HphpArray* packedToMixed();

  ssize_t nextElm(Elm* elms, ssize_t ei) const {
    assert(ei >= -1);
    while (size_t(++ei) < m_used) {
      if (!isTombstone(elms[ei].data.m_type)) {
        return ei;
      }
    }
    return invalid_index;
  }
  ssize_t prevElm(Elm* elms, ssize_t ei) const;

  // Assert a bunch of invariants about this array then return true.
  // usage:  assert(checkInvariants());
  bool checkInvariants() const;

  template <class Hit>
  ssize_t findImpl(size_t h0, Hit) const;
  ssize_t find(int64_t ki) const;
  ssize_t find(const StringData* s, strhash_t prehash) const;

  // The array should already be sized for the new insertion before
  // calling these methods.
  template <class Hit>
  int32_t* findForInsertImpl(size_t h0, Hit) const;
  int32_t* findForInsert(int64_t ki) const;
  int32_t* findForInsert(const StringData* k, strhash_t prehash) const;

  struct InsertPos {
    InsertPos(bool found, TypedValue& tv) : found(found), tv(tv) {}
    bool found;
    TypedValue& tv;
  };
  InsertPos insert(int64_t k);
  InsertPos insert(StringData* k);

  template <class Hit, class Remove>
  ssize_t findForRemoveImpl(size_t h0, Hit, Remove) const;
  ssize_t findForRemove(int64_t ki, bool updateNext);
  ssize_t findForRemove(const StringData* k, strhash_t prehash);

  ssize_t iter_advance_helper(ssize_t prev) const;

  /**
   * findForNewInsert() CANNOT be used unless the caller can guarantee that
   * the relevant key is not already present in the array. Otherwise this can
   * put the array into a bad state; use with caution.
   */
  int32_t* findForNewInsert(size_t h0) const;
  int32_t* findForNewInsert(int32_t* table, size_t mask, size_t h0) const;

  bool nextInsert(const Variant& data);
  ArrayData* nextInsertRef(const Variant& data);
  ArrayData* nextInsertWithRef(const Variant& data);
  ArrayData* addVal(int64_t ki, const Variant& data);
  ArrayData* addVal(StringData* key, const Variant& data);

  template <class K> ArrayData* addLvalImpl(K k, Variant*& ret);
  template <class K> ArrayData* update(K k, const Variant& data);
  template <class K> ArrayData* updateRef(K k, const Variant& data);

  template <class K> ArrayData* zSetImpl(K k, RefData* data);
  ArrayData* zAppendImpl(RefData* data);

  void adjustMArrayIter(ssize_t pos);
  void erase(ssize_t pos);

  HphpArray* copyImpl(HphpArray* target) const;

  bool isFull() const;
  bool isFullPacked() const;

  // Pre: !isFullPacked()
  TypedValue& allocNextElm(uint32_t i);

  Elm& allocElm(int32_t* ei);

  HphpArray* setVal(TypedValue& tv, const Variant& v);
  HphpArray* setRef(TypedValue& tv, const Variant& v);
  HphpArray* getLval(TypedValue& tv, Variant*& ret);
  HphpArray* initVal(TypedValue& tv, const Variant& v);
  HphpArray* initRef(TypedValue& tv, const Variant& v);
  HphpArray* initLval(TypedValue& tv, Variant*& ret);
  HphpArray* initWithRef(TypedValue& tv, const Variant& v);
  HphpArray* moveVal(TypedValue& tv, TypedValue v);

  ArrayData* zInitVal(TypedValue& tv, RefData* v);
  ArrayData* zSetVal(TypedValue& tv, RefData* v);

  /*
   * grow() increases the hash table size and the number of slots for
   * elements by a factor of 2. grow() rebuilds the hash table, but it
   * does not compact the elements.
   */
  static HphpArray* Grow(HphpArray* old);
  static HphpArray* GrowPacked(HphpArray* old);

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
   * Both functions return the new HphpArray* to use (or the old one
   * if they didn't need to grow).  The old HphpArray is left in a
   * zombie state where the only legal action is to decref and then
   * throw it away.
   */
  HphpArray* resize();
  HphpArray* resizeIfNeeded();
  HphpArray* resizePackedIfNeeded();

  Elm* data() const {
    return const_cast<Elm*>(reinterpret_cast<Elm const*>(this + 1));
  }

  int32_t* hashTab() const {
    return const_cast<int32_t*>(
      reinterpret_cast<int32_t const*>(
        data() + m_cap
      )
    );
  }

  bool isZombie() const { return m_used + 1 == 0; }

private:
  // Some of these are packed into qword-sized unions so we can
  // combine stores during initialization.  (gcc won't do it on its
  // own.)
  union {
    struct {
      uint32_t m_cap;       // Number of Elms we can use before having to grow.
      uint32_t m_used;      // Number of used elements (values or tombstones)
    };
    uint64_t m_capAndUsed;
  };
  union {
    struct {
      uint32_t m_tableMask; // Bitmask used when indexing into the hash table.
      uint32_t m_hLoad;     // Hash table load (# of non-empty slots).
    };
    uint64_t m_maskAndLoad;
  };
  int64_t  m_nextKI;        // Next integer key to use for append.
};

extern std::aligned_storage<
  sizeof(ArrayData),
  alignof(ArrayData)
>::type s_theEmptyArray;

//=============================================================================

inline ArrayData* HphpArray::GetStaticEmptyArray() {
  void* vp = &s_theEmptyArray;
  return static_cast<ArrayData*>(vp);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HPHP_ARRAY_H_
