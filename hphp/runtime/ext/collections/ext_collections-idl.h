/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_COLLECTION_H_
#define incl_HPHP_EXT_COLLECTION_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/system/systemlib.h"

#include <limits>

#define DECLARE_COLLECTION_MAGIC_METHODS()           \
  Variant t___get(Variant name);                     \
  Variant t___set(Variant name, Variant value);      \
  bool t___isset(Variant name);                      \
  Variant t___unset(Variant name)

#define DECLARE_KEYEDITERABLE_MATERIALIZE_METHODS()  \
  Object t_tovector();                               \
  Object t_toimmvector();                            \
  Object t_tomap();                                  \
  Object t_toimmmap();                               \
  Object t_toset();                                  \
  Object t_toimmset()

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace collections{
void deepCopy(TypedValue*);
class PairIterator;
class VectorIterator;
class MapIterator;
class SetIterator;
}

/*
 * All native collection class have their m_size field at the same
 * offset in the object.
 */
constexpr ptrdiff_t FAST_COLLECTION_SIZE_OFFSET = use_lowptr ? 16 : 24;
inline size_t getCollectionSize(const ObjectData* od) {
  assert(od->isCollection());
  return *reinterpret_cast<const uint32_t*>(
    reinterpret_cast<const char*>(od) + FAST_COLLECTION_SIZE_OFFSET
  );
}

/**
 * Called by the JIT on an emitVectorSet().
 */
class c_Vector;
void triggerCow(c_Vector* vec);
ArrayIter getArrayIterHelper(const Variant& v, size_t& sz);

using ExtCollectionObjectData = ExtObjectDataFlags<
  ObjectData::IsCppBuiltin |
  ObjectData::IsCollection |
  ObjectData::UseGet |
  ObjectData::UseSet |
  ObjectData::UseIsset |
  ObjectData::UseUnset |
  ObjectData::CallToImpl | // not used for the always-truthy c_Pair
  ObjectData::NoDestructor |
  ObjectData::HasClone>;

void throwOOB(int64_t key) ATTRIBUTE_NORETURN;

///////////////////////////////////////////////////////////////////////////////
// class BaseVector: encapsulates functionality that is common to both
// c_Vector and c_ImmVector. It doesn't map to any PHP-land class.

class BaseVector : public ExtCollectionObjectData {
 public:
  void t___construct(const Variant& iterable = null_variant);

  // ConstCollection
  bool t_isempty();
  int64_t t_count();
  Object t_items();

  // ConstIndexAccess
  Variant t_at(const Variant& key);
  Variant t_get(const Variant& key);
  bool t_containskey(const Variant& key);

  Object t_getiterator();

 protected:
  // KeyedIterable
  template<class TVector, class MakeArgs>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_map(const Variant& callback, MakeArgs) const;

  template<class TVector, class MakeArgs>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_filter(const Variant& callback, MakeArgs) const;

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_take(const Variant& n);

  template<class TVector, bool checkVersion>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_takeWhile(const Variant& fn);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_skip(const Variant& n);

  template<class TVector, bool checkVersion>
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

 public:
  Variant t_firstvalue();
  Variant t_firstkey();
  Variant t_lastvalue();
  Variant t_lastkey();

 protected:
  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  static php_fromItems(const Variant& iterable);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  static php_fromKeysOf(const Variant& container);

  void zip(BaseVector* bvec, const Variant& iterable);
  void keys(BaseVector* bvec);

  // Others
 public:
  Object t_lazy();
  Array t_toarray();
  Array t_tokeysarray();
  Array t_tovaluesarray();
  int64_t t_linearsearch(const Variant& search_value);
  Object t_tomap();
  Object t_toimmmap();
  Object t_toset();
  Object t_toimmset();

  DECLARE_COLLECTION_MAGIC_METHODS();

