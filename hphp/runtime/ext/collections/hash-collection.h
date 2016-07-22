#ifndef incl_HPHP_COLLECTIONS_HASHCOLLECTION_H
#define incl_HPHP_COLLECTIONS_HASHCOLLECTION_H

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/base/mixed-array-defs.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct Header;

// Align to 16-byte boundaries.
using EmptyMixedArrayStorage = std::aligned_storage<
  computeAllocBytes(MixedArray::SmallScale), 16>::type;
extern EmptyMixedArrayStorage s_theEmptyMixedArray;

/*
 * This returns a static empty MixedArray. This gets used internally
 * within the BaseMap implementation but it is not exposed outside of
 * BaseMap.
 */
ALWAYS_INLINE MixedArray* staticEmptyMixedArray() {
  void* vp = &s_theEmptyMixedArray;
  return reinterpret_cast<MixedArray*>(vp);
}

// Common base class for BaseMap/BaseSet collections
struct HashCollection : ObjectData {
  explicit HashCollection(Class* cls, HeaderKind kind)
    : ObjectData(cls, collections::objectFlags, kind)
    , m_versionAndSize(0)
    , m_arr(staticEmptyMixedArray())
  {}
  explicit HashCollection(Class* cls, HeaderKind kind, ArrayData* arr)
    : ObjectData(cls, collections::objectFlags, kind)
    , m_versionAndSize(arr->m_size)
    , m_arr(MixedArray::asMixed(arr))
  {}
  explicit HashCollection(Class* cls, HeaderKind kind, uint32_t cap);

  using Elm = MixedArray::Elm;
  using hash_t = MixedArray::hash_t;

  static const int32_t Empty           = MixedArray::Empty;
  static const int32_t Tombstone       = MixedArray::Tombstone;
  static const uint32_t LoadScale      = MixedArray::LoadScale;
  static const uint32_t SmallScale     = MixedArray::SmallScale;
  static const uint32_t SmallSize      = MixedArray::SmallSize;
  static const uint32_t MaxSize        = MixedArray::MaxSize;
  // HashCollections can only guarantee that it won't throw "cannot add
  // element" exceptions if m_size <= MaxSize / 2. Therefore, we only allow
  // reserve() to make room for up to MaxSize / 2 elements.
  static const uint32_t MaxReserveSize = MaxSize / 2;

  int getVersion() const {
    return m_version;
  }
  int64_t size() const {
    return m_size;
  }

  Array toArray();
  Array toKeysArray();
  Array toValuesArray();

  bool contains(int64_t key) const {
    return find(key, hashint(key)) != Empty;
  }
  bool contains(StringData* key) const {
    return find(key, key->hash()) != Empty;
  }

  void remove(int64_t key);
  void remove(StringData* key);
  void eraseNoCompact(ssize_t pos);
  void erase(ssize_t pos) {
    eraseNoCompact(pos);
    compactOrShrinkIfDensityTooLow();
  }

  void asort(int sort_flags, bool ascending);
  void ksort(int sort_flags, bool ascending);
  bool uasort(const Variant& cmp_function);
  bool uksort(const Variant& cmp_function);

  bool isFull() { return posLimit() == cap(); }
  bool isDensityTooLow() const {
    bool b = (m_size < posLimit() / 2);
    assert(IMPLIES(data() == mixedData(staticEmptyMixedArray()), !b));
    assert(IMPLIES(cap() == 0, !b));
    return b;
  }

  bool isCapacityTooHigh() const {
    // Return true if current capacity at least 8x greater than m_size AND
    // if current capacity is at least 8x greater than the minimum capacity
    bool b = ((uint64_t(cap()) >= uint64_t(m_size) * 8) &&
              (cap() >= HashCollection::SmallSize * 8));
    assert(IMPLIES(data() == mixedData(staticEmptyMixedArray()), !b));
    assert(IMPLIES(cap() == 0, !b));
    return b;
  }

