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
#include "hphp/runtime/base/smart-allocator.h"
#include "hphp/runtime/base/complex-types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArrayInit;
struct MemoryProfile;

class HphpArray : public ArrayData {
  enum SortFlavor { IntegerSort, StringSort, GenericSort };
public:
  friend class ArrayInit;

  // Load factor scaler.  If S is the # of elements, C is the
  // power-of-2 capacity, and L=LoadScale, we grow when S > C-C/L.
  // So 2 gives 0.5 load factor, 4 gives 0.75 load factor, 8 gives
  // 0.125 load factor.  Use powers of 2 to enable shift-divide.
  static const uint LoadScale = 4;

public:
  static HphpArray* GetStaticEmptyArray() {
    return &s_theEmptyArray;
  }

private:
  enum class CopyVector {};
  HphpArray(const HphpArray& other, AllocationMode, CopyVector);

  enum class CopyGeneric {};
  HphpArray(const HphpArray& other, AllocationMode, CopyGeneric);

  // convert in-place from kVector to kHphpArray: fill in keys & hashtable
  HphpArray* vectorToGeneric();

  // Safe downcast helpers
  static HphpArray* asVector(ArrayData* ad);
  static const HphpArray* asVector(const ArrayData* ad);
  static HphpArray* asGeneric(ArrayData* ad);
  static const HphpArray* asGeneric(const ArrayData* ad);
  static HphpArray* asHphpArray(ArrayData* ad);
  static const HphpArray* asHphpArray(const ArrayData* ad);

public:
  // Create an empty array with enough capacity for nSize elements.
  explicit HphpArray(uint nSize);

  // Create and initialize an array with size elements, populated by
  // moving (without refcounting) and reversing vals.
  HphpArray(uint size, const TypedValue* vals); // make tuple

  ~HphpArray();
  void destroyVec();
  void destroy();

  // This behaves the same as iter_begin except that it assumes
  // this array is not empty and its not virtual.
  ssize_t getIterBegin() const {
    assert(!empty());
    if (LIKELY(!isTombstone(m_data[0].data.m_type))) {
      return 0;
    }
    return nextElm(m_data, 0);
  }

  // override/implement ArrayData api's

  // these using directives ensure the full set of overloaded functions
  // are visible in this class, to avoid triggering implicit conversions
  // from a CVarRef key to int64.
  using ArrayData::exists;
  using ArrayData::lval;
  using ArrayData::lvalNew;
  using ArrayData::set;
  using ArrayData::setRef;
  using ArrayData::add;
  using ArrayData::addLval;
  using ArrayData::remove;
  using ArrayData::nvGet;

  // implements ArrayData
  static CVarRef GetValueRef(const ArrayData*, ssize_t pos);

