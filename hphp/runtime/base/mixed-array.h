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
#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

class ArrayInit;
struct MemoryProfile;

//////////////////////////////////////////////////////////////////////

struct MixedArray : private ArrayData {
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
      StringData* skey;
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

    int32_t probe() const {
      return hasIntKey() ? ikey : hash();
    }

    void setStaticKey(StringData* k, strhash_t h) {
      assert(k->isStatic());
      skey = k;
      data.hash() = h | STRHASH_MSB;
    }

    void setStrKey(StringData* k, strhash_t h) {
      skey = k;
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
    return sizeof(MixedArray);
  }

  /*
   * Allocate a new, empty, request-local array in packed mode, with
   * enough space reserved for `capacity' members.
   *
   * The returned array is already incref'd.
   */
  static ArrayData* MakeReserve(uint32_t capacity);
  static ArrayData* MakeReserveSlow(uint32_t capacity);

  /*
   * Allocate a new, empty, request-local array in mixed mode, with
   * enough space reserved for `capacity' members.
   *
   * The returned array is already incref'd.
   */
  static ArrayData* MakeReserveMixed(uint32_t capacity);

  /*
   * Allocate a new, empty, request-local array with the same mode as
   * `other' and with enough space reserved for `capacity' members, or
   * if `capacity' is zero, with the same capacity as `other'.
   *
   * The returned array is already incref'd.
   */
  static ArrayData* MakeReserveLike(const ArrayData* other, uint32_t capacity);

  /*
   * Allocate a packed MixedArray.  This is an array in packed
   * mode, containing `size' values, in the reverse order of the
   * `values' array.
   *
   * This function takes ownership of the TypedValues in `values'.
   *
   * The returned array is already incref'd.
   *
   * Pre: size > 0
   */
  static ArrayData* MakePacked(uint32_t size, const TypedValue* values);
  static ArrayData* MakePackedHelper(uint32_t size, const TypedValue* values);

  /*
   * Allocate a new, empty, request-local array in int map/string map mode, with
   * enough space reserved for `capacity' members.
   *
   * The returned array is already incref'd.
   */
  static ArrayData* MakeReserveIntMap(uint32_t capacity);
  static ArrayData* MakeReserveStrMap(uint32_t capacity);

  /*
   * Like MakePacked, but given static strings, make a struct-like array.
   * Also requires size > 0.
   */
  static MixedArray* MakeStruct(uint32_t size, StringData** keys,
                               const TypedValue* values);

  /*
   * Allocate an uncounted MixedArray and copy the values from the
   * input 'array' into the uncounted one. All values copied are made
   * uncounted as well.  An uncounted array can only contain uncounted
   * values (primitive values, uncounted or static strings and
   * uncounted or static arrays).  The Packed version does the same
   * when the array has a kPackedKind.
   */
  static ArrayData* MakeUncounted(ArrayData* array);
  static ArrayData* MakeUncountedPacked(ArrayData* array);
  static ArrayData* MakeUncountedPackedHelper(ArrayData* array);

  // This behaves the same as iter_begin except that it assumes
  // this array is not empty and its not virtual.
  ALWAYS_INLINE
  ssize_t getIterBegin() const {
    assert(!empty());
    if (LIKELY(!isTombstone(data()[0].data.m_type))) {
      return 0;
    }
    return nextElm(data(), 0);
  }

  using ArrayData::decRefCount;
  using ArrayData::hasMultipleRefs;
  using ArrayData::hasExactlyOneRef;
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
public:

  static size_t Vsize(const ArrayData*);
  static const Variant& GetValueRef(const ArrayData*, ssize_t pos);
  static bool IsVectorData(const ArrayData*);
  static const TypedValue* NvGetInt(const ArrayData*, int64_t ki);
  static const TypedValue* NvGetIntConverted(const ArrayData*, int64_t ki);
  static const TypedValue* NvGetStr(const ArrayData*, const StringData* k);
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
  static ArrayData* LvalStr(ArrayData* ad, StringData* k, Variant*& ret,
                            bool copy);
  static ArrayData* LvalNew(ArrayData*, Variant*& ret, bool copy);
  static ArrayData* SetInt(ArrayData*, int64_t k, Cell v, bool copy);
  static ArrayData* SetIntConverted(ArrayData*, int64_t k, Cell v, bool copy);
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
  static ArrayData* NonSmartCopy(const ArrayData*);
  static ArrayData* Append(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendRef(ArrayData*, Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, const Variant& v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Pop(ArrayData*, Variant& value);
  static ArrayData* Dequeue(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, const Variant& v, bool copy);
  static void Renumber(ArrayData*);
  static void OnSetEvalScalar(ArrayData*);
  static void Release(ArrayData*);
  static void ReleaseUncounted(ArrayData*);
  static void ReleaseUncountedPacked(ArrayData*);
  static constexpr auto ValidMArrayIter = &ArrayCommon::ValidMArrayIter;
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter& fp);
  static ArrayData* Escalate(const ArrayData* ad) {
    return const_cast<ArrayData*>(ad);
  }

  static ArrayData* EscalateForSort(ArrayData* ad);
  static void Ksort(ArrayData*, int sort_flags, bool ascending);
  static void Sort(ArrayData*, int sort_flags, bool ascending);
  static void Asort(ArrayData*, int sort_flags, bool ascending);
  static bool Uksort(ArrayData*, const Variant& cmp_function);
  static bool Usort(ArrayData*, const Variant& cmp_function);
  static bool Uasort(ArrayData*, const Variant& cmp_function);
  static void WarnAndSort(ArrayData*, int sort_flags, bool ascending);
  static bool WarnAndUsort(ArrayData*, const Variant& cmp_function);


  template <ArrayKind aKind>
  static const TypedValue* NvGetStrImpl(const ArrayData*, const StringData* k);
  template <ArrayKind aKind>
  static const TypedValue* NvGetIntImpl(const ArrayData*, int64_t ki);
  template <ArrayKind aKind>
  static bool ExistsIntImpl(const ArrayData*, int64_t k);
  template <ArrayKind aKind>
  static bool ExistsStrImpl(const ArrayData*, const StringData* k);
  template <ArrayKind aKind>
  static ArrayData* LvalIntImpl(ArrayData* ad, int64_t k, Variant*& ret,
                                bool copy);
  template <ArrayKind aKind>
  static ArrayData* LvalStrImpl(ArrayData* ad, StringData* k, Variant*& ret,
                                bool copy);
  template <ArrayKind aKind>
  static ArrayData* LvalNewImpl(ArrayData*, Variant*& ret, bool copy);
  template <ArrayKind aKind>
  static ArrayData* SetStrImpl(ArrayData*, StringData* k, Cell v, bool copy);
  template <ArrayKind aKind>
  static ArrayData* SetIntImpl(ArrayData*, int64_t k, Cell v, bool copy);
  template <ArrayKind aKind>
  static ArrayData* SetRefIntImpl(ArrayData* ad, int64_t k, Variant& v,
                                  bool copy);
  template <ArrayKind aKind>
  static ArrayData* SetRefStrImpl(ArrayData* ad, StringData* k, Variant& v,
                              bool copy);
  template <ArrayKind aKind>
  static ArrayData* AddStrImpl(ArrayData*, StringData* k, Cell v, bool copy);
  template <ArrayKind aKind>
  static ArrayData* RemoveIntImpl(ArrayData*, int64_t k, bool copy);
  template <ArrayKind aKind>
  static ArrayData* RemoveStrImpl(ArrayData*, const StringData* k, bool copy);
  template <ArrayKind aKind>
  static ArrayData* AppendImpl(ArrayData*, const Variant& v, bool copy);
  template <ArrayKind aKind>
  static ArrayData* AppendRefImpl(ArrayData*, Variant& v, bool copy);
  template <ArrayKind aKind>
  static ArrayData* AppendWithRefImpl(ArrayData*, const Variant& v, bool copy);
  template <ArrayData::ArrayKind aKind>
  static ArrayData* PopImpl(ArrayData* ad, Variant& value);
  template <ArrayData::ArrayKind aKind>
  static ArrayData* DequeueImpl(ArrayData* adInput, Variant& value);

  template <ArrayKind aKind>
  static void RenumberImpl(ArrayData*);
  template <ArrayKind aKind>
  static bool AdvanceMArrayIterImpl(ArrayData*, MArrayIter& fp);

private:
  MixedArray* copyMixed() const;
  MixedArray* copyMixedAndResizeIfNeeded() const;
  MixedArray* copyMixedAndResizeIfNeededSlow() const;

public:
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
  static const int32_t Empty      = -1;
  static const int32_t Tombstone  = -2;

  // Use a minimum of an 4-element hash table.  Valid range: [2..32]
  static const uint32_t MinLgTableSize = 2;
  static const uint32_t SmallHashSize = 1 << MinLgTableSize;
  static const uint32_t SmallMask = SmallHashSize - 1;
  static const uint32_t SmallSize = SmallHashSize - SmallHashSize / LoadScale;

  static const uint32_t MaxLgTableSize = 32;
  static const uint64_t MaxHashSize = uint64_t(1) << 32;
  static const uint32_t MaxMask = MaxHashSize - 1;
  static const uint32_t MaxSize = MaxMask - MaxMask / LoadScale;
  static const uint32_t MaxMakeSize = 4 * SmallSize;

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

  size_t hashSize() const;
  static size_t computeMaxElms(uint32_t tableMask);
  static size_t computeDataSize(uint32_t tableMask);

private:
  friend struct ArrayInit;
  friend struct MemoryProfile;
  friend struct EmptyArray;
  friend struct PackedArray;
  friend class HashCollection;
  friend class BaseMap;
  friend class c_Map;
  friend class c_ImmMap;
  friend class BaseSet;
  friend class c_Set;
  friend class c_ImmSet;
  enum class ClonePacked {};
  enum class CloneMixed {};
  enum SortFlavor { IntegerSort, StringSort, GenericSort };

  friend size_t getMemSize(const ArrayData*);

public:
  // Safe downcast helpers
  static MixedArray* asMixed(ArrayData* ad);
  static const MixedArray* asMixed(const ArrayData* ad);

  enum class Reason : uint8_t {
    kForeachByRef,
    kPrepend,
    kPop,
    kTakeByRef,
    kSetRef,
    kAppendRef,
    kAppend,
    kNvGetInt,
    kNvGetStr,
    kExistsInt,
    kExistsStr,
    kSetInt,
    kSetStr,
    kRemoveInt,
    kRemoveStr,
    kDequeue,
    kSort,
    kUsort,
    kNumericString,
    kRenumber,
  };
  static void downgradeAndWarn(ArrayData* ad, const Reason r);
  static void warnUsage(const Reason r, const ArrayKind kind);

private:
  static void getElmKey(const Elm& e, TypedValue* out);

private:
  enum class AllocMode : bool { Smart, NonSmart };

  template<class CopyKeyValue>
  static MixedArray* CopyMixed(const MixedArray& other,
                               AllocMode,
                               CopyKeyValue);
  static MixedArray* CopyReserve(const MixedArray* src, size_t expectedSize);

  MixedArray() = delete;
  MixedArray(const MixedArray&) = delete;
  MixedArray& operator=(const MixedArray&) = delete;
  ~MixedArray() = delete;

private:
  static void initHash(int32_t* table, size_t tableSize);
  static int32_t* copyHash(int32_t* to, const int32_t* from, size_t tableSize);
  static Elm* copyElms(Elm* to, const Elm* from, size_t count);

  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc, bool checkTypes);
  void postSort(bool resetKeys);
  static ArrayData* ArrayPlusEqGeneric(ArrayData*,
    MixedArray*, const ArrayData*, size_t);
  static ArrayData* ArrayMergeGeneric(MixedArray*, const ArrayData*);