  // grow() will increase the capacity of this HashCollection; newScale must
  // be greater than or equal to the current scale so that the new cap and mask
  // satisfy all the usual cap/mask invariants.
  void grow(uint32_t newScale);

  // resizeHelper() dups all of the elements (not copying tombstones) to a
  // new buffer of the specified capacity and decRefs the old buffer. This
  // method can be used to decrease this HashCollection's capacity.
  void resizeHelper(uint32_t newCap);

  // This method will increase capacity or compact as needed to make
  // room to add one new element; it asserts that is is only called
  // when isFull() is true
  void makeRoom();

  // This method performs an in-place compaction; it asserts that it
  // is only called when isDensityTooLow() is true
  void compact();

  // This method reduces this HashCollection's capacity; it asserts that it
  // is only called when isCapacityTooHigh() is true.
  void shrink(uint32_t cap = 0);

  // In general this method should be called after one or more elements
  // have been removed. If density is too low, it will shrink or compact
  // this HashCollection as appropriate.
  void compactOrShrinkIfDensityTooLow() {
    if (UNLIKELY(isDensityTooLow())) {
      if (isCapacityTooHigh()) {
        shrink();
      } else {
        compact();
      }
    }
  }

  // In general this method should be called after a speculative reserve
  // and zero or more adds have been performed. If capacity is too high,
  // it will shrink this HashCollection.
  void shrinkIfCapacityTooHigh(uint32_t oldCap) {
    if (UNLIKELY(isCapacityTooHigh() && cap() > oldCap)) {
      shrink(oldCap);
    }
  }

  HashCollection::Elm& allocElm(int32_t* ei) {
    assert(canMutateBuffer());
    assert(ei && !validPos(*ei) && m_size <= posLimit() && posLimit() < cap());
    size_t i = posLimit();
    *ei = i;
    setPosLimit(i + 1);
    incSize();
    return data()[i];
  }

  HashCollection::Elm& allocElmFront(int32_t* ei);

  // This method will grow or compact as needed in preparation for
  // repeatedly adding new elements until m_size >= sz.
  void reserve(int64_t sz);

  // The iter functions below facilitate iteration over HashCollections.
  // Iterators cannot store Elm pointers (because it's possible for m_data
  // to change without bumping m_version in some cases), so indices are
  // used instead.

  bool iter_valid(ssize_t pos) const {
    return pos < (ssize_t)posLimit();
  }

  bool iter_valid(ssize_t pos, ssize_t limit) const {
    assert(limit == (ssize_t)posLimit());
    return pos < limit;
  }

  const Elm* iter_elm(ssize_t pos) const {
    assert(iter_valid(pos));
    return &(data()[pos]);
  }

  ssize_t iter_begin() const {
    ssize_t limit = posLimit();
    ssize_t pos = 0;
    for (; pos != limit; ++pos) {
      auto* e = iter_elm(pos);
      if (!isTombstone(e)) break;
    }
    return pos;
  }

  ssize_t iter_next(ssize_t pos) const {
    ssize_t limit = posLimit();
    for (++pos; pos < limit; ++pos) {
      auto* e = iter_elm(pos);
      if (!isTombstone(e)) return pos;
    }
    return limit;
  }

  ssize_t iter_prev(ssize_t pos) const {
    ssize_t orig_pos = pos;
    while (pos > 0) {
      --pos;
      auto* e = iter_elm(pos);
      if (!isTombstone(e)) return pos;
    }
    return orig_pos;
  }

  Variant iter_key(ssize_t pos) const {
    assert(iter_valid(pos));
    auto* e = iter_elm(pos);
    if (e->hasStrKey()) {
      return Variant{e->skey};
    }
    return (int64_t)e->ikey;
  }

  const TypedValue* iter_value(ssize_t pos) const {
    assert(iter_valid(pos));
    return &iter_elm(pos)->data;
  }