  // overrides ArrayData
  static bool IsVectorData(const ArrayData*);
  static bool IsVectorDataVec(const ArrayData*);
  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t pos);
  static ssize_t IterRewind(const ArrayData*, ssize_t pos);

  // implements ArrayData
  static bool ExistsInt(const ArrayData*, int64_t k);
  static bool ExistsStr(const ArrayData*, const StringData* k);
  static bool ExistsIntVec(const ArrayData*, int64_t k);
  static bool ExistsStrVec(const ArrayData*, const StringData* k);

  // implements ArrayData
  static ArrayData* LvalInt(ArrayData* ad, int64_t k, Variant*& ret,
                            bool copy);
  static ArrayData* LvalStr(ArrayData* ad, StringData* k, Variant*& ret,
                            bool copy);
  static ArrayData* LvalIntVec(ArrayData* ad, int64_t k, Variant*& ret,
                               bool copy);
  static ArrayData* LvalStrVec(ArrayData* ad, StringData* k, Variant*& ret,
                               bool copy);
  static ArrayData* LvalNew(ArrayData*, Variant*& ret, bool copy);
  static ArrayData* LvalNewVec(ArrayData*, Variant*& ret, bool copy);

  // implements ArrayData
  static ArrayData* SetIntVec(ArrayData*, int64_t k, CVarRef v, bool copy);
  static ArrayData* SetStrVec(ArrayData*, StringData* k, CVarRef v, bool copy);
  static ArrayData* SetInt(ArrayData*, int64_t k, CVarRef v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, CVarRef v, bool copy);

  // implements ArrayData
  static ArrayData* SetRefInt(ArrayData* ad, int64_t k, CVarRef v,
                              bool copy);
  static ArrayData* SetRefStr(ArrayData* ad, StringData* k, CVarRef v,
                              bool copy);
  static ArrayData* SetRefIntVec(ArrayData* ad, int64_t k, CVarRef v,
                                 bool copy);
  static ArrayData* SetRefStrVec(ArrayData* ad, StringData* k, CVarRef v,
                                 bool copy);

  // overrides ArrayData
  static ArrayData* AddInt(ArrayData*, int64_t k, CVarRef v, bool copy);
  static ArrayData* AddStr(ArrayData*, StringData* k, CVarRef v, bool copy);
  static ArrayData* AddIntVec(ArrayData*, int64_t k, CVarRef v, bool copy);

  static ArrayData* AddLvalInt(ArrayData*, int64_t k, Variant*& ret,
                               bool copy);
  static ArrayData* AddLvalStr(ArrayData*, StringData* k, Variant*& ret,
                               bool copy);
  static ArrayData* AddLvalIntVec(ArrayData*, int64_t k, Variant*& ret,
                                  bool copy);

  // implements ArrayData
  static ArrayData* RemoveInt(ArrayData*, int64_t k, bool copy);
  static ArrayData* RemoveStr(ArrayData*, const StringData* k, bool copy);
  static ArrayData* RemoveIntVec(ArrayData*, int64_t k, bool copy);
  static ArrayData* RemoveStrVec(ArrayData*, const StringData* k, bool copy);

  // overrides ArrayData
  static ArrayData* Copy(const ArrayData*);
  static ArrayData* CopyVec(const ArrayData*);
  static ArrayData* CopyWithStrongIterators(const ArrayData*);
  static ArrayData* NonSmartCopy(const ArrayData*);

  HphpArray* copyImpl() const;
  HphpArray* copyVec() const;
  HphpArray* copyGeneric() const;

  static ArrayData* AppendVec(ArrayData*, CVarRef v, bool copy);
  static ArrayData* Append(ArrayData*, CVarRef v, bool copy);
  static ArrayData* AppendRef(ArrayData*, CVarRef v, bool copy);
  static ArrayData* AppendRefVec(ArrayData*, CVarRef v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, CVarRef v, bool copy);
  static ArrayData* AppendWithRefVec(ArrayData*, CVarRef v, bool copy);
  static ArrayData* Plus(ArrayData*, const ArrayData* elems, bool copy);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems, bool copy);
  static ArrayData* Pop(ArrayData*, Variant& value);
  static ArrayData* PopVec(ArrayData*, Variant& value);
  static ArrayData* Dequeue(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, CVarRef v, bool copy);
  static void Renumber(ArrayData*);
  static void RenumberVec(ArrayData*);
  static void OnSetEvalScalar(ArrayData*);
  static void OnSetEvalScalarVec(ArrayData*);

  // overrides ArrayData
  static bool ValidFullPos(const ArrayData*, const FullPos &fp);
  static bool AdvanceFullPos(ArrayData*, FullPos& fp);

  // END overide/implements section

  // nvGet and friends.
  // "nv" stands for non-variant. If we know the types of keys and values
  // through runtime and compile-time chicanery, we can directly call these
  // methods.

  // nvGet returns a pointer to the value if the specified key is in the
  // array, NULL otherwise.
  static TypedValue* NvGetIntVec(const ArrayData*, int64_t ki);
  static TypedValue* NvGetInt(const ArrayData*, int64_t ki);
  static TypedValue* NvGetStrVec(const ArrayData*, const StringData* k);
  static TypedValue* NvGetStr(const ArrayData*, const StringData* k);

  void nvBind(int64_t k, const TypedValue* v) {
    ArrayData::setRef(k, tvAsCVarRef(v), false);
  }
  void nvBind(StringData* k, const TypedValue* v) {
    ArrayData::setRef(k, tvAsCVarRef(v), false);
  }
  void nvAppend(const TypedValue* v) {
    nextInsertVec(tvAsCVarRef(v));
  }
  static void NvGetKeyVec(const ArrayData*, TypedValue* out, ssize_t pos);
  static void NvGetKey(const ArrayData*, TypedValue* out, ssize_t pos);
  bool nvInsert(StringData* k, TypedValue *v);

  /**
   * Main helper for AddNewElemC.  The semantics are slightly different from
   * other helpers, but tuned for the opcode.  The value to set is passed by
   * value; the caller has incref'd it if necessary, and this call *moves* it
   * to its location in the array (caller must not decref).  If the value cannot
   * be stored in the array, this helper decref's it.
   */
  static ArrayData* AddNewElemC(ArrayData* a, TypedValue value);

  /**
   * Inline helpers to be called directly from the TC
   */
  static TypedValue GetCellIntVec(const ArrayData* ad, int64_t ki);
  static uint64_t IssetIntVec(const ArrayData* ad, int64_t ki);

