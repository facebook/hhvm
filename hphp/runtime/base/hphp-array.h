/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/complex-types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArrayInit;
struct MemoryProfile;

class HphpArray : public ArrayData {
  // Load factor scaler.  If S is the # of elements, C is the
  // power-of-2 capacity, and L=LoadScale, we grow when S > C-C/L.
  // So 2 gives 0.5 load factor, 4 gives 0.75 load factor, 8 gives
  // 0.125 load factor.  Use powers of 2 to enable shift-divide.
  static const uint LoadScale = 4;

public:
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
    void setStrKey(StringData* k, strhash_t h) {
      key = k;
      data.hash() = h | STRHASH_MSB;
      k->incRefCount();
    }
    void setIntKey(int64_t k) {
      ikey = k;
      data.hash() = 0;
    }
  };

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
   * Return a pointer to the singleton static empty array.  This is
   * used for initial empty arrays (COW will cause it to escalate to a
   * request-local array if it is modified).
   */
  static HphpArray* GetStaticEmptyArray();

  // This behaves the same as iter_begin except that it assumes
  // this array is not empty and its not virtual.
  ssize_t getIterBegin() const {
    assert(!empty());
    if (LIKELY(!isTombstone(m_data[0].data.m_type))) {
      return 0;
    }
    return nextElm(m_data, 0);
  }

  // These using directives ensure the full set of overloaded functions
  // are visible in this class, to avoid triggering implicit conversions
  // from a CVarRef key to int64.
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
  static CVarRef GetValueRef(const ArrayData*, ssize_t pos);

  // overrides ArrayData
  static bool IsVectorData(const ArrayData*);
  static bool IsVectorDataPacked(const ArrayData*);
  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t pos);
  static ssize_t IterRewind(const ArrayData*, ssize_t pos);

  // implements ArrayData
  static bool ExistsInt(const ArrayData*, int64_t k);
  static bool ExistsStr(const ArrayData*, const StringData* k);
  static bool ExistsIntPacked(const ArrayData*, int64_t k);
  static bool ExistsStrPacked(const ArrayData*, const StringData* k);

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
  static ArrayData* SetIntPacked(ArrayData*, int64_t k, CVarRef v, bool copy);
  static ArrayData* SetStrPacked(ArrayData*, StringData* k, CVarRef v,
                                 bool copy);
  static ArrayData* SetInt(ArrayData*, int64_t k, CVarRef v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, CVarRef v, bool copy);

  static void ZSetInt(ArrayData*, int64_t k, RefData* v);
  static void ZSetStr(ArrayData*, StringData* k, RefData* v);
  static void ZAppend(ArrayData* ad, RefData* v);

  // implements ArrayData
  static ArrayData* SetRefInt(ArrayData* ad, int64_t k, CVarRef v,
                              bool copy);
  static ArrayData* SetRefStr(ArrayData* ad, StringData* k, CVarRef v,
                              bool copy);
  static ArrayData* SetRefIntPacked(ArrayData* ad, int64_t k, CVarRef v,
                                    bool copy);
  static ArrayData* SetRefStrPacked(ArrayData* ad, StringData* k, CVarRef v,
                                    bool copy);

  // overrides ArrayData
  static ArrayData* AddInt(ArrayData*, int64_t k, CVarRef v, bool copy);
  static ArrayData* AddStr(ArrayData*, StringData* k, CVarRef v, bool copy);
  static ArrayData* AddIntPacked(ArrayData*, int64_t k, CVarRef v, bool copy);

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

  static ArrayData* AppendPacked(ArrayData*, CVarRef v, bool copy);
  static ArrayData* Append(ArrayData*, CVarRef v, bool copy);
  static ArrayData* AppendRef(ArrayData*, CVarRef v, bool copy);
  static ArrayData* AppendRefPacked(ArrayData*, CVarRef v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, CVarRef v, bool copy);
  static ArrayData* AppendWithRefPacked(ArrayData*, CVarRef v, bool copy);
  static ArrayData* Plus(ArrayData*, const ArrayData* elems, bool copy);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems, bool copy);
  static ArrayData* Pop(ArrayData*, Variant& value);
  static ArrayData* PopPacked(ArrayData*, Variant& value);
  static ArrayData* Dequeue(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, CVarRef v, bool copy);
  static ArrayData* DequeuePacked(ArrayData*, Variant& value);
  static ArrayData* PrependPacked(ArrayData*, CVarRef v, bool copy);
  static void Renumber(ArrayData*);
  static void RenumberPacked(ArrayData*);
  static void OnSetEvalScalar(ArrayData*);
  static void OnSetEvalScalarPacked(ArrayData*);
  static void ReleasePacked(ArrayData*);
  static void Release(ArrayData*);

  // overrides ArrayData
  static bool ValidFullPos(const ArrayData*, const FullPos &fp);
  static bool AdvanceFullPos(ArrayData*, FullPos& fp);

  HphpArray* copyImpl() const;
  HphpArray* copyPacked() const;
  HphpArray* copyMixed() const;

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

  void nvAppend(const TypedValue* v) {
    nextInsertPacked(tvAsCVarRef(v));
  }
  bool nvInsert(StringData* k, TypedValue *v);

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
  static void Uksort(ArrayData*, CVarRef cmp_function);
  static void Usort(ArrayData*, CVarRef cmp_function);
  static void Uasort(ArrayData*, CVarRef cmp_function);

  // Elm's data.m_type == KindOfInvalid for deleted slots.
  static bool isTombstone(DataType t) {
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

  uint32_t iterLimit() const { return m_used; }

  // Fetch a value and optional key (if keyPos != nullptr), given an
  // iterator pos.  If withref is true, copy the value with "withRef"
  // semantics, and decref the previous key before copying the key.
  // Otherwise get the value cell (unboxing), and initialize keyOut.
  template <bool withRef>
  void getArrayElm(ssize_t pos, TypedValue* out, TypedValue* keyOut) const;
  bool isTombstone(ssize_t pos) const;

  static bool validPos(ssize_t pos);
  static bool validPos(int32_t pos);
  static size_t computeTableSize(uint32_t tableMask);
  static size_t computeMaxElms(uint32_t tableMask);
  static size_t computeDataSize(uint32_t tableMask);

private:
  friend class ArrayInit;
  friend struct MemoryProfile;
  struct EmptyArrayInitializer;
  enum class ClonePacked {};
  enum class CloneMixed {};
  enum EmptyMode { StaticEmptyArray };
  enum SortFlavor { IntegerSort, StringSort, GenericSort };

  struct PromotedPayload {
    uint32_t oldCap;
    uint32_t oldMask;
  };

private:
  static EmptyArrayInitializer s_arrayInitializer;

private:
  // Safe downcast helpers
  static HphpArray* asPacked(ArrayData* ad);
  static const HphpArray* asPacked(const ArrayData* ad);
  static HphpArray* asMixed(ArrayData* ad);
  static const HphpArray* asMixed(const ArrayData* ad);
  static HphpArray* asHphpArray(ArrayData* ad);
  static const HphpArray* asHphpArray(const ArrayData* ad);

  static HphpArray* Make(EmptyMode);
  static void getElmKey(const Elm& e, TypedValue* out);

private:
  static HphpArray* CopyPacked(const HphpArray& other, AllocationMode);
  static HphpArray* CopyMixed(const HphpArray& other, AllocationMode);

  HphpArray() = delete;
  HphpArray(const HphpArray&) = delete;
  HphpArray& operator=(const HphpArray&) = delete;
  ~HphpArray() = delete;

private:
  void initHash(size_t tableSize);
  void initNonEmpty(const HphpArray& other);

  PromotedPayload& promotedPayload() {
    return *reinterpret_cast<PromotedPayload*>(this + 1);
  }

  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc, bool checkTypes);
  void postSort(bool resetKeys);

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

  template <class Hit>
  int32_t* findForInsertImpl(size_t h0, Hit) const;
  int32_t* findForInsert(int64_t ki) const;
  int32_t* findForInsert(const StringData* k, strhash_t prehash) const;

  template <class Hit, class Remove>
  ssize_t findForRemoveImpl(size_t h0, Hit, Remove) const;
  ssize_t findForRemove(int64_t ki, bool updateNext);
  ssize_t findForRemove(const StringData* k, strhash_t prehash);

  ssize_t iter_advance_helper(ssize_t prev) const ATTRIBUTE_COLD;

  /**
   * findForNewInsert() CANNOT be used unless the caller can guarantee that
   * the relevant key is not already present in the array. Otherwise this can
   * put the array into a bad state; use with caution.
   */
  int32_t* findForNewInsert(size_t h0) const;
  static int32_t* findForNewInsertLoop(int32_t* table, size_t h0,
                                       size_t mask);

  bool nextInsert(CVarRef data);
  HphpArray* nextInsertPacked(CVarRef data);
  ArrayData* nextInsertRef(CVarRef data);
  ArrayData* nextInsertWithRef(CVarRef data);
  ArrayData* addLvalImpl(int64_t ki, Variant*& ret);
  ArrayData* addLvalImpl(StringData* key, strhash_t h, Variant*& ret);
  ArrayData* addVal(int64_t ki, CVarRef data);
  ArrayData* addVal(StringData* key, CVarRef data);

  ArrayData* update(int64_t ki, CVarRef data);
  ArrayData* update(StringData* key, CVarRef data);
  ArrayData* updateRef(int64_t ki, CVarRef data);
  ArrayData* updateRef(StringData* key, CVarRef data);

  void zSetImpl(int64_t ki, RefData* data);
  void zSetImpl(StringData* key, RefData* data);
  void zAppendImpl(RefData* data);

  void adjustFullPos(ssize_t pos);
  void erase(ssize_t pos);

  HphpArray* copyImpl(HphpArray* target) const;

  bool isFull() const;
  Elm& newElm(int32_t* ei, size_t h0);
  Elm& newElmGrow(size_t h0);
  Elm& allocElm(int32_t* ei);
  TypedValue& allocNextElm(uint32_t i);

  HphpArray* setVal(TypedValue& tv, CVarRef v);
  HphpArray* setRef(TypedValue& tv, CVarRef v);
  HphpArray* getLval(TypedValue& tv, Variant*& ret);
  HphpArray* initVal(TypedValue& tv, CVarRef v);
  HphpArray* initRef(TypedValue& tv, CVarRef v);
  HphpArray* initLval(TypedValue& tv, Variant*& ret);
  HphpArray* initWithRef(TypedValue& tv, CVarRef v);
  HphpArray* moveVal(TypedValue& tv, TypedValue v);

  void zInitVal(TypedValue& tv, RefData* v);
  void zSetVal(TypedValue& tv, RefData* v);

  /*
   * grow() increases the hash table size and the number of slots for
   * elements by a factor of 2. grow() rebuilds the hash table, but it
   * does not compact the elements.
   */
  void grow() ATTRIBUTE_COLD;
  void growPacked() ATTRIBUTE_COLD;

  /**
   * compact() does not change the hash table size or the number of slots
   * for elements. compact() rebuilds the hash table and compacts the
   * elements into the slots with lower addresses.
   */
  void compact(bool renumber) ATTRIBUTE_COLD;

  /**
   * resize() and resizeIfNeeded() will grow or compact the array as
   * necessary to ensure that there is room for a new element and a
   * new hash entry.
   *
   * resize() assumes that the array does not have room for a new element
   * or a new hash entry. resizeIfNeeded() will first check if there is room
   * for a new element and hash entry before growing or compacting the array.
   */
  void resize();
  void resizeIfNeeded();

  bool isFlat() const {
    return m_data == static_cast<const void*>(this + 1);
  }