  uint32_t nthElmPos(size_t n) const {
    if (LIKELY(!hasTombstones())) {
      // Fast path: HashCollection contains no tombstones
      return n;
    }
    // Slow path: AssoCollection has at least one tombstone,
    // so we need to count forward
    // TODO Task# 4281431: If n > m_size/2 we could get better
    // performance by starting at the end of the buffer and
    // walking backward.
    if (n >= m_size) {
      return posLimit();
    }
    uint32_t pos = 0;
    for (;;) {
      while (isTombstone(pos)) {
        assert(pos + 1 < posLimit());
        ++pos;
      }
      if (n <= 0) break;
      --n;
      assert(pos + 1 < posLimit());
      ++pos;
    }
    return pos;
  }

  /**
   * mutate() must be called before doing anything that mutates
   * this HashCollection's buffer, unless it can be proven that
   * canMutateBuffer() is true. mutate() takes care of updating
   * m_immCopy and making a copy of this HashCollection's buffer
   * if needed.
   */
  void mutate() {
    assert(IMPLIES(!m_immCopy.isNull(), arrayData()->hasMultipleRefs()));
    if (arrayData()->cowCheck()) {
      // mutateImpl() does two things for us. First it drops the the
      // immutable collection held by m_immCopy (if m_immCopy is not
      // null). Second, it takes care of copying the buffer if needed.
      mutateImpl();
    }
    assert(canMutateBuffer());
    assert(m_immCopy.isNull());
  }

  void mutateAndBump() { mutate(); ++m_version; }

  void dropImmCopy() {
    assert(m_immCopy.isNull() ||
           (data() == ((HashCollection*)m_immCopy.get())->data() &&
            arrayData()->hasMultipleRefs()));
    m_immCopy.reset();
  }

  TypedValue* findForUnserialize(int64_t k) {
    auto h = hashint(k);
    auto p = findForInsert(k, h);
    if (UNLIKELY(validPos(*p))) return nullptr;
    auto e = &allocElm(p);
    e->setIntKey(k, h);
    updateNextKI(k);
    return &e->data;
  }

  TypedValue* findForUnserialize(StringData* key) {
    auto h = key->hash();
    auto p = findForInsert(key, h);
    if (UNLIKELY(validPos(*p))) return nullptr;
    auto e = &allocElm(p);
    e->setStrKey(key, h);
    updateIntLikeStrKeys(key);
    return &e->data;
  }

  static bool validPos(ssize_t pos) {
    return pos >= 0;
  }

  static bool validPos(int32_t pos) {
    return pos >= 0;
  }

  ALWAYS_INLINE
  static std::array<TypedValue, 1>
  makeArgsFromHashValue(const HashCollection::Elm& e) {
    // note that this is a potentially unnecessary copy
    // that might be reinterpret_cast ed away
    // http://stackoverflow.com/questions/11205186/treat-c-cstyle-array-as-stdarray
    return std::array<TypedValue, 1> {{ e.data }};
  }

  ALWAYS_INLINE
  static std::array<TypedValue, 2>
  makeArgsFromHashKeyAndValue(const HashCollection::Elm& e) {
    return std::array<TypedValue, 2> {{
      (e.hasIntKey()
        ? make_tv<KindOfInt64>(e.ikey)
        : make_tv<KindOfString>(e.skey)),
      e.data
    }};
  }

  /**
   * canMutateBuffer() indicates whether it is currently safe to directly
   * modify this HashCollection's buffer.
   */
  bool canMutateBuffer() const {
    assert(IMPLIES(!arrayData()->cowCheck(), m_immCopy.isNull()));
    return !arrayData()->cowCheck();
  }

  static constexpr ptrdiff_t arrOffset() {
    return offsetof(HashCollection, m_arr);
  }

  bool toBoolImpl() const {
    return (m_size != 0);
  }