private:
  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc, bool checkTypes);

  void postSort(bool resetKeys);

public:
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

  // Array element.
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
      data.hash() = int32_t(h) | 0x80000000;
      k->incRefCount();
    }
    void setIntKey(int64_t k) {
      ikey = k;
      data.hash() = 0;
    }
  };

  struct ElmKey {
    ElmKey() {}
    ElmKey(int32_t hash, StringData* key) {
      this->hash = hash;
      this->key = key;
    }
    int32_t hash;
    union {
      StringData* key;
      int64_t ikey;
    };
  };

  // Element index, with special values < 0 used for hash tables.
  // NOTE: Unfortunately, g++ on x64 tends to generate worse machine code for
  // 32-bit ints than it does for 64-bit ints. As such, we have deliberately
  // chosen to use ssize_t in some places where ideally we *should* have used
  // ElmInd.
  typedef int32_t ElmInd;
  static const ElmInd ElmIndEmpty      = -1; // == ArrayData::invalid_index
  static const ElmInd ElmIndTombstone  = -2;

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

  uint32_t m_used;       // Number of used elements (values or tombstones)
  uint32_t m_cap;        // Number of Elms we can use before having to grow.
  uint32_t m_tableMask;  // Bitmask used when indexing into the hash table.
  uint32_t m_hLoad;      // Hash table load (# of non-empty slots).
  int64_t  m_nextKI;     // Next integer key to use for append.
  Elm*     m_data;       // Contains elements and hash table.
  ElmInd*  m_hash;       // Hash table.
  union {
    struct {
      Elm slots[SmallSize];
      ElmInd hash[SmallHashSize];
    } m_inline_data;
    ElmInd m_inline_hash[sizeof(m_inline_data) / sizeof(ElmInd)];
  };

  ssize_t nextElm(Elm* elms, ssize_t ei) const {
    assert(ei >= -1);
    while (size_t(++ei) < m_used) {
      if (!isTombstone(elms[ei].data.m_type)) {
        return ei;
      }
    }
    return (ssize_t)ElmIndEmpty;
  }
  ssize_t prevElm(Elm* elms, ssize_t ei) const;

  // Assert a bunch of invariants about this array then return true.
  // usage:  assert(checkInvariants());
  bool checkInvariants() const;

  static void getElmKey(const Elm& e, TypedValue* out);

  template <class Hit>
  ElmInd findBody(size_t h0, Hit) const;

  template <class Hit>
  ElmInd* findForInsertBody(size_t h0, Hit) const;

  ssize_t find(int64_t ki) const;
  ssize_t find(const StringData* s, strhash_t prehash) const;
  ElmInd* findForInsert(int64_t ki) const;
  ElmInd* findForInsert(const StringData* k, strhash_t prehash) const;

  ssize_t iter_advance_helper(ssize_t prev) const ATTRIBUTE_COLD;

  /**
   * findForNewInsert() CANNOT be used unless the caller can guarantee that
   * the relevant key is not already present in the array. Otherwise this can
   * put the array into a bad state; use with caution.
   */
  ElmInd* findForNewInsert(size_t h0) const;
  ElmInd* findForNewInsertLoop(size_t tableMask, size_t h0) const;

  bool nextInsert(CVarRef data);
  HphpArray* nextInsertVec(CVarRef data);
  ArrayData* nextInsertRef(CVarRef data);
  ArrayData* nextInsertWithRef(CVarRef data);
  ArrayData* addLvalImpl(int64_t ki, Variant*& ret);
  ArrayData* addLvalImpl(StringData* key, strhash_t h, Variant*& ret);
  ArrayData* addVal(int64_t ki, CVarRef data);
  ArrayData* addVal(StringData* key, CVarRef data);
  ArrayData* addValWithRef(int64_t ki, CVarRef data);
  ArrayData* addValWithRef(StringData* key, CVarRef data);

  ArrayData* update(int64_t ki, CVarRef data);
  ArrayData* update(StringData* key, CVarRef data);
  ArrayData* updateRef(int64_t ki, CVarRef data);
  ArrayData* updateRef(StringData* key, CVarRef data);

  void adjustFullPos(ElmInd pos);

  ArrayData* erase(ElmInd* ei, bool updateNext);

  HphpArray* copyImpl(HphpArray* target) const;

  bool isFull() const;
  Elm* newElm(ElmInd* e, size_t h0);
  Elm* newElmGrow(size_t h0);
  Elm* allocElm(ElmInd* ei);
  Elm* allocElmFast(ElmInd* ei);
  TypedValue& allocNextElm(uint32_t i);

  HphpArray* setVal(TypedValue& tv, CVarRef v);
  HphpArray* setRef(TypedValue& tv, CVarRef v);
  HphpArray* getLval(TypedValue& tv, Variant*& ret);
  HphpArray* initVal(TypedValue& tv, CVarRef v);
  HphpArray* initRef(TypedValue& tv, CVarRef v);
  HphpArray* initLval(TypedValue& tv, Variant*& ret);
  HphpArray* initWithRef(TypedValue& tv, CVarRef v);
  HphpArray* moveVal(TypedValue& tv, TypedValue v);

  ElmInd* allocData(size_t maxElms, size_t tableSize);
  ElmInd* reallocData(size_t maxElms, size_t tableSize);

  /**
   * grow() increases the hash table size and the number of slots for
   * elements by a factor of 2. grow() rebuilds the hash table, but it
   * does not compact the elements.
   */
  void grow() ATTRIBUTE_COLD;
  void growVec() ATTRIBUTE_COLD;

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

  // Memory allocator methods.
  DECLARE_SMART_ALLOCATION(HphpArray);
  static void ReleaseVec(ArrayData*);
  static void Release(ArrayData*);