 public:
  ArrayData* arrayData() {
    auto* ret = getArrayFromPackedData(m_data);
    assert(ret == staticEmptyArray() || ret->isPacked());
    return ret;
  }
  const ArrayData* arrayData() const {
    auto* ret = getArrayFromPackedData(m_data);
    assert(ret == staticEmptyArray() || ret->isPacked());
    return ret;
  }
  void setSize(uint32_t sz) {
    assert(sz <= m_capacity);
    if (sz == m_size) return;
    assert(!arrayData()->hasMultipleRefs());
    m_size = sz;
    arrayData()->m_size = sz;
  }
  void incSize() {
    assert(m_size + 1 <= m_capacity);
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

 protected:
  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, TVector*>::type
  static Clone(ObjectData* obj);

 public:

  static Array ToArray(const ObjectData* obj) {
    check_collection_cast_to_array();
    return const_cast<BaseVector*>(
      static_cast<const BaseVector*>(obj)
    )->t_toarray();
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

  void init(const Variant& t);

  TypedValue* at(int64_t key) {
    if (UNLIKELY((uint64_t)key >= (uint64_t)m_size)) {
      throwOOB(key);
      return nullptr;
    }
    return &m_data[key];
  }
  TypedValue* at(const TypedValue* key) {
    assert(key->m_type != KindOfRef);
    if (LIKELY(key->m_type == KindOfInt64)) {
      return at(key->m_data.num);
    }
    throwBadKeyType();
  }

  TypedValue* get(int64_t key) {
    if ((uint64_t)key >= (uint64_t)m_size) {
      return nullptr;
    }
    return &m_data[key];
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

  int getVersion() const {
    return m_version;
  }

  int64_t size() const {
    return m_size;
  }

  bool toBoolImpl() const {
    return (m_size != 0);
  }

  void reserve(int64_t sz);

  static constexpr size_t sizeOffset() { return offsetof(BaseVector, m_size); }
  static constexpr size_t dataOffset() { return offsetof(BaseVector, m_data); }
  static constexpr size_t immCopyOffset() {
    return offsetof(BaseVector, m_immCopy);
  }

  void addFront(const TypedValue* val);

  Variant popFront();

 protected:
  // Make sure this one is inlined all the way
  explicit BaseVector(Class* cls, HeaderKind kind)
    : ExtCollectionObjectData(cls, kind)
    , m_size(0)
    , m_versionAndCap(0)
    , m_data(packedData(staticEmptyArray()))
  {}
  explicit BaseVector(Class* cls, HeaderKind kind, ArrayData* arr)
    : ExtCollectionObjectData(cls, kind)
    , m_size(arr->size())
    , m_versionAndCap(arr->cap())
    , m_data(packedData(arr))
  {
    assertx(arr == staticEmptyArray() || arr->isPacked());
  }
  explicit BaseVector(Class* cls, HeaderKind, uint32_t cap);

  ~BaseVector();

  Cell* data() const { return m_data; }
  void grow();
  void reserveImpl(uint32_t newCap);

  static constexpr uint64_t MaxCapacity() {
    // same as mixed-array for now
    return MixedArray::MaxSize;
  }

  template <bool raw>
  ALWAYS_INLINE
  void addImpl(const TypedValue* val) {
    assert(val->m_type != KindOfRef);
    if (m_capacity <= m_size) {
      grow();
    }
    if (!raw) {
      mutateAndBump();
    }
    assert(canMutateBuffer());
    cellDup(*val, m_data[m_size]);
    incSize();
  }

  // addRaw() adds a new element to this Vector but doesn't check for an
  // immutable buffer and doesn't increment m_version, so it's only safe
  // to use in some cases. If you're not sure, use add() instead.
  void addRaw(const TypedValue* val) { addImpl<true>(val); }
  void addRaw(const Variant& val) { addRaw(val.asCell()); }

 public:
  void add(const TypedValue* val) { addImpl<false>(val); }
  void add(const Variant& val) { add(val.asCell()); }

 protected:
  // setRaw() assigns a value to the specified key in this Vector but
  // doesn't increment m_version, so it's only safe to use in some cases.
  // If you're not sure, use set() instead.
  void setRaw(int64_t key, const TypedValue* val) {
    assert(val->m_type != KindOfRef);
    assert(canMutateBuffer());
    if (UNLIKELY((uint64_t)key >= (uint64_t)m_size)) {
      throwOOB(key);
      return;
    }
    TypedValue* tv = &m_data[key];
    DataType oldType = tv->m_type;
    uint64_t oldDatum = tv->m_data.num;
    cellDup(*val, *tv);
    if (IS_REFCOUNTED_TYPE(oldType)) {
      tvDecRefHelper(oldType, oldDatum);
    }
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

 public:
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

  /**
   * canMutateBuffer() indicates whether it is currently safe to directly
   * modify this Vector's buffer. canMutateBuffer() is vacuously true for
   * buffers with zero capacity (i.e. the staticEmptyArray() case) because
   * you can't meaningfully mutate zero-capacity buffer without first doing
   * a grow. This may seem weird, but its actually much smoother in practice
   * than the alternative of returning false for such cases.
   */
  bool canMutateBuffer() const {
    assert(IMPLIES(!arrayData()->hasMultipleRefs(), m_immCopy.isNull()));
    return m_capacity == 0 || !arrayData()->hasMultipleRefs();
  }

  /**
   * mutate() must be called before any doing anything that mutates this
   * Vector's buffer, unless it can be proven that canMutateBuffer() is
   * true. mutate() takes care of updating m_immCopy and making a copy
   * this Vector's buffer if needed.
   */
  void mutate() {
    if (arrayData()->hasMultipleRefs()) {
      // mutateImpl() does two things for us. First it drops the the
      // immutable collection held by m_immCopy (if m_immCopy is not
      // null). Second, it takes care of copying the buffer if needed.
      mutateImpl();
    }
  }

  void mutateAndBump() { mutate(); ++m_version; }

  void dropImmCopy() {
    assert(m_immCopy.isNull() ||
           (m_data == ((BaseVector*)m_immCopy.get())->m_data &&
            arrayData()->hasMultipleRefs()));
    m_immCopy.reset();
  }

  static void Unserialize(ObjectData* obj,
                          VariableUnserializer* uns, int64_t sz, char type);

 protected:
  /**
   * Copy the buffer and reset the immutable copy.
   */
  void mutateImpl();

  static void throwBadKeyType() ATTRIBUTE_NORETURN;

  // Fields
#ifndef USE_LOWPTR
  // Keep `m_size' aligned at the same offset for all collection classes.
  UNUSED uint32_t dummy;
#endif
  uint32_t m_size;

  union {
    struct {
      uint32_t m_capacity;
      int32_t m_version;
    };
    int64_t m_versionAndCap;
  };


  // m_data is an interior pointer into an ArrayData as computed by the
  // packedData() helper function. The ArrayData's address can be computed
  // from m_data via the getArrayFromPackedData() helper function. When
  // capacity is non-zero, m_data points to the beginning of the element
  // store within a packed array.
  TypedValue* m_data;

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
      offsetof(BaseVector, m_size) == FAST_COLLECTION_SIZE_OFFSET, "");
  }

  // Friends

  friend class collections::VectorIterator;
  friend class BaseMap;
  friend class BaseSet;
  friend class c_Pair;
  friend class c_AwaitAllWaitHandle;

  friend void collections::deepCopy(TypedValue*);

  friend void collectionReserve(ObjectData* obj, int64_t sz);
  friend void collectionInitAppend(ObjectData* obj, TypedValue* val);

  template <typename F> friend void scan(const BaseVector& this_, F& mark);
};

///////////////////////////////////////////////////////////////////////////////
// class Vector

class c_Vector : public BaseVector {
 public:
  DECLARE_CLASS_NO_SWEEP(Vector)

 public:
  explicit c_Vector(Class* cls = c_Vector::classof())
    : BaseVector(cls, HeaderKind::Vector) { }
  explicit c_Vector(Class* cls, ArrayData* arr)
    : BaseVector(cls, HeaderKind::Vector, arr) { }
  explicit c_Vector(Class* cls, uint32_t cap)
    : BaseVector(cls, HeaderKind::Vector, cap) { }
  explicit c_Vector(uint32_t cap, Class* cls = c_Vector::classof())
    : c_Vector(cls, cap) { }

  static c_Vector* Clone(ObjectData* obj);

  Object t_add(const Variant& val);
  Object t_addall(const Variant& val);
  Object t_addallkeysof(const Variant& val);
  Object t_append(const Variant& val); // deprecated
  Variant t_pop();
  void t_resize(const Variant& sz, const Variant& value);
  void t_reserve(const Variant& sz);
  Object t_clear();
  Object t_keys();
  Object t_values();
  Object t_set(const Variant& key, const Variant& value);
  Object t_setall(const Variant& iterable);
  bool t_contains(const Variant& key); // deprecated
  Object t_removekey(const Variant& key);
  void t_reverse();
  void t_splice(const Variant& offset, const Variant& len = uninit_null(),
                const Variant& replacement = uninit_null());
  void t_shuffle();
  Object t_map(const Variant& callback);
  Object t_mapwithkey(const Variant& callback);
  Object t_filter(const Variant& callback);
  Object t_filterwithkey(const Variant& callback);
  Object t_zip(const Variant& iterable);
  Object t_take(const Variant& n);
  Object t_takewhile(const Variant& fn);
  Object t_skip(const Variant& n);
  Object t_skipwhile(const Variant& fn);
  Object t_slice(const Variant& start, const Variant& len);
  Object t_concat(const Variant& iterable);
  static Object ti_fromitems(const Variant& iterable);
  static Object ti_fromkeysof(const Variant& container);
  static Object ti_fromarray(const Variant& arr); // deprecated
  Object t_tovector();
  Object t_toimmvector();
  Object t_immutable();
  String t___tostring();

  static void throwOOB(int64_t key) ATTRIBUTE_NORETURN;

  void sort(int sort_flags, bool ascending);
  bool usort(const Variant& cmp_function);

  static void OffsetSet(ObjectData* obj, const TypedValue* key,
                        const TypedValue* val);
  static void OffsetUnset(ObjectData* obj, const TypedValue* key);

 private:
  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc);

 protected:
  Object getImmutableCopy();

 private:
  int64_t checkRequestedCapacity(const Variant& sz);

  // Friends
  friend void collections::append(ObjectData* obj, TypedValue* val);
  friend void triggerCow(c_Vector* vec);

  friend class BaseMap;
  friend class c_Pair;
  friend class ArrayIter;
};

///////////////////////////////////////////////////////////////////////////////
// class ImmVector

class c_ImmVector : public BaseVector {
 public:
  DECLARE_CLASS_NO_SWEEP(ImmVector)