  template <class Hit>
  ssize_t findImpl(hash_t h0, Hit) const;
  ssize_t find(int64_t k, inthash_t h) const;
  ssize_t find(const StringData* s, strhash_t h) const;

  ssize_t findForRemove(int64_t k, inthash_t h) {
    assert(canMutateBuffer());
    return arrayData()->findForRemove(k, h, false);
  }

  ssize_t findForRemove(const StringData* s, strhash_t h) {
    assert(canMutateBuffer());
    return arrayData()->findForRemove(s, h);
  }

  template <class Hit>
  ALWAYS_INLINE
  int32_t* findForInsertImpl(hash_t h0, Hit hit) const {
    uint32_t mask = tableMask();
    auto elms = data();
    auto hashtable = hashTab();
    int32_t* ret = nullptr;
    for (uint32_t probe = h0, i = 1;; ++i) {
      auto ei = &hashtable[probe & mask];
      ssize_t pos = *ei;
      if (validPos(pos)) {
        if (hit(elms[pos])) {
          return ei;
        }
      } else {
        if (!ret) ret = ei;
        if (pos & 1) {
          assert(pos == Empty);
          return LIKELY(i <= 100) ? ret : warnUnbalanced(i, ret);
        }
      }
      probe += i;
      assertx(i <= mask);
      assertx(probe == static_cast<uint32_t>(h0) + (i + i * i) / 2);
    }
  }

  static bool hitStringKey(const HashCollection::Elm& e,
                           const StringData* s, int32_t hash) {
    // hitStringKey() should only be called on an Elm that is
    // referenced by a hash table entry. HashCollection guarantees
    // that when it adds a hash table entry that it always sets it to
    // refer to a valid element. Likewise when it removes an element
    // it always removes the corresponding hash entry.  Therefore the
    // assertion below must hold.
    assert(!HashCollection::isTombstone(&e));
    return hash == e.hash() && (s == e.skey || s->same(e.skey));
  }

  static bool hitIntKey(const HashCollection::Elm& e, int64_t ki) {
    // hitIntKey() should only be called on an Elm that is referenced
    // by a hash table entry. HashCollection guarantees that when it
    // adds a hash table entry that it always sets it to refer to a
    // valid element. Likewise when it removes an element it always
    // removes the corresponding hash entry.  Therefore the assertion
    // below must hold.
    assert(!HashCollection::isTombstone(&e));
    return e.ikey == ki && e.hasIntKey();
  }

  int32_t* findForInsert(int64_t ki, inthash_t h) const {
    return findForInsertImpl(h, [ki] (const Elm& e) {
        return hitIntKey(e, ki);
      });
  }

  int32_t* findForInsert(const StringData* s, strhash_t h) const {
    return findForInsertImpl(h, [s, h] (const Elm& e) {
        return hitStringKey(e, s, h);
      });
  }

  // findForNewInsert() is only safe to use if you know for sure that the
  // key is not already present in the HashCollection.
  ALWAYS_INLINE int32_t* findForNewInsert(
    int32_t* table, size_t mask, hash_t h0) const {
    for (uint32_t i = 1, probe = h0;; ++i) {
      auto ei = &table[probe & mask];
      if (!validPos(*ei)) {
        return ei;
      }
      probe += i;
      assertx(i <= mask);
      assertx(probe == static_cast<uint32_t>(h0) + (i + i * i) / 2);
    }
  }

  ALWAYS_INLINE int32_t* findForNewInsert(hash_t h0) const {
    return findForNewInsert(hashTab(), tableMask(), h0);
  }

  static void copyElm(const Elm& frE, Elm& toE) {
    memcpy(&toE, &frE, sizeof(Elm));
  }

  static void dupElm(const Elm& frE, Elm& toE) {
    assert(!isTombstoneType(frE.data.m_type));
    memcpy(&toE, &frE, sizeof(Elm));
    if (toE.hasStrKey()) toE.skey->incRefCount();
    tvRefcountedIncRef(&toE.data);
  }