private:
  enum EmptyMode { StaticEmptyArray };
  explicit HphpArray(EmptyMode);
  // static singleton empty array.  Not a subclass because we want a fast
  // isHphpArray implementation; HphpArray should be effectively final.
  static HphpArray s_theEmptyArray;

  void initHash(size_t tableSize);
  void initNonEmpty(const HphpArray& other);

public:
  static bool validElmInd(ssize_t /*HphpArray::ElmInd*/ ei) {
    return (ei > ssize_t(HphpArray::ElmIndEmpty));
  }

  static size_t computeTableSize(uint32_t tableMask) {
    return size_t(tableMask) + size_t(1U);
  }

  static size_t computeMaxElms(uint32_t tableMask) {
    return size_t(tableMask) - size_t(tableMask) / HphpArray::LoadScale;
  }

  static size_t computeDataSize(uint32_t tableMask) {
    return computeTableSize(tableMask) * sizeof(HphpArray::ElmInd) +
      computeMaxElms(tableMask) * sizeof(HphpArray::Elm);
  }

  friend struct MemoryProfile;
};

//=============================================================================
// inline for performance reasons

inline HphpArray* ArrayData::Make(uint capacity) {
  return NEW(HphpArray)(capacity);
}

// HphpArray has more than one kind, so reuse ArrayData's virtual dispatch.
inline void HphpArray::release() {
  ArrayData::release();
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HPHP_ARRAY_H_