 public:
  // The methods that implement the ConstVector interface simply forward
  // invocations to the implementations in BaseVector. Unfortunately, we need
  // to explicitly declare them so that the code automatically generated from
  // the IDL can link against them.

  // KeyedIterable
  Object t_map(const Variant& callback);
  Object t_mapwithkey(const Variant& callback);
  Object t_filter(const Variant& callback);
  Object t_filterwithkey(const Variant& callback);
  Object t_zip(const Variant& iterable);
  Object t_take(const Variant& n);
  Object t_takewhile(const Variant& fn);
  Object t_skip(const Variant& n);
  Object t_skipwhile(const Variant& fn);
  Object t_slice(const Variant& start, const Variant& len);
  Object t_concat(const Variant& iterable);
  Object t_keys();

  // Others
  Object t_values();

  Object t_tovector();
  Object t_toimmvector();
  Object t_immutable();
  String t___tostring();

  static Object ti_fromitems(const Variant& iterable);
  static Object ti_fromkeysof(const Variant& container);

 public:
  explicit c_ImmVector(Class* cls = c_ImmVector::classof())
    : BaseVector(cls, HeaderKind::ImmVector) { }
  explicit c_ImmVector(Class* cls, ArrayData* arr)
    : BaseVector(cls, HeaderKind::ImmVector, arr) { }
  explicit c_ImmVector(Class* cls, uint32_t cap)
    : BaseVector(cls, HeaderKind::ImmVector, cap) { }
  explicit c_ImmVector(uint32_t cap, Class* cls = c_ImmVector::classof())
    : c_ImmVector(cls, cap) { }

  static c_ImmVector* Clone(ObjectData* obj);

  friend class c_Vector;
  friend class c_Pair;
};

//////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
// class HashCollection

class HashCollection : public ExtCollectionObjectData {
 public:
  explicit HashCollection(Class* cls, HeaderKind kind)
    : ExtCollectionObjectData(cls, kind)
    , m_versionAndSize(0)
    , m_data(mixedData(staticEmptyMixedArray()))
  {}
  explicit HashCollection(Class* cls, HeaderKind kind, ArrayData* arr)
    : ExtCollectionObjectData(cls, kind)
    , m_versionAndSize(arr->m_size)
    , m_data(mixedData(MixedArray::asMixed(arr)))
  {}
  explicit HashCollection(Class* cls, HeaderKind kind, uint32_t cap);

  typedef MixedArray::Elm Elm;

 protected:
  union {
    struct {
      uint32_t m_size;    // Number of values
      int32_t m_version;  // Version number (high bit used to indicate if this
    };                    //   collection might contain int-like string keys)
    int64_t m_versionAndSize;
  };

  Elm* m_data;           // Elm store.

  // A pointer to an immutable collection that shares its buffer with
  // this collection.
  Object m_immCopy;

  template <typename F> friend void scan(const HashCollection& this_, F& mark);

 protected:
  // Read the high bit of m_version to tell if this collection might contain
  // int-like string keys. If this method returns false it is safe to assume
  // that no int-like strings keys are present. If this method returns true
  // that means there _might_ be int-like string keys, but there might not be.
  bool intLikeStrKeys() const { return (bool)(m_version & 0x80000000UL); }
  // Beware: calling this method can invalidate iterators, so use with
  // caution
  void setIntLikeStrKeys(bool b) {
    if (b) {
      m_version |= 0x80000000UL;
    } else {
      m_version &= ~0x80000000UL;
    }
  }

 public:
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

  static void copyElm(const Elm& frE, Elm& toE) {
    memcpy(&toE, &frE, sizeof(Elm));
  }

  static void dupElm(const Elm& frE, Elm& toE) {
    assert(!isTombstoneType(frE.data.m_type));
    memcpy(&toE, &frE, sizeof(Elm));
    if (toE.hasStrKey()) toE.skey->incRefCount();
    tvRefcountedIncRef(&toE.data);
  }

  MixedArray* arrayData() {
    auto* ret = getArrayFromMixedData(m_data);
    assert(ret == staticEmptyMixedArray() || ret->isMixed());
    return ret;
  }
  const MixedArray* arrayData() const {
    auto* ret = getArrayFromMixedData(m_data);
    assert(ret == staticEmptyMixedArray() || ret->isMixed());
    return ret;
  }

 protected:
  inline Elm* data() { return m_data; }
  inline const Elm* data() const { return m_data; }
  inline int32_t* hashTab() const {
    return reinterpret_cast<int32_t*>(
      m_data + static_cast<size_t>(arrayData()->scale()) * 3
    );
  }

