#ifndef incl_HPHP_EXT_COLLECTIONS_VECTOR_H
#define incl_HPHP_EXT_COLLECTIONS_VECTOR_H

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct BaseMap;
struct BaseSet;
struct c_Pair;
struct c_AwaitAllWaitHandle;

namespace collections {
void append(ObjectData*, TypedValue*);
void deepCopy(TypedValue*);
struct VectorIterator;
}

///////////////////////////////////////////////////////////////////////////////
//// class BaseVector: encapsulates functionality that is common to both
//// c_Vector and c_ImmVector. It doesn't map to any PHP-land class.

struct BaseVector : ObjectData {
protected:
  // Make sure this one is inlined all the way
  explicit BaseVector(Class* cls, HeaderKind kind)
    : ObjectData(cls, NoInit{}, collections::objectFlags, kind)
    , m_versionAndSize(0)
    , m_arr(staticEmptyVecArray())
  {}
  explicit BaseVector(Class* cls, HeaderKind kind, ArrayData* arr)
    : ObjectData(cls, NoInit{}, collections::objectFlags, kind)
    , m_versionAndSize(arr->m_size)
    , m_arr(arr)
  {
    assertx(arr->isVecArray());
  }
  explicit BaseVector(Class* cls, HeaderKind kind, uint32_t cap)
    : ObjectData(cls, NoInit{}, collections::objectFlags, kind)
    , m_versionAndSize(0)
    , m_arr(PackedArray::MakeReserveVec(cap))
  {}
  ~BaseVector();

public:
  Variant firstValue() const {
    if (!m_size) return init_null_variant;
    return tvAsCVarRef(&data()[0]);
  }

  Variant lastValue() const {
    if (!m_size) return init_null_variant;
    return tvAsCVarRef(&data()[m_size-1]);
  }

  template<class TVector, bool useKey>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_map(const Variant& callback);

  template<class TVector, bool useKey>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_filter(const Variant& callback);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_take(const Variant& n);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_takeWhile(const Variant& fn);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_skip(const Variant& n);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_skipWhile(const Variant& fn);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_slice(const Variant& start, const Variant& len);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_concat(const Variant& iterable);

  ArrayData* arrayData() {
    assert(m_arr->isVecArray());
    return m_arr;
  }
  const ArrayData* arrayData() const {
    assert(m_arr->isVecArray());
    return m_arr;
  }
  void setSize(uint32_t sz) {
    assert(canMutateBuffer());
    assert(sz <= PackedArray::capacity(arrayData()));
    m_size = sz;
    arrayData()->m_size = sz;
  }
  void incSize() {
    assert(canMutateBuffer());
    assert(m_size < PackedArray::capacity(arrayData()));
    ++m_size;
    arrayData()->m_size = m_size;
  }
  void decSize() {
    assert(canMutateBuffer());
    assert(m_size > 0);
    --m_size;
    arrayData()->m_size = m_size;
  }
  TypedValue* appendForUnserialize(int64_t k) {
    assert(k == m_size);
    incSize();
    return &data()[k];
  }

  static Array ToArray(const ObjectData* obj) {
    check_collection_cast_to_array();
    return const_cast<BaseVector*>(
      static_cast<const BaseVector*>(obj)
    )->toArray();
  }

  static bool ToBool(const ObjectData* obj) {
    return static_cast<const BaseVector*>(obj)->toBoolImpl();
  }

  template <bool throwOnMiss>
  static TypedValue* OffsetAt(ObjectData* obj, const TypedValue* key) {
    assertx(key->m_type != KindOfRef);
    auto vec = static_cast<BaseVector*>(obj);
    if (key->m_type == KindOfInt64) {
      return throwOnMiss ? vec->at(key->m_data.num)
                         : vec->get(key->m_data.num);
    }
    throwBadKeyType();
    return nullptr;
  }
  static bool OffsetIsset(ObjectData* obj, const TypedValue* key);
  static bool OffsetEmpty(ObjectData* obj, const TypedValue* key);
  static bool OffsetContains(ObjectData* obj, const TypedValue* key);
  static bool Equals(const ObjectData* obj1, const ObjectData* obj2);

  template<typename T> typename
    std::enable_if<std::is_integral<T>::value, TypedValue*>::type
  at(T key) {
    if (UNLIKELY((uint64_t)key >= (uint64_t)m_size)) {
      collections::throwOOB(key);
      return nullptr;
    }
    return &data()[key];
  }
  TypedValue* at(const TypedValue* key) {
    assertx(key->m_type != KindOfRef);
    if (LIKELY(key->m_type == KindOfInt64)) {
      return at(key->m_data.num);
    }
    throwBadKeyType();
  }

