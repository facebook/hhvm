#ifndef incl_HPHP_COLLECTIONS_HASHCOLLECTION_H
#define incl_HPHP_COLLECTIONS_HASHCOLLECTION_H

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/tv-refcount.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE MixedArray* staticEmptyDictArrayAsMixed() {
  return static_cast<MixedArray*>(staticEmptyDictArray());
}

// Common base class for BaseMap/BaseSet collections
struct HashCollection : ObjectData {
  explicit HashCollection(Class* cls, HeaderKind kind)
    : ObjectData(cls, NoInit{}, collections::objectFlags, kind)
    , m_versionAndSize(0)
    , m_arr(staticEmptyDictArrayAsMixed())
  {}
  explicit HashCollection(Class* cls, HeaderKind kind, ArrayData* arr)
    : ObjectData(cls, NoInit{}, collections::objectFlags, kind)
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
    return find(key, hash_int64(key)) != Empty;
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
    assert(IMPLIES(
      arrayData() == staticEmptyDictArrayAsMixed(),
      !b));
    assert(IMPLIES(cap() == 0, !b));
    return b;
  }

  bool isCapacityTooHigh() const {
    // Return true if current capacity at least 8x greater than m_size AND
    // if current capacity is at least 8x greater than the minimum capacity
    bool b = ((uint64_t(cap()) >= uint64_t(m_size) * 8) &&
              (cap() >= HashCollection::SmallSize * 8));
    assert(IMPLIES(
      arrayData() == staticEmptyDictArrayAsMixed(),
      !b));
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

  HashCollection::Elm& allocElm(MixedArray::Inserter ei) {
    assert(canMutateBuffer());
    assert(MixedArray::isValidIns(ei) && !MixedArray::isValidPos(*ei)
           && m_size <= posLimit() && posLimit() < cap());
    size_t i = posLimit();
    *ei = i;
    setPosLimit(i + 1);
    incSize();
    return data()[i];
  }

  HashCollection::Elm& allocElmFront(MixedArray::Inserter ei);

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
           (arrayData() == ((HashCollection*)m_immCopy.get())->arrayData() &&
            arrayData()->hasMultipleRefs()));
    m_immCopy.reset();
  }

  TypedValue* findForUnserialize(int64_t k) {
    auto h = hash_int64(k);
    auto p = findForInsert(k, h);
    if (UNLIKELY(MixedArray::isValidPos(*p))) return nullptr;
    auto e = &allocElm(p);
    e->setIntKey(k, h);
    updateNextKI(k);
    return &e->data;
  }

  TypedValue* findForUnserialize(StringData* key) {
    auto h = key->hash();
    auto p = findForInsert(key, h);
    if (UNLIKELY(MixedArray::isValidPos(*p))) return nullptr;
    auto e = &allocElm(p);
    e->setStrKey(key, h);
    return &e->data;
  }

  /*
   * Batch insertion interface that postpones hashing, for cache efficiency.
   * Use these methods in order:
   *  1. batchInsertBegin,
   *  2. any number of batchInsert calls,
   *  3. tryBatchInsertEnd or batchInsertAbort
   * No other methods may be called between 1 and 3.
   */
  uint32_t batchInsertBegin(int64_t n) {
    reserve(m_size + n);
    return posLimit();
  }
  TypedValue* batchInsert(StringData* key) {
    auto h = key->hash();
    // Not hashing yet, so position is unused for now.
    int32_t unusedPos = -1;
    auto e = &allocElm(MixedArray::Inserter(&unusedPos));
    e->setStrKey(key, h);
    return &e->data;
  }
  /*
   * Attempts to finalize a batch insertion, given the return value from the
   * call to batchInsertBegin. Returns true on success, and aborts and returns
   * false on failure (e.g., duplicate keys).
   */
  bool tryBatchInsertEnd(uint32_t begin) {
    for (auto i = begin; i < posLimit(); ++i) {
      auto e = data()[i];
      auto h = e.hash();
      auto p = e.hasStrKey() ?
        findForInsert(e.skey, h) :
        findForInsert(e.ikey, h);
      if (UNLIKELY(MixedArray::isValidPos(*p))) {
        batchInsertAbort(begin);
        return false;
      }
      *p = i;
    }
    return true;
  }
  /*
   * Cancels a started batch insertion, given the return value from the call to
   * batchInsertBegin, reverting to the state before it began. Idempotent.
   */
  void batchInsertAbort(uint32_t begin) {
    for (auto i = posLimit(); i > begin; --i) {
      auto& e = data()[i - 1];
      auto tv = &e.data;
      auto const old = *tv;
      tv->m_type = kInvalidDataType;
      decSize();
      setPosLimit(i - 1);
      if (e.hasStrKey()) decRefStr(e.skey);
      tvDecRefGen(old);
    }
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

  ssize_t find(int64_t ki, inthash_t h) const {
    return m_arr->find(ki, h);
  }

  ssize_t find(const StringData* s, strhash_t h) const {
    return m_arr->find(s, h);
  }

  ssize_t findForRemove(int64_t k, inthash_t h) {
    assert(canMutateBuffer());
    return m_arr->findForRemove(k, h, false);
  }

  ssize_t findForRemove(const StringData* s, strhash_t h) {
    assert(canMutateBuffer());
    return m_arr->findForRemove(s, h);
  }

  MixedArray::Inserter findForInsert(int64_t ki,
                                                 inthash_t h) const {
    return m_arr->findForInsertUpdate(ki, h);
  }

  MixedArray::Inserter findForInsert(const StringData* s,
                                                 strhash_t h) const {
    return m_arr->findForInsertUpdate(s, h);
  }

  MixedArray::Inserter findForNewInsert(int32_t* table, size_t mask,
                                                    hash_t h0) const {
    return m_arr->findForNewInsert(table, mask, h0);
  }

  static void copyElm(const Elm& frE, Elm& toE) {
    memcpy(&toE, &frE, sizeof(Elm));
  }

  static void dupElm(const Elm& frE, Elm& toE) {
    assert(!isTombstoneType(frE.data.m_type));
    memcpy(&toE, &frE, sizeof(Elm));
    if (toE.hasStrKey()) toE.skey->incRefCount();
    tvIncRefGen(&toE.data);
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
    if (m_arr == staticEmptyDictArrayAsMixed()) {
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
    if (a == staticEmptyDictArrayAsMixed()) {
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

  static uint32_t computeMaxElms(uint32_t tableMask) {
    return tableMask - tableMask / LoadScale;
  }

  [[noreturn]] void throwTooLarge();
  [[noreturn]] void throwReserveTooLarge();
  int32_t* warnUnbalanced(size_t n, int32_t* ei) const;

  /**
   * Raises a warning if the set contains an int and a string with the same
   * numeric value: e.g. Set {'123', 123}. It's a no-op otherwise.
   */
  void warnOnStrIntDup() const;

  void scan(type_scan::Scanner& scanner) const {
    scanner.scan(m_arr);
    scanner.scan(m_immCopy);
  }

 protected:

  // Replace the m_arr field with a new MixedArray. The array must be known to
  // *not* contain any references.
  void replaceArray(ArrayData* adata) {
    auto* oldAd = m_arr;
    dropImmCopy();
    m_arr = MixedArray::asMixed(adata);
    adata->incRefCount();
    m_size = adata->size();
    decRefArr(oldAd);
    ++m_version;
  }

  union {
    struct {
      uint32_t m_size;    // Number of values
      int32_t m_version;  // Version number
    };
    int64_t m_versionAndSize;
  };

  MixedArray* m_arr;      // Elm store.

  // A pointer to an immutable collection that shares its buffer with
  // this collection.
  Object m_immCopy;

 private:
  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc, bool checkTypes);
  void postSort();


};

/////////////////////////////////////////////////////////////////////////////
}
#endif