  void setSize(uint32_t sz) {
    assert(sz <= cap());
    if (m_data == mixedData(staticEmptyMixedArray())) {
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
  inline uint32_t scale() const {
    return arrayData()->scale();
  }
  inline uint32_t cap() const {
    return arrayData()->capacity();
  }
  inline uint32_t tableMask() const {
    return arrayData()->mask();
  }
  inline uint32_t posLimit() const {
    return arrayData()->m_used;
  }
  inline void incPosLimit() {
    assert(!arrayData()->hasMultipleRefs());
    assert(posLimit() + 1 <= cap());
    arrayData()->m_used++;
  }
  inline void setPosLimit(uint32_t limit) {
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

  void throwTooLarge() ATTRIBUTE_NORETURN;
  void throwReserveTooLarge() ATTRIBUTE_NORETURN;
  int32_t* warnUnbalanced(size_t n, int32_t* ei) const;

  /**
   * Raises a warning if the set contains an int and a string with the same
   * numeric value: e.g. Set {'123', 123}. It's a no-op otherwise.
   */
  void warnOnStrIntDup() const;

 public:
  int getVersion() const {
    return m_version;
  }
  int64_t size() const {
    return m_size;
  }

  bool contains(int64_t key) const;
  bool contains(StringData* key) const;

 protected:
  bool toBoolImpl() const {
    return (m_size != 0);
  }

  template <class Hit>
  ssize_t findImpl(size_t h0, Hit) const;
  ssize_t find(int64_t h) const;
  ssize_t find(const StringData* s, strhash_t h) const;

  template <class Hit>
  int32_t* findForInsertImpl(size_t h0, Hit) const;
  int32_t* findForInsert(int64_t h) const;
  int32_t* findForInsert(const StringData* s, strhash_t h) const;

  ssize_t findForRemove(int64_t h) {
    assert(canMutateBuffer());
    return arrayData()->findForRemove(h, false);
  }

  ssize_t findForRemove(const StringData* s, strhash_t h) {
    assert(canMutateBuffer());
    return arrayData()->findForRemove(s, h);
  }

  int32_t* findForNewInsert(size_t h0) const;
  int32_t* findForNewInsert(int32_t* table, size_t mask, size_t h0) const;

 public:
  /**
   * canMutateBuffer() indicates whether it is currently safe to directly
   * modify this HashCollection's buffer.
   */
  bool canMutateBuffer() const {
    assert(IMPLIES(!arrayData()->hasMultipleRefs(), m_immCopy.isNull()));
    return !arrayData()->hasMultipleRefs();
  }

  static constexpr ptrdiff_t dataOffset() {
    return offsetof(HashCollection, m_data);
  }

  static bool validPos(ssize_t pos) {
    return pos >= 0;
  }

  static bool validPos(int32_t pos) {
    return pos >= 0;
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
    return skipTombstonesNoBoundsCheck(pos, posLimit(), m_data);
  }

  inline const Elm* firstElmImpl() const {
    const Elm* e = data();
    const Elm* eLimit = elmLimit();
    for (; e != eLimit && isTombstone(e); ++e) {}
    return (Elm*)e;
  }
  inline Elm* firstElm() {
    return (Elm*)firstElmImpl();
  }
  inline const Elm* firstElm() const {
    return firstElmImpl();
  }

  inline Elm* elmLimit() {
    return data() + posLimit();
  }
  inline const Elm* elmLimit() const {
    return data() + posLimit();
  }

  inline static Elm* nextElm(Elm* e, Elm* eLimit) {
    assert(e != eLimit);
    for (++e; e != eLimit && isTombstone(e); ++e) {}
    return e;
  }
  inline static const Elm* nextElm(const Elm* e, const Elm* eLimit) {
    return (const Elm*)nextElm((Elm*)e, (Elm*)eLimit);
  }

  static bool isTombstoneType(DataType t) {
    assert(IS_REAL_TYPE(t) || t == kInvalidDataType);
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
    wordfill(table, Empty, tableSize);
  }

  void remove(int64_t key);
  void remove(StringData* key);

  void eraseNoCompact(ssize_t pos);

  void erase(ssize_t pos) {
    eraseNoCompact(pos);
    compactOrShrinkIfDensityTooLow();
  }

  bool isFull() { return posLimit() == cap(); }

  bool isDensityTooLow() const {
    bool b = (m_size < posLimit() / 2);
    assert(IMPLIES(m_data == mixedData(staticEmptyMixedArray()), !b));
    assert(IMPLIES(cap() == 0, !b));
    return b;
  }

  bool isCapacityTooHigh() const {
    // Return true if current capacity at least 8x greater than m_size AND
    // if current capacity is at least 8x greater than the minimum capacity
    bool b = ((uint64_t(cap()) >= uint64_t(m_size) * 8) &&
              (cap() >= HashCollection::SmallSize * 8));
    assert(IMPLIES(m_data == mixedData(staticEmptyMixedArray()), !b));
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
      return e->skey;
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

 public:
  /**
   * mutate() must be called before doing anything that mutates
   * this HashCollection's buffer, unless it can be proven that
   * canMutateBuffer() is true. mutate() takes care of updating
   * m_immCopy and making a copy of this HashCollection's buffer
   * if needed.
   */
  void mutate() {
    assert(IMPLIES(!m_immCopy.isNull(), arrayData()->hasMultipleRefs()));
    if (arrayData()->hasMultipleRefs()) {
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
           (m_data == ((HashCollection*)m_immCopy.get())->m_data &&
            arrayData()->hasMultipleRefs()));
    m_immCopy.reset();
  }

 protected:
  /**
   * Copy the buffer and reset the immutable copy.
   */
  void mutateImpl();

 public:
  bool t_isempty();
  int64_t t_count();
  void t_reserve(const Variant& sz);

  Array t_toarray();
  Array t_tokeysarray();
  Array t_tovaluesarray();
  Object t_lazy();

  DECLARE_COLLECTION_MAGIC_METHODS();

 private:
  struct EmptyMixedInitializer;
  static EmptyMixedInitializer s_empty_mixed_initializer;

 private:
  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc, bool checkTypes);
  void postSort();

 public:
  void asort(int sort_flags, bool ascending);
  void ksort(int sort_flags, bool ascending);
  bool uasort(const Variant& cmp_function);
  bool uksort(const Variant& cmp_function);
};

///////////////////////////////////////////////////////////////////////////////
// class BaseMap

/**
 * BaseMap is a hash-table implementation with int and string keys only.
 * It doesn't represent any PHP-land class; that job is delegated to its
 * c_-prefixed child classes.
 */
class BaseMap : public HashCollection {
 protected:
  static void throwOOB(int64_t key) ATTRIBUTE_NORETURN;
  static void throwOOB(StringData* key) ATTRIBUTE_NORETURN;

 public:
  TypedValue* at(int64_t key) const;
  TypedValue* at(StringData* key) const;
  TypedValue* get(int64_t key) const;
  TypedValue* get(StringData* key) const;

  void add(const TypedValue* val);
  void add(const Variant& val) { add(val.asCell()); }

  Variant pop();
  Variant popFront();

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, TMap*>::type
  static Clone(ObjectData* obj);

  static Array ToArray(const ObjectData* obj);
  static bool ToBool(const ObjectData* obj);
  template <bool throwOnMiss>
  static TypedValue* OffsetAt(ObjectData* obj, const TypedValue* key) {
    assertx(key->m_type != KindOfRef);
    auto mp = static_cast<BaseMap*>(obj);
    if (key->m_type == KindOfInt64) {
      return throwOnMiss ? mp->at(key->m_data.num)
                         : mp->get(key->m_data.num);
    }
    if (IS_STRING_TYPE(key->m_type)) {
      return throwOnMiss ? mp->at(key->m_data.pstr)
                         : mp->get(key->m_data.pstr);
    }
    throwBadKeyType();
    return nullptr;
  }
  static void OffsetSet(ObjectData* obj, const TypedValue* key,
                        const TypedValue* val);
  static bool OffsetIsset(ObjectData* obj, const TypedValue* key);
  static bool OffsetEmpty(ObjectData* obj, const TypedValue* key);
  static bool OffsetContains(ObjectData* obj, const TypedValue* key);
  static void OffsetUnset(ObjectData* obj, const TypedValue* key);

  enum EqualityFlavor { OrderMatters, OrderIrrelevant };

  static bool Equals(EqualityFlavor eq,
                     const ObjectData* obj1, const ObjectData* obj2);

  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type);

  void init(const Variant& t);

  template<bool raw>
  void setImpl(int64_t h, const TypedValue* val);
  template<bool raw>
  void setImpl(StringData* key, const TypedValue* data);

  // setRaw() assigns a value to the specified key in this Map, but doesn't
  // check for an immutable buffer and doesn't increment m_version, so it's
  // only safe to use in some cases. If you're not sure, use set() instead.
  void setRaw(int64_t h, const TypedValue* data);
  void setRaw(StringData* key, const TypedValue* data);
  void setRaw(int64_t h, const Variant& data) {
    setRaw(h, data.asCell());
  }
  void setRaw(StringData* key, const Variant& data) {
    setRaw(key, data.asCell());
  }
  void setRaw(const TypedValue* key, const TypedValue* data) {
    assert(key->m_type != KindOfRef);
    if (key->m_type == KindOfInt64) {
      setRaw(key->m_data.num, data);
    } else if (IS_STRING_TYPE(key->m_type)) {
      setRaw(key->m_data.pstr, data);
    } else {
      throwBadKeyType();
    }
  }
  void setRaw(const Variant& key, const Variant& data) {
    setRaw(key.asCell(), data.asCell());
  }

  void set(int64_t h, const TypedValue* data);
  void set(StringData* key, const TypedValue* data);
  void set(int64_t h, const Variant& data) {
    set(h, data.asCell());
  }
  void set(StringData* key, const Variant& data) {
    set(key, data.asCell());
  }
  void set(const TypedValue* key, const TypedValue* data) {
    assert(key->m_type != KindOfRef);
    if (key->m_type == KindOfInt64) {
      set(key->m_data.num, data);
    } else if (IS_STRING_TYPE(key->m_type)) {
      set(key->m_data.pstr, data);
    } else {
      throwBadKeyType();
    }
  }
  void set(const Variant& key, const Variant& data) {
    set(key.asCell(), data.asCell());
  }

 protected:
  // BaseMap is an abstract class, with no additional member needing
  // initialization.
  using HashCollection::HashCollection;

  ~BaseMap();

 public:
  static void throwBadKeyType() ATTRIBUTE_NORETURN;

 private:
  friend void collections::deepCopy(TypedValue*);

  friend class collections::MapIterator;
  friend class c_Vector;
  friend class c_Map;
  friend class c_ImmMap;
  friend class ArrayIter;
  friend class c_AwaitAllWaitHandle;
  friend class c_GenMapWaitHandle;

  static void compileTimeAssertions() {
    // For performance, all native collection classes have their m_size field
    // at the same offset.
    static_assert(offsetof(BaseMap, m_size)
                  == FAST_COLLECTION_SIZE_OFFSET, "");
  }

 protected:
  template<class TVector>
  Object php_values() {
    auto target = req::make<TVector>();
    int64_t sz = m_size;
    target->reserve(sz);
    assert(target->canMutateBuffer());
    target->setSize(sz);
    auto* out = target->m_data;
    auto* eLimit = elmLimit();
    for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit), ++out) {
      cellDup(e->data, *out);
    }
    return Object{std::move(target)};
  }