  // convert in-place from kPackedKind to kMixedKind: fill in keys & hashtable
  MixedArray* packedToMixed();

  ssize_t nextElm(Elm* elms, ssize_t ei) const {
    assert(ei >= -1);
    while (size_t(++ei) < m_used) {
      if (!isTombstone(elms[ei].data.m_type)) {
        return ei;
      }
    }
    return m_used;
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
   * put the array into a bad state; use with caution. The *CheckUnbalanced
   * version checks for the array becoming too unbalanced because of hash
   * collisions, and is only called when an array Grow()s.
   */
  int32_t* findForNewInsert(size_t h0) const;
  int32_t* findForNewInsert(int32_t* table, size_t mask, size_t h0) const;
  int32_t* findForNewInsertCheckUnbalanced(int32_t* table,
                                           size_t mask, size_t h0) const;

  bool nextInsert(const Variant& data);
  ArrayData* nextInsertRef(Variant& data);
  ArrayData* nextInsertWithRef(const Variant& data);
  ArrayData* addVal(int64_t ki, Cell data);
  ArrayData* addVal(StringData* key, Cell data);

  template <class K> ArrayData* addLvalImpl(K k, Variant*& ret);
  template <class K> ArrayData* update(K k, Cell data);
  template <class K> ArrayData* updateRef(K k, Variant& data);

  template <class K> ArrayData* zSetImpl(K k, RefData* data);
  ArrayData* zAppendImpl(RefData* data, int64_t* key_ptr);

  void adjustMArrayIter(ssize_t pos);
  void eraseNoCompact(ssize_t pos);
  void erase(ssize_t pos) {
    eraseNoCompact(pos);
    if (m_size < m_used / 2) {
      // Compact in order to keep elms from being overly sparse.
      compact(false);
    }
  }

  MixedArray* copyImpl(MixedArray* target) const;

  bool isFull() const;
  bool isFullPacked() const;

  // Pre: !isFullPacked()
  TypedValue& allocNextElm(uint32_t i);

  Elm& allocElm(int32_t* ei);

  MixedArray* setVal(TypedValue& tv, Cell v);
  MixedArray* getLval(TypedValue& tv, Variant*& ret);
  MixedArray* initVal(TypedValue& tv, Cell v);
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
  static void InsertCheckUnbalanced(MixedArray* ad, int32_t* table,
                                    uint32_t mask,
                                    Elm* iter, Elm* stop);
  /*
   * grow() increases the hash table size and the number of slots for
   * elements by a factor of 2. grow() rebuilds the hash table, but it
   * does not compact the elements.
   */
  static MixedArray* Grow(MixedArray* old, uint32_t newCap, uint32_t newMask);
  static MixedArray* GrowPacked(MixedArray* old);

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
  MixedArray* resizePackedIfNeeded();

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
  void setZombie() { m_used = -uint32_t{1}; }

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
  uint32_t m_tableMask;     // Bitmask used when indexing into the hash table.
  UNUSED uint32_t m_unused2;
  int64_t  m_nextKI;        // Next integer key to use for append.
};

//////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_HPHP_ARRAY_H_