private:
  // Small: Array elements and the hash table are allocated inline.
  //
  //            +--------------------+
  // this -->   | HphpArray fields   |
  //            +--------------------+
  // m_data --> | slot 0 ...         | SmallSize slots for elements.
  //            | slot SmallSize-1   |
  //            +--------------------+
  // m_hash --> |                    | 2^MinLgTableSize hash table entries.
  //            +--------------------+
  //
  // Medium: Just the hash table is allocated inline, array elements
  // are allocated from malloc.
  //
  //            +--------------------+
  // this -->   | HphpArray fields   |
  //            +--------------------+
  // m_hash --> |                    | 2^K hash table entries
  //            +--------------------+
  //
  //            +--------------------+
  // m_data --> | slot 0             | 0.75 * 2^K slots for elements.
  //            | slot 1             |
  //            | ...                |
  //            +--------------------+
  //
  // Big: Array elements and the hash table are contiguously allocated, and
  // elements are pointer aligned.
  //
  //            +--------------------+
  // m_data --> | slot 0             | 0.75 * 2^K slots for elements.
  //            | slot 1             |
  //            | ...                |
  //            +--------------------+
  // m_hash --> |                    | 2^K hash table entries.
  //            +--------------------+

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
  Elm*     m_data;          // Contains elements and hash table.
  int32_t* m_hash;          // Hash table.
};

extern std::aligned_storage<
  sizeof(HphpArray) +
    sizeof(HphpArray::Elm) * HphpArray::SmallSize,
  alignof(HphpArray)
>::type s_theEmptyArray;

//=============================================================================

inline HphpArray* HphpArray::GetStaticEmptyArray() {
  void* vp = &s_theEmptyArray;
  return static_cast<HphpArray*>(vp);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HPHP_ARRAY_H_