  template<class TVector>
  Object php_keys() {
    auto vec = req::make<TVector>();
    vec->reserve(m_size);
    assert(vec->canMutateBuffer());
    auto* e = firstElm();
    auto* eLimit = elmLimit();
    ssize_t j = 0;
    for (; e != eLimit; e = nextElm(e, eLimit), vec->incSize(), ++j) {
      if (e->hasIntKey()) {
        vec->m_data[j].m_data.num = e->ikey;
        vec->m_data[j].m_type = KindOfInt64;
      } else {
        assert(e->hasStrKey());
        cellDup(make_tv<KindOfString>(e->skey), vec->m_data[j]);
      }
    }
    return Object{std::move(vec)};
  }

 public:
  void t___construct(const Variant& iterable = null_variant);

  Variant t_at(const Variant& key);
  Variant t_get(const Variant& key);
  bool t_containskey(const Variant& key);
  bool t_contains(const Variant& key);
  Object t_keys();
  Object t_items();
  Object t_getiterator();

 private:
  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_differenceByKey(const Variant& it);

  template<class TMap, class MakeArgs>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_map(const Variant& callback, MakeArgs) const;

  template<class TMap, class MakeArgs>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_filter(const Variant& callback, MakeArgs) const;

  template<class MakeArgs>
  Object php_retain(const Variant& callback, MakeArgs);

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_zip(const Variant& iterable) const;

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_take(const Variant& n);

  template<class TMap, bool checkVersion>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_takeWhile(const Variant& fn);

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_skip(const Variant& n);

  template<class TMap, bool checkVersion>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_skipWhile(const Variant& fn);

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_slice(const Variant& start, const Variant& len);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_concat(const Variant& iterable);

 public:
  Variant t_firstvalue();
  Variant t_firstkey();
  Variant t_lastvalue();
  Variant t_lastkey();

  Object t_tovector();
  Object t_toimmvector();
  Object t_toset();
  Object t_toimmset();

 private:
  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  static php_mapFromItems(const Variant& iterable);

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  static php_mapFromArray(const Variant& arr);
};

///////////////////////////////////////////////////////////////////////////////
// class Map

class c_Map : public BaseMap {
 public:
  DECLARE_CLASS_NO_SWEEP(Map)