  template<typename T> typename
    std::enable_if<std::is_integral<T>::value, TypedValue*>::type
  get(T key) {
    if ((uint64_t)key >= (uint64_t)m_size) {
      return nullptr;
    }
    return &data()[key];
  }
  TypedValue* get(const TypedValue* key) {
    assert(key->m_type != KindOfRef);
    if (LIKELY(key->m_type == KindOfInt64)) {
      return get(key->m_data.num);
    }
    throwBadKeyType();
  }

  bool contains(int64_t key) const {
    return ((uint64_t)key < (uint64_t)m_size);
  }

  void init(const Variant& it) {
    assert(m_size == 0);
    addAllImpl(it);
  }

  int64_t linearSearch(const Variant& search_value);
  void zip(BaseVector* bvec, const Variant& iterable);
  void addFront(TypedValue v);
  Variant popFront();

  int getVersion() const { return m_version; }
  int64_t size() const { return m_size; }
  bool toBoolImpl() const { return (m_size != 0); }
  /**
   * mutate() or reserve() must be called before any doing anything that mutates
   * this Vector's buffer, unless it can be proven that canMutateBuffer() is
   * true. reserve() takes care of dropping m_immCopy and making a copy of
   * this Vector's buffer if needed, and ensures that there is room for at least
   * sz items in the Vector.
   */
  void reserve(uint32_t sz);
  Array toArray() {
    if (!m_size) return empty_array();
    return Array::attach(const_cast<ArrayData*>(arrayData()->toPHPArray(true)));
  }

  static constexpr size_t sizeOffset() { return offsetof(BaseVector, m_size); }
  static constexpr size_t arrOffset() { return offsetof(BaseVector, m_arr); }

  /*
   * Append `v' to the Vector and incref it if it's refcounted.
   */
  void add(TypedValue v) { addImpl<false>(v); }
  void add(const Variant& v) { add(*v.asCell()); }

  /*
   * Add `k' => `v' to the Vector, increffing `v' if it's refcounted.
   */
  void set(int64_t key, const TypedValue* val) {
    mutate();
    setRaw(key, val);
  }
  void set(int64_t key, const Variant& val) {
    set(key, val.asCell());
  }
  void set(const TypedValue* key, const TypedValue* val) {
    assert(key->m_type != KindOfRef);
    if (key->m_type != KindOfInt64) {
      throwBadKeyType();
    }
    set(key->m_data.num, val);
  }
  void set(const Variant& key, const Variant& val) {
    set(key.asCell(), val.asCell());
  }

  template<class TVector>
  static typename
    std::enable_if<std::is_base_of<BaseVector, TVector>::value, Object>::type
  fromKeysOf(const TypedValue& container);

  /**
   * canMutateBuffer() indicates whether it is currently safe to directly
   * modify this Vector's buffer.
   */
  bool canMutateBuffer() const {
    assert(IMPLIES(!arrayData()->hasMultipleRefs(), m_immCopy.isNull()));
    assert(m_size == arrayData()->m_size);
    return !arrayData()->cowCheck();
  }

  /**
   * mutate() or reserve() must be called before any doing anything that mutates
   * this Vector's buffer, unless it can be proven that canMutateBuffer() is
   * true. mutate() takes care of dropping m_immCopy and making a copy of
   * this Vector's buffer if needed.
   */
  void mutate() {
    if (!canMutateBuffer()) {
      dropImmCopy();
      if (!canMutateBuffer()) {
        mutateImpl();
      }
    }
    assert(canMutateBuffer());
  }

  void mutateAndBump() { mutate(); ++m_version; }

  Object getImmutableCopy();
  void dropImmCopy() {
    assert(m_immCopy.isNull() ||
           (arrayData() == ((BaseVector*)m_immCopy.get())->arrayData() &&
            !canMutateBuffer()));
    m_immCopy.reset();
  }

  [[noreturn]] static void throwBadKeyType();

  static constexpr uint64_t MaxCapacity() {
    return MixedArray::MaxSize;
  }

  void scan(type_scan::Scanner& scanner) const {
    scanner.scan(m_arr);
    scanner.scan(m_immCopy);
  }

protected:
  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, TVector*>::type
  static Clone(ObjectData* obj);

  Cell* data() const { return packedData(m_arr); }
  void reserveImpl(uint32_t newCap);

  void addAllImpl(const Variant& t);

  template<bool raw> ALWAYS_INLINE
  void addImpl(TypedValue tv) {
    assert(tv.m_type != KindOfRef);
    auto oldAd = arrayData();
    if (raw) {
      assert(canMutateBuffer());
      m_arr = PackedArray::AppendVec(oldAd, tv, false);
    } else {
      ++m_version;
      dropImmCopy();
      m_arr = PackedArray::AppendVec(oldAd, tv, oldAd->cowCheck());
    }
    if (m_arr != oldAd) {
      decRefArr(oldAd);
    }
    m_size = arrayData()->m_size;
  }

