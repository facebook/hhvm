/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include "hphp/runtime/base/array/array_data.h"
#include "hphp/runtime/base/memory/smart_allocator.h"
#include "hphp/runtime/base/complex_types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArrayInit;

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
  // for copy-on-write escalation
  explicit HphpArray(AllocMode);

public:
  // Create an empty array with enough capacity for nSize elements.
  explicit HphpArray(uint nSize);

  // Create and initialize an array with size elements, populated by
  // moving (without refcounting) and reversing vals.
  HphpArray(uint size, const TypedValue* vals); // make tuple

  virtual ~HphpArray();

  // unlike ArrayData::size(), this functions doesn't delegate
  // to the virtual vsize() functions, so its more efficient to
  // use this when you know you have an HphpArray.
  ssize_t getSize() const {
    return m_size;
  }

  // This behaves the same as iter_begin except that it assumes
  // this array is not empty and its not virtual.
  ssize_t getIterBegin() const {
    assert(!empty());
    if (LIKELY((m_data[0].data.m_type < KindOfTombstone))) {
      return 0;
    }
    return nextElm(m_data, 0);
  }

  // override/implement ArrayData api's

  // these using directives ensure the full set of overloaded functions
  // are visible in this class, to avoid triggering implicit conversions
  // from a CVarRef key to int64.
  using ArrayData::exists;
  using ArrayData::get;
  using ArrayData::getIndex;
  using ArrayData::lval;
  using ArrayData::lvalNew;
  using ArrayData::lvalPtr;
  using ArrayData::set;
  using ArrayData::setRef;
  using ArrayData::add;
  using ArrayData::addLval;
  using ArrayData::remove;

  // implements ArrayData
  ssize_t vsize() const;
  Variant getKey(ssize_t pos) const;
  Variant getValue(ssize_t pos) const;
  CVarRef getValueRef(ssize_t pos) const;

  // overrides ArrayData
  bool isVectorData() const;
  ssize_t iter_begin() const;
  ssize_t iter_end() const;
  ssize_t iter_advance(ssize_t prev) const;
  ssize_t iter_rewind(ssize_t prev) const;

  // overrides ArrayData
  Variant reset();
  Variant prev();
  Variant current() const;
  Variant next();
  Variant end();
  Variant key() const;
  Variant value(ssize_t& pos) const;
  Variant each();

  // implements ArrayData
  bool exists(int64_t k) const;
  bool exists(const StringData* k) const;

  // implements ArrayData
  CVarRef get(int64_t k, bool error=false) const FLATTEN;
  CVarRef get(const StringData* k, bool error=false) const FLATTEN;

  // implements ArrayData
  ssize_t getIndex(int64_t k) const;
  ssize_t getIndex(const StringData* k) const;

  // implements ArrayData
  ArrayData* lval(int64_t k, Variant*& ret, bool copy, bool checkExist=false);
  ArrayData* lval(StringData* k, Variant*& ret, bool copy,
                  bool checkExist=false);
  ArrayData* lvalNew(Variant*& ret, bool copy);

  // overrides ArrayData
  ArrayData* lvalPtr(StringData* k, Variant*& ret, bool copy,
                     bool create);

  // implements ArrayData
  ArrayData* set(int64_t k, CVarRef v, bool copy);
  ArrayData* set(StringData* k, CVarRef v, bool copy);

  // implements ArrayData
  ArrayData* setRef(int64_t k, CVarRef v, bool copy);
  ArrayData* setRef(StringData* k, CVarRef v, bool copy);

  // overrides ArrayData
  ArrayData *add(int64_t k, CVarRef v, bool copy);
  ArrayData *add(StringData* k, CVarRef v, bool copy);
  ArrayData *addLval(int64_t k, Variant*& ret, bool copy);
  ArrayData *addLval(StringData* k, Variant*& ret, bool copy);

  // implements ArrayData
  ArrayData* remove(int64_t k, bool copy);
  ArrayData* remove(const StringData* k, bool copy);

  // overrides/implements ArrayData
  ArrayData* copy() const;
  ArrayData* copyWithStrongIterators() const;

  ArrayData* nonSmartCopy() const;
  HphpArray* copyImpl() const;

  ArrayData* append(CVarRef v, bool copy);
  ArrayData* appendRef(CVarRef v, bool copy);
  ArrayData* appendWithRef(CVarRef v, bool copy);
  ArrayData* append(const ArrayData* elems, ArrayOp op, bool copy);
  ArrayData* pop(Variant& value);
  ArrayData* dequeue(Variant& value);
  ArrayData* prepend(CVarRef v, bool copy);
  void renumber();
  void onSetEvalScalar();

  // overrides ArrayData
  bool validFullPos(const FullPos &fp) const;
  bool advanceFullPos(FullPos& fp);
  CVarRef currentRef();
  CVarRef endRef();

  // END overide/implements section

  // nvGet and friends.
  // "nv" stands for non-variant. If we know the types of keys and values
  // through runtime and compile-time chicanery, we can directly call these
  // methods.

  // nvGet returns a pointer to the value if the specified key is in the
  // array, NULL otherwise.
  TypedValue* nvGet(int64_t ki) const;
  TypedValue* nvGet(const StringData* k) const;

  // nvGetCell is a variation of get, however it unwraps a KindOfRef,
  // returns KindOfNull if the key doesn't exist, and always warns.
  TypedValue* nvGetCell(int64_t ki) const;
  TypedValue* nvGetCell(const StringData* k) const;

  void nvBind(int64_t ki, const TypedValue* v) {
    updateRef(ki, tvAsCVarRef(v));
  }
  void nvBind(StringData* k, const TypedValue* v) {
    updateRef(k, tvAsCVarRef(v));
  }
  void nvAppend(const TypedValue* v) {
    nextInsert(tvAsCVarRef(v));
  }
  ArrayData* nvNew(TypedValue*& v, bool copy);
  TypedValue* nvGetValueRef(ssize_t pos);
  void nvGetKey(TypedValue* out, ssize_t pos);
  bool nvInsert(StringData* k, TypedValue *v);

  /**
   * Main helper for AddNewElemC.  The semantics are slightly different from
   * other helpers, but tuned for the opcode.  The value to set is passed by
   * value; the caller has incref'd it if necessary, and this call *moves* it
   * to its location in the array (caller must not decref).  If the value cannot
   * be stored in the array, this helper decref's it.
   */
  static ArrayData* AddNewElemC(ArrayData* a, TypedValue value);