 public:
  explicit c_Map(Class* cls = c_Map::classof())
    : BaseMap(cls, HeaderKind::Map) { }
  explicit c_Map(Class* cls, ArrayData* arr)
    : BaseMap(cls, HeaderKind::Map, arr) { }
  explicit c_Map(Class* cls, uint32_t cap)
    : BaseMap(cls, HeaderKind::Map, cap) { }
  explicit c_Map(uint32_t cap, Class* cls = c_Map::classof())
    : c_Map(cls, cap) { }

  static c_Map* Clone(ObjectData* obj);

 public:
  Object t_add(const Variant& val);
  Object t_addall(const Variant& val);
  Object t_clear();
  Object t_set(const Variant& key, const Variant& value);
  Object t_setall(const Variant& iterable);
  Object t_remove(const Variant& key);
  Object t_removekey(const Variant& key);
  Object t_tomap();
  Object t_toimmmap();
  Object t_values();
  Object t_keys();
  Object t_map(const Variant& callback);
  Object t_mapwithkey(const Variant& callback);
  Object t_filter(const Variant& callback);
  Object t_filterwithkey(const Variant& callback);
  Object t_retain(const Variant& callback);
  Object t_retainwithkey(const Variant& callback);
  Object t_zip(const Variant& iterable);
  Object t_take(const Variant& n);
  Object t_takewhile(const Variant& callback);
  Object t_skip(const Variant& n);
  Object t_skipwhile(const Variant& fn);
  Object t_slice(const Variant& start, const Variant& len);
  Object t_concat(const Variant& iterable);
  static Object ti_fromitems(const Variant& iterable);
  static Object ti_fromarray(const Variant& arr); // deprecated
  Object t_differencebykey(const Variant& it);
  Object t_immutable();
  String t___tostring();

 protected:
  Object getImmutableCopy();

  friend class BaseMap;
  friend class c_ImmMap;
};

///////////////////////////////////////////////////////////////////////////////
// class ImmMap

class c_ImmMap : public BaseMap {
 public:
  DECLARE_CLASS_NO_SWEEP(ImmMap)

 public:
  explicit c_ImmMap(Class* cls = c_ImmMap::classof())
    : BaseMap(cls, HeaderKind::ImmMap) { }
  explicit c_ImmMap(Class* cls, ArrayData* arr)
    : BaseMap(cls, HeaderKind::ImmMap, arr) { }
  explicit c_ImmMap(Class* cls, uint32_t cap)
    : BaseMap(cls, HeaderKind::ImmMap, cap) { }
  explicit c_ImmMap(uint32_t cap, Class* cls = c_ImmMap::classof())
    : c_ImmMap(cls, cap) { }

  static c_ImmMap* Clone(ObjectData* obj);

 public:
  Object t_tomap();
  Object t_toimmmap();
  Object t_differencebykey(const Variant& it);
  Object t_values();
  Object t_keys();
  Object t_map(const Variant& callback);
  Object t_mapwithkey(const Variant& callback);
  Object t_filter(const Variant& callback);
  Object t_filterwithkey(const Variant& callback);
  Object t_zip(const Variant& iterable);
  Object t_take(const Variant& n);
  Object t_takewhile(const Variant& callback);
  Object t_skip(const Variant& n);
  Object t_skipwhile(const Variant& fn);
  Object t_slice(const Variant& start, const Variant& len);
  Object t_concat(const Variant& iterable);
  static Object ti_fromitems(const Variant& iterable);
  Object t_immutable();
  String t___tostring();

  friend class BaseMap;
  friend class c_Map;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * BaseSet is a hash-table implementation of the Set ADT. It doesn't represent
 * any PHP-land class. That job is delegated to its c_-prefixed child classes.
 */
class BaseSet : public HashCollection {
 public:
  void addAllKeysOf(Cell container);
  void addAll(const Variant& t);

  void init(const Variant& t);

  template <bool raw>
  void addImpl(int64_t h);
  template <bool raw>
  void addImpl(StringData* key);

  void addRaw(int64_t h);
  void addRaw(StringData* key);
  void addRaw(const TypedValue* val) {
    assert(val->m_type != KindOfRef);
    if (val->m_type == KindOfInt64) {
      addRaw(val->m_data.num);
    } else if (IS_STRING_TYPE(val->m_type)) {
      addRaw(val->m_data.pstr);
    } else {
      throwBadValueType();
    }
  }
  void addRaw(const Variant& val) {
    addRaw(val.asCell());
  }

  void add(int64_t h);
  void add(StringData* key);
  void add(const TypedValue* val) {
    assert(val->m_type != KindOfRef);
    if (val->m_type == KindOfInt64) {
      add(val->m_data.num);
    } else if (IS_STRING_TYPE(val->m_type)) {
      add(val->m_data.pstr);
    } else {
      throwBadValueType();
    }
  }
  void add(const Variant& val) {
    add(val.asCell());
  }

  void addFront(int64_t h);
  void addFront(StringData* key);
  void addFront(const TypedValue* val) {
    if (val->m_type == KindOfInt64) {
      addFront(val->m_data.num);
    } else if (IS_STRING_TYPE(val->m_type)) {
      addFront(val->m_data.pstr);
    } else {
      throwBadValueType();
    }
  }

  Variant pop();
  Variant popFront();

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, TSet*>::type
  static Clone(ObjectData* obj);

  // Static methods
  static void throwOOB(int64_t key) ATTRIBUTE_NORETURN;
  static void throwOOB(StringData* key) ATTRIBUTE_NORETURN;
  static void throwNoMutableIndexAccess() ATTRIBUTE_NORETURN;

  static Array ToArray(const ObjectData* obj);
  static bool ToBool(const ObjectData* obj);

  template <bool throwOnMiss>
  static TypedValue* OffsetAt(ObjectData* obj, const TypedValue* key) {
    assertx(key->m_type != KindOfRef);
    auto st = static_cast<BaseSet*>(obj);
    ssize_t p;
    if (key->m_type == KindOfInt64) {
      p = st->find(key->m_data.num);
    } else if (IS_STRING_TYPE(key->m_type)) {
      p = st->find(key->m_data.pstr, key->m_data.pstr->hash());
    } else {
      BaseSet::throwBadValueType();
    }
    if (LIKELY(p != Empty)) {
      return reinterpret_cast<TypedValue*>(&st->data()[p].data);
    }
    if (!throwOnMiss) {
      return nullptr;
    }
    if (key->m_type == KindOfInt64) {
      BaseSet::throwOOB(key->m_data.num);
    } else {
      assert(IS_STRING_TYPE(key->m_type));
      BaseSet::throwOOB(key->m_data.pstr);
    }
  }
  static bool OffsetIsset(ObjectData* obj, const TypedValue* key);
  static bool OffsetEmpty(ObjectData* obj, const TypedValue* key);
  static bool OffsetContains(ObjectData* obj, const TypedValue* key);
  static void OffsetUnset(ObjectData* obj, const TypedValue* key);