  // addRaw() adds a new element to this Vector but doesn't check for an
  // immutable buffer and doesn't increment m_version, so it's only safe
  // to use in some cases. If you're not sure, use add() instead.
  void addRaw(TypedValue v) { addImpl<true>(v); }
  void addRaw(const Variant& v) { addRaw(*v.asCell()); }

  // setRaw() assigns a value to the specified key in this Vector but
  // doesn't increment m_version, so it's only safe to use in some cases.
  // If you're not sure, use set() instead.
  void setRaw(int64_t key, const TypedValue* val) {
    assert(val->m_type != KindOfRef);
    assert(canMutateBuffer());
    if (UNLIKELY((uint64_t)key >= (uint64_t)m_size)) {
      collections::throwOOB(key);
      return;
    }
    TypedValue* tv = &data()[key];
    auto const oldTV = *tv;
    cellDup(*val, *tv);
    tvDecRefGen(oldTV);
  }
  void setRaw(int64_t key, const Variant& val) {
    setRaw(key, val.asCell());
  }
  void setRaw(const TypedValue* key, const TypedValue* val) {
    assert(key->m_type != KindOfRef);
    if (key->m_type != KindOfInt64) {
      throwBadKeyType();
    }
    setRaw(key->m_data.num, val);
  }
  void setRaw(const Variant& key, const Variant& val) {
    setRaw(key.asCell(), val.asCell());
  }

  /**
   * Copy the buffer and reset the immutable copy.
   */
  void mutateImpl();

  Object getIterator();
  Variant php_at(const Variant& key) {
    return tvAsCVarRef(at(key.asCell()));
  }
  Variant php_get(const Variant& key) {
    if (auto tv = get(key.asCell())) {
      return tvAsCVarRef(tv);
    }
    return init_null_variant;
  }

  template<class TVector> typename
    std::enable_if<std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_keys();

  template<class TVector> typename
    std::enable_if<std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_zip(const Variant& it);

  /////////////////////////////////////////////////////////////////////////////

  // Fields
  union {
    struct {
      uint32_t m_size;
      int32_t m_version;
    };
    int64_t m_versionAndSize;
  };


  // The ArrayData's element area can be computed from m_arr via the
  // packedData() helper function. When capacity is non-zero, m_arr points
  // to a VecArray.
  ArrayData* m_arr;

  // m_immCopy is a smart pointer to an ImmVector that is an up-to-date
  // shallow copy of this Vector (or m_immCopy is null). We maintain the
  // invariant that a Vector and its m_immCopy share the same ArrayData
  // buffer. Neither the Vector or the ImmVector "owns" the ArrayData
  // buffer; instead we rely on the ArrayData's ref counting to deal
  // with freeing the buffer at the right time.
  Object m_immCopy;

private:
  static void compileTimeAssertions() {
    // For performance, all native collection classes have their m_size field
    // at the same offset.
    static_assert(
      offsetof(BaseVector, m_size) == collections::FAST_SIZE_OFFSET, "");
  }

  // Friends

  friend struct collections::VectorIterator;
  friend struct collections::CollectionsExtension;
  friend struct BaseMap;
  friend struct BaseSet;
  friend struct c_Pair;
  friend struct c_AwaitAllWaitHandle;

  friend void collections::deepCopy(TypedValue*);

  friend void collectionReserve(ObjectData* obj, int64_t sz);
  friend void collectionInitAppend(ObjectData* obj, TypedValue* val);
};

/////////////////////////////////////////////////////////////////////////////
// c_Vector

struct c_Vector : BaseVector {
  DECLARE_COLLECTIONS_CLASS(Vector);

  explicit c_Vector()
    : BaseVector(c_Vector::classof(), HeaderKind::Vector) { }
  explicit c_Vector(ArrayData* arr)
    : BaseVector(c_Vector::classof(), HeaderKind::Vector, arr) { }
  explicit c_Vector(uint32_t cap)
    : BaseVector(c_Vector::classof(), HeaderKind::Vector, cap) { }

  void addAll(const Variant& it) {
    addAllImpl(it);
  }

  static c_Vector* Clone(ObjectData* obj);

  void addAllKeysOf(const Variant& val);
  void clear();
  Variant pop();
  void resize(uint32_t sz, const Cell* val);
  void removeKey(int64_t k);
  void reverse();
  void shuffle();
  void splice(int64_t startPos, int64_t endPos);
  void splice(int64_t startPos) {
    splice(startPos, m_size - 1);
  }
  void sort(int sort_flags, bool ascending);
  bool usort(const Variant& cmp_function);