  MixedArray* arrayData() { return m_arr; }
  const MixedArray* arrayData() const { return m_arr; }

  /**
   * Copy the buffer and reset the immutable copy.
   */
  void mutateImpl();

  Elm* data() { return m_arr->data(); }
  const Elm* data() const { return m_arr->data(); }
  int32_t* hashTab() const { return m_arr->hashTab(); }

  void setSize(uint32_t sz) {
    assert(sz <= cap());
    if (m_arr == staticEmptyMixedArray()) {
      assert(sz == 0);
      return;
    }
    assert(!arrayData()->hasMultipleRefs());
    m_size = sz;
    arrayData()->m_size = sz;
  }
  void incSize() {
    assert(m_size + 1 <= cap());
    assert(!arrayData()->hasMultipleRefs());
    ++m_size;
    arrayData()->m_size = m_size;
  }
  void decSize() {
    assert(m_size > 0);
    assert(!arrayData()->hasMultipleRefs());
    --m_size;
    arrayData()->m_size = m_size;
  }
  uint32_t scale() const {
    return arrayData()->scale();
  }
  uint32_t cap() const {
    return arrayData()->capacity();
  }
  uint32_t tableMask() const {
    return arrayData()->mask();
  }
  uint32_t posLimit() const {
    return arrayData()->m_used;
  }
  void incPosLimit() {
    assert(!arrayData()->hasMultipleRefs());
    assert(posLimit() + 1 <= cap());
    arrayData()->m_used++;
  }
  void setPosLimit(uint32_t limit) {
    auto* a = arrayData();
    if (a == staticEmptyMixedArray()) {
      assert(limit == 0);
      return;
    }
    assert(!a->hasMultipleRefs());
    assert(limit <= cap());
    a->m_used = limit;
  }
  int64_t nextKI() {
    return arrayData()->m_nextKI;
  }
  void setNextKI(int64_t ki) {
    assert(!arrayData()->hasMultipleRefs());
    arrayData()->m_nextKI = ki;
  }
  void updateNextKI(int64_t ki) {
    assert(!arrayData()->hasMultipleRefs());
    auto* a = arrayData();
    if (ki >= a->m_nextKI && a->m_nextKI >= 0) {
      a->m_nextKI = ki + 1;
    }
  }
  void updateIntLikeStrKeys(const StringData* s) {
    int64_t ignore;
    if (UNLIKELY(s->isStrictlyInteger(ignore))) {
      setIntLikeStrKeys(true);
    }
  }

  void updateIntLikeStrKeys() {
    int64_t ignore;
    if (intLikeStrKeys()) return;
    const Elm* e = data();
    const Elm* eLimit = elmLimit();
    for (; e != eLimit; ++e) {
      if (!isTombstone(e) && e->hasStrKey() &&
          e->skey->isStrictlyInteger(ignore)) {
        setIntLikeStrKeys(true);
        return;
      }
    }
  }

  // The skipTombstonesNoBoundsCheck helper functions assume that either
  // the specified location is not a tombstone OR that there is at least
  // one non-tombstone after the specified position.

  static int32_t
  skipTombstonesNoBoundsCheck(int32_t pos, int32_t posLimit, const Elm* data) {
    assert(pos < posLimit);
    while (isTombstone(pos, data)) {
      ++pos;
      assert(pos < posLimit);
    }
    return pos;
  }

  int32_t skipTombstonesNoBoundsCheck(int32_t pos) {
    return skipTombstonesNoBoundsCheck(pos, posLimit(), data());
  }

  const Elm* firstElmImpl() const {
    const Elm* e = data();
    const Elm* eLimit = elmLimit();
    for (; e != eLimit && isTombstone(e); ++e) {}
    return (Elm*)e;
  }
  Elm* firstElm() {
    return (Elm*)firstElmImpl();
  }
  const Elm* firstElm() const {
    return firstElmImpl();
  }