  static bool Equals(const ObjectData* obj1, const ObjectData* obj2);

  static void Unserialize(ObjectData* obj,
                          VariableUnserializer* uns, int64_t sz, char type);

 protected:
  // PHP-land methods exported by child classes.
  template<class TVector>
  Object php_values() {
    auto vec = req::make<TVector>();
    vec->init(VarNR(this));
    return Object{std::move(vec)};
  }

 public:
  void t___construct(const Variant& iterable = null_variant);

  Object t_getiterator();
  Object t_items();
  bool t_contains(const Variant& key);

 protected:
  template<class TSet, class MakeArgs>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_map(const Variant& callback, MakeArgs) const;

  template<class TSet, class MakeArgs>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_filter(const Variant& callback, MakeArgs) const;

  template<class MakeArgs>
  Object php_retain(const Variant& callback, MakeArgs);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_zip(const Variant& iterable);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_take(const Variant& n);

  template<class TSet, bool checkVersion>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_takeWhile(const Variant& fn);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_skip(const Variant& n);

  template<class TSet, bool checkVersion>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_skipWhile(const Variant& fn);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_slice(const Variant& start, const Variant& len);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_concat(const Variant& iterable);

 public:
  Variant t_firstvalue();
  Variant t_firstkey();
  Variant t_lastvalue();
  Variant t_lastkey();

  Object t_tovector();
  Object t_toimmvector();
  Object t_tomap();
  Object t_toimmmap();

 protected:
  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  static php_fromItems(const Variant& iterable);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  static php_fromKeysOf(const Variant& container);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  static php_fromArray(const Variant& arr);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  static php_fromArrays(int _argc, const Array& _argv = null_array);

 protected:
  // BaseSet is an abstract class with no additional member needing
  // initialization.
  using HashCollection::HashCollection;

  ~BaseSet();

 private:
  // Helpers

  static void throwBadValueType() ATTRIBUTE_NORETURN;

 private:

  friend class collections::SetIterator;
  friend class c_Vector;
  friend class c_Set;
  friend class c_Map;
  friend class ArrayIter;

  static void compileTimeAssertions() {
    // For performance, all native collection classes have their m_size field
    // at the same offset.
    static_assert(offsetof(BaseSet, m_size) == FAST_COLLECTION_SIZE_OFFSET, "");
  }
};

///////////////////////////////////////////////////////////////////////////////
// class Set

class c_Set : public BaseSet {
 public:
  DECLARE_CLASS_NO_SWEEP(Set)

 public:
  // PHP-land methods.
  explicit c_Set(Class* cls = c_Set::classof())
    : BaseSet(cls, HeaderKind::Set) { }
  explicit c_Set(Class* cls, ArrayData* arr)
    : BaseSet(cls, HeaderKind::Set, arr) { }
  explicit c_Set(Class* cls, uint32_t cap)
    : BaseSet(cls, HeaderKind::Set, cap) { }
  explicit c_Set(uint32_t cap, Class* cls = c_Set::classof())
    : c_Set(cls, cap) { }

  Object t_add(const Variant& val);
  Object t_addall(const Variant& val);
  Object t_addallkeysof(const Variant& val);
  Object t_clear();
  Object t_values();
  Object t_keys();
  Object t_remove(const Variant& key);
  Object t_map(const Variant& callback);
  Object t_mapwithkey(const Variant& callback);
  Object t_filter(const Variant& callback);
  Object t_filterwithkey(const Variant& callback);
  Object t_retain(const Variant& callback);
  Object t_retainwithkey(const Variant& callback);
  Object t_zip(const Variant& iterable);
  Object t_take(const Variant& n);
  Object t_takewhile(const Variant& callback);
  Object t_skip(const Variant& n);
  Object t_skipwhile(const Variant& fn);
  Object t_slice(const Variant& start, const Variant& len);
  Object t_concat(const Variant& iterable);
  Object t_removeall(const Variant& iterable);
  Object t_difference(const Variant& iterable);
  static Object ti_fromitems(const Variant& iterable);
  static Object ti_fromkeysof(const Variant& container);
  static Object ti_fromarray(const Variant& arr); // deprecated
  static Object ti_fromarrays(int _argc, const Array& _argv = null_array);
  Object t_toset();
  Object t_toimmset();
  Object t_immutable();
  String t___tostring();

 public:
  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type);

  static c_Set* Clone(ObjectData* obj);

 protected:
  Object getImmutableCopy();
};

///////////////////////////////////////////////////////////////////////////////
// class ImmSet

class c_ImmSet : public BaseSet {
 public:
  DECLARE_CLASS_NO_SWEEP(ImmSet)

 public:
  // API
  Object t_values();
  Object t_keys();
  Object t_map(const Variant& callback);
  Object t_mapwithkey(const Variant& callback);
  Object t_filter(const Variant& callback);
  Object t_filterwithkey(const Variant& callback);
  Object t_zip(const Variant& iterable);
  Object t_take(const Variant& n);
  Object t_takewhile(const Variant& callback);
  Object t_skip(const Variant& n);
  Object t_skipwhile(const Variant& fn);
  Object t_slice(const Variant& start, const Variant& len);
  Object t_concat(const Variant& iterable);

  // Static methods.
  static Object ti_fromitems(const Variant& iterable);
  static Object ti_fromkeysof(const Variant& container);
  static Object ti_fromarrays(int _argc, const Array& _argv = null_array);

  Object t_toset();
  Object t_toimmset();
  Object t_immutable();
  String t___tostring();

 public:
  explicit c_ImmSet(Class* cls = c_ImmSet::classof())
    : BaseSet(cls, HeaderKind::ImmSet) { }
  explicit c_ImmSet(Class* cls, ArrayData* arr)
    : BaseSet(cls, HeaderKind::ImmSet, arr) { }
  explicit c_ImmSet(Class* cls, uint32_t cap)
    : BaseSet(cls, HeaderKind::ImmSet, cap) { }
  explicit c_ImmSet(uint32_t cap, Class* cls = c_ImmSet::classof())
    : c_ImmSet(cls, cap) { }

  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type);

  static c_ImmSet* Clone(ObjectData* obj);
};

///////////////////////////////////////////////////////////////////////////////
// class Pair