  static void OffsetSet(ObjectData* obj, const TypedValue* key,
                        const TypedValue* val);
  static void OffsetUnset(ObjectData* obj, const TypedValue* key);
  static Object fromArray(const Class*, const Variant&);

 protected:
  uint32_t checkRequestedSize(const Variant& sz) {
    int64_t intSz = sz.isInteger() ? sz.toInt64() : -1;
    if (intSz < 0) {
      SystemLib::throwInvalidArgumentExceptionObject(
        "Parameter sz must be a non-negative integer");
    }
    if (intSz > BaseVector::MaxCapacity()) {
      auto msg = folly::format(
        "Parameter sz must be at most {}; {} passed",
        BaseVector::MaxCapacity(),
        intSz
      ).str();
      SystemLib::throwInvalidArgumentExceptionObject(msg);
    }
    return intSz;
  }

  Object php_add(const Variant& value) {
    add(value);
    return Object{this};
  }
  Object php_addAll(const Variant& it) {
    addAll(it);
    return Object{this};
  }
  Object php_addAllKeysOf(const Variant& it) {
    addAllKeysOf(it);
    return Object{this};
  }
  Object php_clear() {
    clear();
    return Object{this};
  }
  Object php_removeKey(const Variant& key) {
    if (UNLIKELY(!key.isInteger())) {
      throwBadKeyType();
    }
    removeKey(key.toInt64());
    return Object{this};
  }
  void php_reserve(const Variant& sz) {
    return reserve(checkRequestedSize(sz));
  }
  void php_resize(const Variant& sz, const Variant& value) {
    return resize(checkRequestedSize(sz), value.asCell());
  }
  Object php_set(const Variant& key, const Variant& value) {
    set(key, value);
    return Object{this};
  }
  Object php_setAll(const Variant& it) {
    if (it.isNull()) return Object{this};
    size_t sz;
    ArrayIter iter = getArrayIterHelper(it, sz);
    for (; iter; ++iter) {
      set(iter.first(), iter.second());
    }
    return Object{this};
  }
  void php_splice(const Variant& offset,
                  const Variant& len = uninit_variant,
                  const Variant& replacement = uninit_variant);

private:
  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc);

  // Friends
  friend struct collections::CollectionsExtension;
  friend void collections::append(ObjectData* obj, TypedValue* val);
  friend void triggerCow(c_Vector* vec);

  friend struct ArrayIter;
  friend struct BaseMap;
  friend struct c_Pair;
};

/////////////////////////////////////////////////////////////////////////////
// c_ImmVector

struct c_ImmVector : BaseVector {
  DECLARE_COLLECTIONS_CLASS(ImmVector)

  explicit c_ImmVector()
    : BaseVector(c_ImmVector::classof(), HeaderKind::ImmVector) { }
  explicit c_ImmVector(ArrayData* arr)
    : BaseVector(c_ImmVector::classof(), HeaderKind::ImmVector, arr) { }
  explicit c_ImmVector(uint32_t cap)
    : BaseVector(c_ImmVector::classof(), HeaderKind::ImmVector, cap) { }

  static c_ImmVector* Clone(ObjectData* obj);
};

namespace collections {
/////////////////////////////////////////////////////////////////////////////
// VectorIterator

extern const StaticString
  s_VectorIterator;

struct VectorIterator {
  VectorIterator() {}
  VectorIterator(const VectorIterator& src) = delete;
  VectorIterator& operator=(const VectorIterator& src) {
    m_obj = src.m_obj;
    m_pos = src.m_pos;
    m_version = src.m_version;
    return *this;
  }
  ~VectorIterator() {}

  static Object newInstance() {
    static Class* cls = Unit::lookupClass(s_VectorIterator.get());
    assertx(cls);
    return Object{cls};
  }

  void setVector(BaseVector* vec) {
    m_obj = vec;
    m_pos = 0;
    m_version = vec->getVersion();
  }

  Variant current() const {
    auto vec = m_obj.get();
    if (UNLIKELY(m_version != vec->getVersion())) {
      throw_collection_modified();
    }
    if (m_pos >= vec->m_size) {
      throw_iterator_not_valid();
    }
    return tvAsCVarRef(&vec->data()[m_pos]);
  }

  int64_t key() const {
    auto vec = m_obj.get();
    if (m_pos >= vec->m_size) {
      throw_iterator_not_valid();
    }
    return m_pos;
  }

  bool valid() const {
    auto vec = m_obj.get();
    return vec && (m_pos < vec->m_size);
  }

  void next()   { ++m_pos;   }
  void rewind() { m_pos = 0; }

 private:
  req::ptr<BaseVector> m_obj;
  uint32_t m_pos{0};
  int32_t  m_version{0};
};

/////////////////////////////////////////////////////////////////////////////
}}
#endif
