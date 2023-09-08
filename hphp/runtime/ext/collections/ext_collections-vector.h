#pragma once

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/vanilla-vec-defs.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct BaseMap;
struct BaseSet;
struct c_Pair;

namespace collections {
void append(ObjectData*, TypedValue*);
void deepCopy(tv_lval);
struct VectorIterator;
}

///////////////////////////////////////////////////////////////////////////////
//// class BaseVector: encapsulates functionality that is common to both
//// c_Vector and c_ImmVector. It doesn't map to any PHP-land class.

struct BaseVector : c_Collection {
protected:
  // Make sure this one is inlined all the way
  explicit BaseVector(Class* cls, HeaderKind kind)
    : c_Collection(cls, kind)
    , m_unusedAndSize(0)
  {
    setArrayData(ArrayData::CreateVec());
  }
  explicit BaseVector(Class* cls, HeaderKind kind, ArrayData* arr)
    : c_Collection(cls, kind)
    , m_unusedAndSize(arr->m_size)
  {
    setArrayData(arr);
  }
  explicit BaseVector(Class* cls, HeaderKind kind, uint32_t cap)
    : c_Collection(cls, kind)
    , m_unusedAndSize(0)
  {
    setArrayData(VanillaVec::MakeReserveVec(cap));
  }
  ~BaseVector();

public:
  ArrayData* arrayData() { return m_arr; }
  const ArrayData* arrayData() const { return m_arr; }
  void setArrayData(ArrayData* arr) {
    assertx(arr->isVanillaVec());
    assertx(!arr->isLegacyArray());
    m_arr = arr;
  }

  void setSize(uint32_t sz) {
    assertx(canMutateBuffer());
    assertx(sz <= VanillaVec::capacity(arrayData()));
    m_size = sz;
    arrayData()->m_size = sz;
  }
  void incSize() {
    assertx(canMutateBuffer());
    assertx(m_size < VanillaVec::capacity(arrayData()));
    ++m_size;
    arrayData()->m_size = m_size;
  }
  void decSize() {
    assertx(canMutateBuffer());
    assertx(m_size > 0);
    --m_size;
    arrayData()->m_size = m_size;
  }
  tv_lval appendForUnserialize(int64_t k) {
    assertx(k == m_size);
    incSize();
    return lvalAt(k);
  }

  template <IntishCast intishCast = IntishCast::None>
  static Array ToArray(const ObjectData* obj) {
    check_collection_cast_to_array();
    return const_cast<BaseVector*>(
      static_cast<const BaseVector*>(obj)
    )->toPHPArrayImpl();
  }

  static bool ToBool(const ObjectData* obj) {
    return static_cast<const BaseVector*>(obj)->toBoolImpl();
  }

  template <bool throwOnMiss>
  static tv_lval OffsetAt(ObjectData* obj, const TypedValue* key) {
    auto vec = static_cast<BaseVector*>(obj);
    if (key->m_type == KindOfInt64) {
      return throwOnMiss ? vec->at(key->m_data.num)
                         : vec->get(key->m_data.num);
    }
    throwBadKeyType();
    return nullptr;
  }
  static bool OffsetIsset(ObjectData* obj, const TypedValue* key);
  static bool OffsetContains(ObjectData* obj, const TypedValue* key);
  static bool Equals(const ObjectData* obj1, const ObjectData* obj2);

  template<typename T> typename
    std::enable_if<std::is_integral<T>::value, tv_lval>::type
  at(T key) {
    if (UNLIKELY((uint64_t)key >= (uint64_t)m_size)) {
      collections::throwOOB(key);
      return nullptr;
    }
    return lvalAt(key);
  }
  tv_lval at(const TypedValue* key) {
    if (LIKELY(key->m_type == KindOfInt64)) {
      return at(key->m_data.num);
    }
    throwBadKeyType();
  }

  template<typename T> typename
    std::enable_if<std::is_integral<T>::value, tv_lval>::type
  get(T key) {
    if ((uint64_t)key >= (uint64_t)m_size) {
      return nullptr;
    }
    return lvalAt(key);
  }
  tv_lval get(const TypedValue* key) {
    if (LIKELY(key->m_type == KindOfInt64)) {
      return get(key->m_data.num);
    }
    throwBadKeyType();
  }