class c_Pair : public ExtObjectDataFlags<ObjectData::IsCollection|
                                         ObjectData::UseGet|
                                         ObjectData::UseSet|
                                         ObjectData::UseIsset|
                                         ObjectData::UseUnset|
                                         ObjectData::HasClone|
                                         ObjectData::NoDestructor> {
 public:
  DECLARE_CLASS_NO_SWEEP(Pair)

 public:
  enum class NoInit {};

  explicit c_Pair(Class* cls = c_Pair::classof())
    : ExtObjectDataFlags(cls, HeaderKind::Pair)
    , m_size(2)
  {
    tvWriteNull(&elm0);
    tvWriteNull(&elm1);
  }
  explicit c_Pair(NoInit, Class* cls = c_Pair::classof())
    : ExtObjectDataFlags(cls, HeaderKind::Pair)
    , m_size(0)
  {}

  void reserve(int64_t sz) { assertx(sz == 2); }
  ~c_Pair();
  void t___construct(int _argc, const Array& _argv = null_array);
  bool t_isempty();
  int64_t t_count();
  Object t_items();
  Object t_keys();
  Object t_values();
  Object t_lazy();
  Variant t_at(const Variant& key);
  Variant t_get(const Variant& key);
  bool t_containskey(const Variant& key);
  Array t_toarray();
  Array t_tokeysarray();
  Array t_tovaluesarray();
  DECLARE_KEYEDITERABLE_MATERIALIZE_METHODS();
  Object t_getiterator();
  int64_t t_linearsearch(const Variant& search_value);
  Object t_map(const Variant& callback);
  Object t_mapwithkey(const Variant& callback);
  Object t_filter(const Variant& callback);
  Object t_filterwithkey(const Variant& callback);
  Object t_zip(const Variant& iterable);
  Object t_take(const Variant& n);
  Object t_takewhile(const Variant& callback);
  Object t_skip(const Variant& n);
  Object t_skipwhile(const Variant& fn);
  Object t_slice(const Variant& start, const Variant& len);
  Object t_concat(const Variant& iterable);
  Variant t_firstvalue();
  Variant t_firstkey();
  Variant t_lastvalue();
  Variant t_lastkey();
  DECLARE_COLLECTION_MAGIC_METHODS();
  Object t_immutable();
  String t___tostring();

  static void throwOOB(int64_t key) ATTRIBUTE_NORETURN;

  /**
   * Most methods that operate on Pairs can safely assume that all Pairs have
   * two elements that have been initialized. However, methods that deal with
   * initializing and destructing Pairs needs to handle intermediate states
   * where one or both of the elements is uninitialized.
   */
  bool isFullyConstructed() const {
    return m_size == 2;
  }

  TypedValue* at(int64_t key) {
    assert(isFullyConstructed());
    if (UNLIKELY(uint64_t(key) >= uint64_t(2))) {
      throwOOB(key);
      return NULL;
    }
    return &getElms()[key];
  }
  TypedValue* get(int64_t key) {
    assert(isFullyConstructed());
    if (uint64_t(key) >= uint64_t(2)) {
      return NULL;
    }
    return &getElms()[key];
  }
  void initAdd(const TypedValue* val) {
    assert(!isFullyConstructed());
    assert(val->m_type != KindOfRef);
    cellDup(*val, getElms()[m_size]);
    ++m_size;
  }
  void initAdd(const Variant& val) {
    initAdd(val.asCell());
  }
  bool contains(int64_t key) const {
    assert(isFullyConstructed());
    return (uint64_t(key) < uint64_t(2));
  }

  Array toArrayImpl() const;

  static c_Pair* Clone(ObjectData* obj);
  static bool ToBool(const ObjectData* obj) {
    assertx(obj->getVMClass() == c_Pair::classof());
    assertx(static_cast<const c_Pair*>(obj)->isFullyConstructed());
    return true;
  }
  static Array ToArray(const ObjectData* obj);
  template <bool throwOnMiss>
  static TypedValue* OffsetAt(ObjectData* obj, const TypedValue* key) {
    assertx(key->m_type != KindOfRef);
    auto pair = static_cast<c_Pair*>(obj);
    assertx(pair->isFullyConstructed());
    if (key->m_type == KindOfInt64) {
      return throwOnMiss ? pair->at(key->m_data.num)
                         : pair->get(key->m_data.num);
    }
    throwBadKeyType();
    return nullptr;
  }
  static bool OffsetIsset(ObjectData* obj, const TypedValue* key);
  static bool OffsetEmpty(ObjectData* obj, const TypedValue* key);
  static bool OffsetContains(ObjectData* obj, const TypedValue* key);
  static bool Equals(const ObjectData* obj1, const ObjectData* obj2);
  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type);

  int64_t size() const {
    assert(isFullyConstructed());
    return 2;
  }

  static constexpr uint32_t dataOffset() { return offsetof(c_Pair, elm0); }

 private:
  static void throwBadKeyType() ATTRIBUTE_NORETURN;

#ifndef USE_LOWPTR
  // Add 4 bytes here to keep m_size aligned the same way as in BaseVector and
  // HashCollection.
  UNUSED uint32_t dummy;
#endif
  uint32_t m_size;

  TypedValue elm0;
  TypedValue elm1;

  TypedValue* getElms() { return &elm0; }
  const TypedValue* getElms() const { return &elm0; }

  int getVersion() const { return 0; }

  friend void collections::deepCopy(TypedValue*);
  friend class collections::PairIterator;
  friend class c_Vector;
  friend class BaseVector;
  friend class BaseMap;
  friend class ArrayIter;

  static void compileTimeAssertions() {
    // For performance, all native collection classes have their m_size field
    // at the same offset.
    static_assert(offsetof(c_Pair, m_size) == FAST_COLLECTION_SIZE_OFFSET, "");
  }
};

///////////////////////////////////////////////////////////////////////////////

// Collection constructors do not throw exceptions, let's not try to catch
// exceptions here.
template<class T, class... Args> T* newCollectionObj(Args&&... args) {
  static_assert(std::is_convertible<T*,BaseVector*>::value ||
                std::is_convertible<T*,HashCollection*>::value ||
                std::is_convertible<T*,c_Pair*>::value, "");
  auto const mem = MM().mallocSmallSize(sizeof(T));
  auto col = new (mem) T(std::forward<Args>(args)...);
  assert(col->hasExactlyOneRef());
  return col;
}

}

#undef DECLARE_COLLECTION_MAGIC_METHODS
#undef DECLARE_ITERABLE_MATERIALIZE_METHODS
#undef DECLARE_KEYEDITERABLE_MATERIALIZE_METHODS

#endif // incl_HPHP_EXT_COLLECTION_H_