private:
  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc, bool checkTypes);

  void postSort(bool resetKeys);

public:
  ArrayData* escalateForSort();
  void ksort(int sort_flags, bool ascending);
  void sort(int sort_flags, bool ascending);
  void asort(int sort_flags, bool ascending);
  void uksort(CVarRef cmp_function);
  void usort(CVarRef cmp_function);
  void uasort(CVarRef cmp_function);
  void dumpDebugInfo() const;

  // Used in Elm's data.m_type field to denote an invalid Elm.
  static const HPHP::DataType KindOfTombstone = MaxNumDataTypes;

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
    // data.m_type == KindOfTombstone if this is an empty slot in the
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
  static const uint32_t SmallSize = SmallHashSize - SmallHashSize / LoadScale;

  struct InlineSlots {
    Elm slots[SmallSize];
    ElmInd hash[SmallHashSize];
  };

  ElmInd getLastE() const { return m_lastE; }
  Elm*   getElm(ssize_t pos)  const {
    assert(unsigned(pos) <= unsigned(m_lastE));
    return &m_data[pos];
  }
  static void getElmKey(Elm* e, TypedValue* out) {
    if (e->hasIntKey()) {
      out->m_data.num = e->ikey;
      out->m_type = KindOfInt64;
      return;
    }
    out->m_data.pstr = e->key;
    out->m_type = KindOfString;
    e->key->incRefCount();
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

  ElmInd  m_lastE;       // Index of last used element.
  Elm*    m_data;        // Contains elements and hash table.
  ElmInd* m_hash;        // Hash table.
  int64_t   m_nextKI;      // Next integer key to use for append.
  uint32_t  m_tableMask;   // Bitmask used when indexing into the hash table.
  uint32_t  m_hLoad;       // Hash table load (# of non-empty slots).
  union {
    InlineSlots m_inline_data;
    ElmInd m_inline_hash[sizeof(m_inline_data) / sizeof(ElmInd)];
  };

  ssize_t /*ElmInd*/ nextElm(Elm* elms, ssize_t /*ElmInd*/ ei) const {
    assert(ei >= -1);
    ssize_t lastE = m_lastE;
    while (ei < lastE) {
      ++ei;
      if (elms[ei].data.m_type < KindOfTombstone) {
        return ei;
      }
    }
    return (ssize_t)ElmIndEmpty;
  }
  ssize_t /*ElmInd*/ prevElm(Elm* elms, ssize_t /*ElmInd*/ ei) const;

  ssize_t /*ElmInd*/ find(int64_t ki) const;
  ssize_t /*ElmInd*/ find(const StringData* s, strhash_t prehash) const;
  ElmInd* findForInsert(int64_t ki) const;
  ElmInd* findForInsert(const StringData* k, strhash_t prehash) const;

  ssize_t iter_advance_helper(ssize_t prev) const ATTRIBUTE_COLD;

  /**
   * findForNewInsert() CANNOT be used unless the caller can guarantee that
   * the relevant key is not already present in the array. Otherwise this can
   * put the array into a bad state; use with caution.
   */
  ElmInd* findForNewInsertLoop(size_t tableMask, size_t h0) const;

  inline ALWAYS_INLINE
  ElmInd* findForNewInsert(size_t h0) const {
    size_t tableMask = m_tableMask;
    size_t probeIndex = h0 & tableMask;
    ElmInd* ei = &m_hash[probeIndex];
    return !validElmInd(*ei) ? ei :
           findForNewInsertLoop(tableMask, h0);
  }

  bool nextInsert(CVarRef data);
  ArrayData* nextInsertRef(CVarRef data);
  ArrayData* nextInsertWithRef(CVarRef data);
  ArrayData* addLvalImpl(int64_t ki, Variant** pDest);
  ArrayData* addLvalImpl(StringData* key, strhash_t h, Variant** pDest);
  ArrayData* addVal(int64_t ki, CVarRef data);
  ArrayData* addVal(StringData* key, CVarRef data);
  ArrayData* addValWithRef(int64_t ki, CVarRef data);
  ArrayData* addValWithRef(StringData* key, CVarRef data);

  ArrayData* update(int64_t ki, CVarRef data);
  ArrayData* update(StringData* key, CVarRef data);
  ArrayData* updateRef(int64_t ki, CVarRef data);
  ArrayData* updateRef(StringData* key, CVarRef data);

  ArrayData* erase(ElmInd* ei, bool updateNext = false);

  HphpArray* copyImpl(HphpArray* target) const;

  bool isFull() const;
  Elm* newElm(ElmInd* e, size_t h0);
  Elm* newElmGrow(size_t h0);
  Elm* allocElm(ElmInd* ei);
  Elm* allocElmFast(ElmInd* ei);
  void initElmInt(Elm* e, int64_t ki, CVarRef data, bool byRef=false);
  void initElmStr(Elm* e, strhash_t h, StringData* key, CVarRef data,
                  bool byRef=false);
  void newElmInt(ElmInd* ei, int64_t ki, CVarRef data,
                      bool byRef=false);
  void newElmStr(ElmInd* ei, strhash_t h, StringData* key, CVarRef data,
                      bool byRef=false);
  void allocData(size_t maxElms, size_t tableSize);
  void reallocData(size_t maxElms, size_t tableSize, uint oldMask);

  /**
   * init(size) allocates space for size elements but initializes
   * as an empty array
   */
  void init(uint size);

  /**
   * grow() increases the hash table size and the number of slots for
   * elements by a factor of 2. grow() rebuilds the hash table, but it
   * does not compact the elements.
   */
  void grow() ATTRIBUTE_COLD;

  /**
   * compact() does not change the hash table size or the number of slots
   * for elements. compact() rebuilds the hash table and compacts the
   * elements into the slots with lower addresses.
   */
  void compact(bool renumber=false) ATTRIBUTE_COLD;

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

private:
  enum EmptyMode { StaticEmptyArray };
  explicit HphpArray(EmptyMode);
  // static singleton empty array.  Not a subclass because we want a fast
  // isHphpArray implementation; HphpArray should be effectively final.
  static HphpArray s_theEmptyArray;

  static void initHash(HphpArray::ElmInd* hash, size_t tableSize) {
    assert(HphpArray::ElmIndEmpty == -1);
    memset(hash, 0xffU, tableSize * sizeof(HphpArray::ElmInd));
  }

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

private:
  HphpArray* clone(AllocMode am) const;
  void cloneNonEmpty(HphpArray* target) const;
};