  bool contains(int64_t key) const {
    return ((uint64_t)key < (uint64_t)m_size);
  }

  void init(const Variant& it) {
    assertx(m_size == 0);
    addAllImpl(it);
  }

  void zip(BaseVector* bvec, const Variant& iterable);
  void addFront(TypedValue v);
  Variant popFront();

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

  Array toPHPArrayImpl() {
    if (!m_size) return empty_dict_array();
    auto ad = arrayData()->toDict(true);
    return ad != arrayData() ? Array::attach(ad) : Array{ad};
  }

  static constexpr size_t sizeOffset() { return offsetof(BaseVector, m_size); }
  static constexpr size_t arrOffset() { return offsetof(BaseVector, m_arr); }

  /*
   * Append `v' to the Vector and incref it if it's refcounted.
   */
  void add(TypedValue v) { addImpl<false>(v); }
  void add(const Variant& v) { add(*v.asTypedValue()); }

  /*
   * Add `k' => `v' to the Vector, increffing `v' if it's refcounted.
   */
  void set(int64_t key, TypedValue val) {
    tvIncRefGen(val);
    setMove(key, val);
  }
  void set(int64_t key, const TypedValue* val) {
    set(key, *val);
  }
  void set(int64_t key, const Variant& val) {
    set(key, val.asTypedValue());
  }
  void set(const TypedValue* key, const TypedValue* val) {
    if (key->m_type != KindOfInt64) {
      throwBadKeyType();
    }
    set(key->m_data.num, val);
  }
  void set(const Variant& key, const Variant& val) {
    set(key.asTypedValue(), val.asTypedValue());
  }

  /*
   * Set a particular value in a vector without inc-ref-ing the value.
   */
  void setMove(int64_t key, TypedValue val) {
    mutate();
    // Cast to unsigned to check "0 <= key && key < m_size" in one comparison.
    if (UNLIKELY((uint64_t)key >= (uint64_t)m_size)) {
      collections::throwOOB(key);
    }
    tvMove(val, lvalAt(key));
  }

  /**
   * canMutateBuffer() indicates whether it is currently safe to directly
   * modify this Vector's buffer.
   */
  bool canMutateBuffer() const {
    assertx(IMPLIES(!arrayData()->hasMultipleRefs(), m_immCopy.isNull()));
    assertx(m_size == arrayData()->m_size);
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
    assertx(canMutateBuffer());
  }

  Object getImmutableCopy();
  void dropImmCopy() {
    assertx(m_immCopy.isNull() ||
           (arrayData() == ((BaseVector*)m_immCopy.get())->arrayData() &&
            !canMutateBuffer()));
    m_immCopy.reset();
  }

  [[noreturn]] static void throwBadKeyType();

  static constexpr uint64_t MaxCapacity() {
    return VanillaDict::MaxSize;
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

  // Get a (mutable or const) ref to the value. `index` must be a valid index.
  tv_lval lvalAt(int64_t index) {
    return VanillaVec::LvalUncheckedInt(arrayData(), index);
  }
  tv_rval rvalAt(int64_t index) const {
    return const_cast<BaseVector*>(this)->lvalAt(index);
  }

  // Returns the value at k, with no refcount change. Requires contains(k).
  TypedValue removeKeyImpl(int64_t k);

  void reserveImpl(uint32_t newCap);

  void addAllImpl(const Variant& t);

  template<bool raw> ALWAYS_INLINE
  void addImpl(TypedValue tv) {
    auto oldAd = arrayData();
    if (raw) {
      assertx(canMutateBuffer());
      setArrayData(VanillaVec::AppendInPlace(oldAd, tv));
      if (arrayData() != oldAd) {
        decRefArr(oldAd);
      }
    } else {
      dropImmCopy();
      setArrayData(VanillaVec::AppendMove(oldAd, tv));
      tvIncRefGen(tv);
    }
    m_size = arrayData()->m_size;
  }

  // addRaw() adds a new element to this Vector but doesn't check for an
  // immutable buffer, so it's only safe to use in some cases. If you're not
  // sure, use add() instead.
  void addRaw(TypedValue v) { addImpl<true>(v); }
  void addRaw(const Variant& v) { addRaw(*v.asTypedValue()); }

  /**
   * Copy the buffer and reset the immutable copy.
   */
  void mutateImpl();

  Object getIterator();

  /////////////////////////////////////////////////////////////////////////////

  // Fields
  union {
    struct {
      uint32_t m_size;
      int32_t m_unused;
    };
    int64_t m_unusedAndSize;
  };

  // The vec backing this collection. See setArrayData() for a list of the
  // invariants that this vec must satisfy.
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

  friend void collections::deepCopy(tv_lval);

  friend void collectionReserve(ObjectData* obj, int64_t sz);
  friend void collectionInitAppend(ObjectData* obj, TypedValue* val);
};

/////////////////////////////////////////////////////////////////////////////
// c_Vector

struct c_Vector : BaseVector, SystemLib::ClassLoader<"HH\\Vector"> {
  DECLARE_COLLECTIONS_CLASS(Vector)

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