  Elm* elmLimit() {
    return data() + posLimit();
  }
  const Elm* elmLimit() const {
    return data() + posLimit();
  }

  static Elm* nextElm(Elm* e, Elm* eLimit) {
    assert(e != eLimit);
    for (++e; e != eLimit && isTombstone(e); ++e) {}
    return e;
  }
  static const Elm* nextElm(const Elm* e, const Elm* eLimit) {
    return (const Elm*)nextElm((Elm*)e, (Elm*)eLimit);
  }

  static bool isTombstoneType(DataType t) {
    assert(isRealType(t) || t == kInvalidDataType);
    return t < KindOfUninit;
    static_assert(KindOfUninit == 0 && kInvalidDataType < 0, "");
  }

  static bool isTombstone(const Elm* e) {
    return isTombstoneType(e->data.m_type);
  }

  static bool isTombstone(ssize_t pos, const Elm* data) {
    return isTombstoneType(data[pos].data.m_type);
  }

  bool isTombstone(ssize_t pos) const {
    assert(size_t(pos) <= posLimit());
    return isTombstone(pos, data());
  }

  bool hasTombstones() const { return m_size != posLimit(); }

  size_t hashSize() const {
    return size_t(tableMask()) + 1;
  }

  static uint32_t computeMaxElms(uint32_t tableMask) {
    return tableMask - tableMask / LoadScale;
  }

  static void initHash(int32_t* table, size_t tableSize) {
    static_assert(Empty == -1, "Cannot use wordfillones().");
    wordfillones(table, tableSize);
  }

  [[noreturn]] void throwTooLarge();
  [[noreturn]] void throwReserveTooLarge();
  int32_t* warnUnbalanced(size_t n, int32_t* ei) const;

  /**
   * Raises a warning if the set contains an int and a string with the same
   * numeric value: e.g. Set {'123', 123}. It's a no-op otherwise.
   */
  void warnOnStrIntDup() const;

  static bool instanceof(const ObjectData*);

  template<typename F>
  void scan(F& mark) const {
    mark(m_arr);
    mark(m_immCopy);
  }

 protected:

  // Replace the m_arr field with a new MixedArray. The array must be known to
  // *not* contain any references.  WARNING: does not update intLikeStrKeys
  void replaceArray(ArrayData* adata) {
    auto* oldAd = m_arr;
    m_arr = MixedArray::asMixed(adata);
    adata->incRefCount();
    m_size = adata->size();
    decRefArr(oldAd);
    ++m_version;
  }

  union {
    struct {
      uint32_t m_size;    // Number of values
      int32_t m_version;  // Version number (high bit used to indicate if this
    };                    //   collection might contain int-like string keys)
    int64_t m_versionAndSize;
  };

  MixedArray* m_arr;      // Elm store.

  // A pointer to an immutable collection that shares its buffer with
  // this collection.
  Object m_immCopy;

  // Read the high bit of m_version to tell if this collection might contain
  // int-like string keys. If this method returns false it is safe to assume
  // that no int-like strings keys are present. If this method returns true
  // that means there _might_ be int-like string keys, but there might not be.
  bool intLikeStrKeys() const { return (bool)(m_version & 0x80000000UL); }
  // So that BaseMap can call intLikeStrKeys on a HashCollection
  static bool intLikeStrKeys(const HashCollection* hc) {
    return hc->intLikeStrKeys();
  }
  // Beware: calling this method can invalidate iterators, so use with
  // caution
  void setIntLikeStrKeys(bool b) {
    if (b) {
      m_version |= 0x80000000UL;
    } else {
      m_version &= ~0x80000000UL;
    }
  }

 private:
  struct EmptyMixedInitializer;
  static EmptyMixedInitializer s_empty_mixed_initializer;

  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc, bool checkTypes);
  void postSort();


};

/////////////////////////////////////////////////////////////////////////////
}
#endif