//=============================================================================
// VM runtime support functions.

enum ArrayGetFlags {
  DecRefKey = 1,
  CheckInts = 2
};

ArrayData* array_setm_ik1_v(RefData* ref, ArrayData* ad, int64_t key,
                            TypedValue* value);
ArrayData* array_setm_ik1_v0(RefData* ref, ArrayData* ad, int64_t key,
                             TypedValue* value);
ArrayData* array_setm_sk1_v(RefData* ref, ArrayData* ad, StringData* key,
                            TypedValue* value);
ArrayData* array_setm_sk1_v0(RefData* ref, ArrayData* ad, StringData* key,
                             TypedValue* value);
ArrayData* array_setm_s0k1_v(RefData* ref, ArrayData* ad, StringData* key,
                             TypedValue* value);
ArrayData* array_setm_s0k1_v0(RefData* ref, ArrayData* ad, StringData* key,
                              TypedValue* value);
ArrayData* array_setm_s0k1nc_v(RefData* ref, ArrayData* ad, StringData* key,
                               TypedValue* value);
ArrayData* array_setm_s0k1nc_v0(RefData* ref, ArrayData* ad,
                                StringData* key, TypedValue* value);
ArrayData* array_setm_wk1_v0(ArrayData* ad, TypedValue* value);
ArrayData* array_getm_i(void* hphpArray, int64_t key, TypedValue* out);

ArrayData* array_getm_s(ArrayData* a, StringData* key, TypedValue* out,
                        int flags);
uint64_t array_issetm_s(const void* hphpArray, StringData* sd)
  FLATTEN;
uint64_t array_issetm_s0(const void* hphpArray, StringData* sd)
  FLATTEN;
uint64_t array_issetm_s_fast(const void* hphpArray, StringData* sd)
  FLATTEN;
uint64_t array_issetm_s0_fast(const void* hphpArray, StringData* sd)
  FLATTEN;
uint64_t array_issetm_i(const void* hphpArray, int64_t key)
  FLATTEN;
ArrayData* array_add(ArrayData* a1, ArrayData* a2);

//=============================================================================

// inline for performance reasons

inline HphpArray* ArrayData::Make(uint capacity) {
  return NEW(HphpArray)(capacity);
}

inline HphpArray* ArrayData::Make(uint size, const TypedValue* data) {
  return NEW(HphpArray)(size, data);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HPHP_ARRAY_H_