  void clear();
  Variant pop();
  void resize(uint32_t sz, const TypedValue* val);
  void removeKey(int64_t k);
  void reverse();
  void shuffle();
  void splice(int64_t startPos, int64_t endPos);
  void splice(int64_t startPos) {
    splice(startPos, m_size - 1);
  }

  static void OffsetSet(ObjectData* obj, const TypedValue* key,
                        const TypedValue* val);
  static void OffsetUnset(ObjectData* obj, const TypedValue* key);
  static Object fromArray(const Class*, const Variant&);

  struct SortTmp {
    SortTmp(c_Vector* v, SortFunction sf) : m_v(v) {
      if (hasUserDefinedCmp(sf)) {
        m_ad = VanillaVec::Copy(m_v->arrayData());
      } else {
        m_v->mutate();
        m_ad = m_v->arrayData();
      }
    }
    ~SortTmp() {
      if (m_v->arrayData() != m_ad) {
        Array tmp = Array::attach(m_v->arrayData());
        m_v->setArrayData(m_ad);
      }
    }
    ArrayData* operator->() { return m_ad; }
   private:
    c_Vector* m_v;
    ArrayData* m_ad;
  };

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

  void php_removeKey(const Variant& key) {
    if (UNLIKELY(!key.isInteger())) {
      throwBadKeyType();
    }
    removeKey(key.toInt64());
  }

  void php_reserve(const Variant& sz) {
    return reserve(checkRequestedSize(sz));
  }
  void php_resize(const Variant& sz, const Variant& value) {
    return resize(checkRequestedSize(sz), value.asTypedValue());
  }
  void php_splice(const Variant& offset,
                  const Variant& len = uninit_variant,
                  const Variant& replacement = uninit_variant);

private:
  // Friends
  friend struct collections::CollectionsExtension;
  friend void collections::append(ObjectData* obj, TypedValue* val);

  friend struct BaseMap;
  friend struct c_Pair;
};

/////////////////////////////////////////////////////////////////////////////
// c_ImmVector

struct c_ImmVector : BaseVector, SystemLib::ClassLoader<"HH\\ImmVector"> {
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

struct VectorIterator : SystemLib::ClassLoader<"VectorIterator"> {
  VectorIterator() {}
  VectorIterator(const VectorIterator& src) = delete;
  VectorIterator& operator=(const VectorIterator& src) {
    m_obj = src.m_obj;
    m_pos = src.m_pos;
    return *this;
  }
  ~VectorIterator() {}

  static Object newInstance() {
    return Object{classof()};
  }

  void setVector(BaseVector* vec) {
    m_obj = vec;
    m_pos = 0;
  }

  Variant current() const {
    auto vec = m_obj.get();
    if (m_pos >= vec->m_size) {
      throw_iterator_not_valid();
    }
    const auto tv = *vec->rvalAt(m_pos);
    return Variant(tvAsCVarRef(&tv));
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
};

/////////////////////////////////////////////////////////////////////////////
}}
